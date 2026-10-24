# Shared ModelSim GUI transaction drivers and real X11 window captures.
namespace eval rover {
    variable here [file dirname [info script]]
    variable root [file normalize [file join $here ../..]]
    variable build $::env(ROVER_SIM_BUILD)
    variable shots [file join $root screenshots]
    variable results {}
    variable checks 0
    variable failures 0
    variable t 0
    variable top {}
    variable ww {}
    variable category {}
    variable pending_failures 0
    variable check_results {}
    variable simulation_generation 0
    file mkdir $shots
}
proc rover::begin {design title signals args} {
    variable t; variable top; variable ww; variable category
    variable pending_failures; set pending_failures 0
    variable simulation_generation; incr simulation_generation
    catch {quit -sim}
    catch {delete wave *}
    set t 0
    set category $title
    # HLS ROM initialization files are kept with the source. Copy them into the
    # external simulator directory, refreshing colliding generated filenames.
    variable libraries; variable build
    set lib [lindex [split $design .] 0]
    if {[info exists libraries] && [dict exists $libraries $lib]} {
        foreach src [dict get $libraries $lib] {
            foreach dat [glob -nocomplain -directory [file dirname $src] *.dat] {
                file copy -force $dat [file join $build [file tail $dat]]
            }
        }
    }
    vsim -t 1ps -voptargs=+acc -L unisim {*}$args $design
    set top /[lindex [split $design .] end]
    set ww [view wave -undock -width 1600 -height 700 -x 0 -y 0]
    add wave -divider $title
    if {$signals eq "@auto"} {
        set all [lsort [find signals $top/*]]
        if {![llength $all]} {set all [lsort [find signals -r $top/*]]}
        set signals {}
        foreach s $all {
            if {[regexp -nocase {clk|rst|reset|sclk|sdio|tcs|data|addr|busy|done|error|valid|gain|coupling|relay} [file tail $s]]} {
                lappend signals $s
                if {[llength $signals]>=24} {break}
            }
        }
        if {![llength $signals]} {set signals [lrange $all 0 23]}
    }
    foreach s $signals {
        add wave -radix hexadecimal -label [file tail $s] [path $s]
    }
    configure wave -namecolwidth 280 -valuecolwidth 150 -signalnamewidth 1 -timelineunits ns
    onbreak {}
    set ::NumericStdNoWarnings 1
    set ::StdArithNoWarnings 1
}
proc rover::path {s} {
    variable top
    if {[string match pwm_axi_* $s]} {set s [string tolower $s]}
    if {[string match /* $s]} {return $s}
    return $top/$s
}
proc rover::drive {s value} {
    if {[string is entier -strict $value]} {set value 10#[expr {$value}]}
    force -freeze [path $s] $value
}
proc rover::clock {s {period 10}} {
    force -freeze [path $s] 0 0ns, 1 [expr {$period/2.0}]ns -repeat ${period}ns
}
proc rover::step {n} {
    variable t
    set target [expr {$t+$n}]
    run ${n}ns
    set t [expr {$::now/1000}]
    if {$t+0.0001 < $target} {error "Simulation stopped at ${t}ns before ${target}ns ([runStatus -full])"}
}
proc rover::value {s} {return [string trim [examine -radix unsigned [path $s]]]}
proc rover::check {s expected} {
    variable checks; variable failures
    variable pending_failures
    variable check_results; variable category; variable t
    incr checks
    set actual [value $s]
    lappend check_results [dict create category $category signal $s expected $expected observed $actual passed [expr {$actual eq $expected}] time_ns $t]
    if {$actual ne $expected} {
        incr failures
        incr pending_failures
        puts "CHECK FAILED $s: expected $expected, observed $actual"
        return 0
    }
    return 1
}
proc rover::snap {name start {end {}} {status observed}} {
    variable t; variable shots; variable ww; variable results; variable category
    variable pending_failures
    if {$pending_failures && $status ni {incomplete stopped_early}} {set status check_failed}
    set pending_failures 0
    if {$end eq {}} {set end $t}
    if {$start < 0} {set start 0}
    if {$end <= $start} {set end [expr {$start+10}]}
    wave zoom range ${start}ns ${end}ns
    set cursor [expr {($start+$end)/2}]
    catch {wave cursor time -time ${cursor}ns}
    set wt [winfo toplevel $ww]
    raise $wt
    update
    after 40
    update
    set dest [file join $shots ${name}.png]
    set captured 0
    for {set attempt 0} {$attempt<3} {incr attempt} {
        if {![catch {exec env MAGICK_THREAD_LIMIT=1 OMP_NUM_THREADS=1 import -window [winfo id $wt] -define png:exclude-chunks=date,time $dest} capture_error]} {set captured 1;break}
        after 200
        update
    }
    if {!$captured} {error "ModelSim screenshot capture failed: $capture_error"}
    lappend results [dict create screenshot screenshots/${name}.png category $category status $status start_ns $start end_ns $end]
    puts "CAPTURE $name ($status)"
}
proc rover::case_run {name body} {
    variable results; variable t; variable simulation_generation
    set previous_generation $simulation_generation
    if {[catch {uplevel 1 $body} msg opts]} {
        puts "CASE BLOCKED $name: $msg"
        lappend results [dict create case $name status blocked reason $msg]
        if {$t>0 && $simulation_generation>$previous_generation} {
            set safe [regsub -all {[^A-Za-z0-9_]} $name _]
            catch {snap stopped_$safe [expr {max(0,$t-3000)}] $t stopped_early}
        }
    }
    checkpoint
}
proc rover::checkpoint {} {
    variable build
    set f [open [file join $build state_new.tcl] w]
    foreach field {results check_results checks failures} {puts $f [list set $field [set ::rover::$field]]}
    close $f
    file rename -force [file join $build state_new.tcl] [file join $build state.tcl]
}
proc rover::wait_high {s {limit 200}} {
    for {set n 0} {$n < $limit} {incr n} {
        if {[value $s] eq "1"} {return}
        step 10
    }
    error "Timeout waiting for $s"
}
# All AXI channel drivers wait for handshakes, with bounded timeouts. The suite
# also tests AW-before-W, W-before-AW, response stalls and byte strobes.
proc rover::axi_init {prefix {clk S_AXI_ACLK} {reset S_AXI_ARESETN}} {
    variable ax; variable axreset
    set ax $prefix; set axreset $reset
    clock $clk
    drive $reset 0
    foreach {s v} {AWADDR 0 AWPROT 0 AWVALID 0 WDATA 0 WSTRB 15 WVALID 0 BREADY 1 ARADDR 0 ARPROT 0 ARVALID 0 RREADY 1} {
        catch {drive ${prefix}$s $v}
    }
    step 40; drive $reset 1; step 20
}
proc rover::axi_write {addr data {strobe 15} {skew 0} {stall 0}} {
    variable ax
    drive ${ax}AWADDR $addr; drive ${ax}WDATA $data; drive ${ax}WSTRB $strobe
    drive ${ax}BREADY [expr {$stall == 0}]
    set aw 0; set w 0
    drive ${ax}AWVALID [expr {$skew >= 0}]
    drive ${ax}WVALID [expr {$skew <= 0}]
    for {set n 0} {$n < 100} {incr n} {
        if {$skew > 0 && $n*10 >= $skew && !$w} {drive ${ax}WVALID 1}
        if {$skew < 0 && $n*10 >= -$skew && !$aw} {drive ${ax}AWVALID 1}
        if {!$aw && ($skew>=0 || $n*10>=-$skew) && [value ${ax}AWREADY] eq "1"} {set aw 1}
        if {!$w && ($skew<=0 || $n*10>=$skew) && [value ${ax}WREADY] eq "1"} {set w 1}
        step 10
        if {$aw} {drive ${ax}AWVALID 0}
        if {$w} {drive ${ax}WVALID 0}
        if {$aw && $w} {break}
    }
    if {!$aw || !$w} {error "AXI write handshake timeout"}
    wait_high ${ax}BVALID
    check ${ax}BRESP 0
    if {$stall} {step $stall; check ${ax}BVALID 1; drive ${ax}BREADY 1}
    step 20
}
proc rover::axi_read {addr {expected {}} {stall 0}} {
    variable ax
    drive ${ax}ARADDR $addr; drive ${ax}ARVALID 1
    drive ${ax}RREADY 0
    wait_high ${ax}ARREADY
    step 10; drive ${ax}ARVALID 0
    wait_high ${ax}RVALID
    set got [value ${ax}RDATA]
    check ${ax}RRESP 0
    if {$expected ne {}} {check ${ax}RDATA $expected}
    if {$stall} {step $stall; check ${ax}RVALID 1; check ${ax}RDATA $got}
    drive ${ax}RREADY 1; step 20
    return $got
}

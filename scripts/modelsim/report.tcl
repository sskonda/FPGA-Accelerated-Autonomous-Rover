# Called by the GUI after the simulations. The resulting Tcl is both a machine
# readable coverage inventory and a standalone, printable summary.
proc rover::write_coverage {} {
    variable root; variable build; variable results; variable check_results
    variable compile_status; variable checks; variable failures
    # Recovery can replay an already captured case. Keep the most recent image
    # and case result; check counts still describe actual executions.
    set unique {}
    set seen [dict create]
    foreach r [lreverse $results] {
        set key {}
        if {[dict exists $r screenshot]} {set key image:[file tail [dict get $r screenshot]]}
        if {[dict exists $r case]} {set key case:[dict get $r case]}
        if {$key ne {} && [dict exists $seen $key]} {continue}
        if {$key ne {}} {dict set seen $key 1}
        lappend unique $r
    }
    set results [lreverse $unique]
    if {[info exists ::env(ROVER_SIM_VERSION)]} {set version $::env(ROVER_SIM_VERSION)} else {set version [exec vsim -version]}
    set parent [exec git -C $root rev-parse HEAD]
    set compiled {}
    foreach {path data} $compile_status {
        set rel [string map [list $root/ {} $build/ {<scratch>/}] $path]
        dict set data diagnostic [string map [list $root/ {} $build/ {<scratch>/}] [dict get $data diagnostic]]
        dict set compiled $rel $data
    }
    set preflight {}
    if {[file exists [file join $build testbench_catalog.tcl]]} {
        source [file join $build testbench_catalog.tcl]
        variable testbench_catalog
        foreach entry $testbench_catalog {
            foreach key {source diagnostic} {
                dict set entry $key [string map [list $root/ {} $build/ {<scratch>/}] [dict get $entry $key]]
            }
            lappend preflight $entry
        }
    }
    set cleaned {}
    set count 0
    foreach r $results {
        if {[dict exists $r screenshot]} {
            incr count
            set shot [dict get $r screenshot]
            set name [file tail $shot]
            set family supplied
            if {[regexp {^(csi2_ecc|csi2_lane|csi2_crc)} $name]} {set family camera}
            if {[regexp {^(axi_|stopped_i2s_registers)} $name]} {set family axi}
            if {[regexp {^(video_|tmds_|vga_|displayport_)} $name]} {set family video}
            if {[regexp {^(audio_|stopped_audio)} $name]} {set family audio}
            if {[regexp {^(pwm_|wheel_|sync_|cdc_|input_glitch_|csi2_deskew_)} $name]} {set family control}
            if {[regexp {^(i2c_|ps2_|adc_|usb_|dpti_|stopped_i2c)} $name]} {set family peripherals}
            set dest [file join screenshots $family $name]
            file mkdir [file join $root screenshots $family]
            if {$shot ne $dest} {
                if {[file exists [file join $root $shot]]} {
                    file rename -force [file join $root $shot] [file join $root $dest]
                } elseif {![file exists [file join $root $dest]]} {error "Missing capture: $shot"}
            }
            dict set r screenshot $dest
        }
        if {[dict exists $r reason]} {dict set r reason [string map [list $root/ {} $build/ {<scratch>/}] [dict get $r reason]]}
        lappend cleaned $r
    }
    set meta [dict create simulator [string trim $version] source_commit $parent utc [::clock format [::clock seconds] -gmt 1 -format {%Y-%m-%dT%H:%M:%SZ}] screenshots $count checks $checks failed_checks $failures]
    set diagnostics {}
    set text {}
    foreach log {simulation_initial.log simulation.log} {
        if {[file exists [file join $build $log]]} {
            set f [open [file join $build $log] r]
            append text [read $f] \n
            close $f
        }
    }
    set lines [split $text \n]
    set context {}
    for {set i 0} {$i<[llength $lines]} {incr i} {
        set line [lindex $lines $i]
        if {[regexp {^# vsim -t} $line]} {set context $line}
        if {[regexp {\*\* (Fatal|Failure|Error):} $line whole level]} {
            set detail [lindex $lines [expr {$i+1}]]
            set entry [dict create design $context level $level message $line detail $detail]
            lappend diagnostics [string map [list $root/ {} $build/ {<scratch>/}] $entry]
        }
    }
    set scope {
        {Directed functional cases and supplied testbenches; not exhaustive state-space coverage.}
        {Check counts describe actual executions, including recovery replays; the gallery keeps the most recent capture for each filename.}
        {Every screenshot is a real capture of the ModelSim Wave window; PNGs are not drawn or synthesized.}
        {Original source files are unchanged. Generated fixtures, libraries, WLFs and transcripts live in external scratch.}
        {Only a digital tri-state IOBUF model is supplied; Xilinx SERDES, PLL, clock buffers, DDR registers and generated FIFO IP are not replaced by stubs.}
        {Configured gamma/contrast/saturation datapath tests hold the internal free-running start net low for 500ns while their actual AXI registers are programmed; separate cold-start runs apply no internal forces.}
        {HLS output checks cover beat count, frame/line markers and known data. They do not establish mathematical pixel accuracy of every transform.}
        {The tb_LM compatibility copy constrains an unconstrained aggregate without changing stimulus or DUT behavior. USB Context units use their compatible VHDL-2002 language mode.}
        {Missing motor_ctrl_4ch RTL, generated board/system modules, vendor IP models and ARM/RP2040 firmware execution prevent full rover/firmware simulation. The four-channel PWM shown is the available Digilent PWM v2, not the missing motor controller.}
    }
    set f [open [file join $root scripts modelsim coverage_results.tcl] w]
    puts $f {# Generated by the ModelSim run. Print with: tclsh scripts/modelsim/coverage_results.tcl}
    foreach {name data} [list simulation_metadata $meta verification_scope $scope source_compilation $compiled testbench_preflight $preflight transaction_results $cleaned signal_checks $check_results runtime_diagnostics $diagnostics] {
        set serialized [list set $name $data]
        regsub -all -line {[ \t]+$} $serialized {} serialized
        puts $f $serialized
    }
    puts $f {if {[file normalize $::argv0] eq [file normalize [info script]]} {
    puts "Simulator: [dict get $simulation_metadata simulator]"
    puts "Screenshots: [dict get $simulation_metadata screenshots]"
    puts "Checks: [dict get $simulation_metadata checks]; failed checks: [dict get $simulation_metadata failed_checks]"
    set c 0
    foreach {p d} $source_compilation {if {[dict get $d compiled]} {incr c}}
    puts "Compiler inputs: $c/[dict size $source_compilation] compiled"
    foreach r $transaction_results {
        if {[dict exists $r reason]} {puts "[dict get $r status]: [dict get $r case] -- [dict get $r reason]"}
    }
    foreach d $runtime_diagnostics {
        if {[dict get $d level] eq "Fatal"} {puts "[dict get $d message]\n[dict get $d detail]"}
    }
}}
    close $f
}

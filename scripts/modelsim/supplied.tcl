# Execute every elaboratable supplied TB, and parameter sweeps for seven ADC
# devices and both SPI directions. Digital IOBUF only; no device timing claims.
proc rover::supplied_one {design duration args} {
    variable t; variable build; variable results
    set name [string tolower [string map {. _} $design]]
    foreach a $args {
        if {[regexp {^-gkZmodID=([0-9]+)$} $a whole id]} {
            append name _adc_variant_$id
        } elseif {[regexp {^-gkErrorType=([0-9]+)$} $a whole id]} {
            append name _ [lindex {valid_protocol clock_period_violation setup_time_violation data_hold_violation clock_hold_and_width_violations clock_period_and_width_violations extra_command_bit} $id]
        } elseif {[regexp {^-gkCmdRdWr='([01])'$} $a whole direction]} {
            append name _ [lindex {write read} $direction]
        } else {
            append name _ [regsub -all {[^A-Za-z0-9_]} [string map {-g {} = _} $a] _]
        }
    }
    # The device-model TB intentionally injects setup/hold/clock errors. Let
    # every injection execute; assertion messages remain visible in the audit.
    set ::BreakOnAssertion 4
    set offset [file size [file join $build simulation.log]]
    begin $design "Supplied TB: $design (digital IOBUF model where needed)" @auto {*}$args
    set status supplied_testbench_observed
    if {[catch {step $duration} msg]} {set status stopped_early; puts "SUPPLIED $design: $msg"}
    set f [open [file join $build simulation.log] r]
    seek $f $offset start
    set text [read $f]
    close $f
    set assertions {}
    foreach line [split $text \n] {
        if {[regexp {\*\* (Failure|Error|Fatal):} $line]} {lappend assertions $line}
    }
    if {[llength $assertions]} {
        if {[string match *SPI_Model* $design]} {set status error_injection_observed} else {set status assertion_reported}
        lappend results [dict create case "$design $args" status $status assertion_messages $assertions]
    }
    if {$t>0} {
        snap ${name}_overview 0 $t $status
        if {$duration>=10000} {
            snap ${name}_initial_transaction 0 [expr {min(4000,$t)}] $status
            if {[string match *.tb_TestSPI $design]} {
                snap ${name}_read_transaction 3000 [expr {min(8000,$t)}] $status
            }
            if {[string match *ConfigRelay* $design]} {
                for {set start 0;set i 0} {$start<$t} {incr start 20000;incr i} {
                    snap ${name}_relay_configuration_$i $start [expr {min($start+20000,$t)}] $status
                }
            }
        }
    }
    set ::BreakOnAssertion 3
}
proc rover::supplied_tests {} {
    variable build; variable results
    source [file join $build testbench_catalog.tcl]
    variable testbench_catalog
    foreach entry $testbench_catalog {
        set design [dict get $entry design]
        if {[dict get $entry status] ne "ready"} {
            # tb_LM is enabled by the constrained-aggregate compatibility copy.
            if {![string match *.tb_LM $design]} {
                lappend results [dict create case $design status blocked reason {Original testbench fails compilation or has missing generated IP; see preflight audit in coverage_results.tcl}]
                continue
            }
        }
        if {[string match *.tb_ECC $design]} {continue}
        set duration 20000
        # The supplied relay TB explicitly selects its 399-cycle simulation
        # timers. 250us covers its complete configuration sequence.
        if {[string match *ConfigRelay* $design]} {set duration 250000}
        case_run $design {supplied_one $design $duration}
    }
    foreach lib {zmodscopecontroller zmoddigitizercontroller zmodsdrcontroller} {
        for {set id 0} {$id<7} {incr id} {
            case_run ${lib}_spi_device_$id {supplied_one $lib.tb_TestSPI 12000 -gkZmodID=$id}
        }
        foreach direction {'0' '1'} {
            for {set err 0} {$err<=6} {incr err} {
                case_run ${lib}_spi_error_${direction}_$err {
                    supplied_one $lib.tb_TestAD96xx_92xxSPI_Model 12000 -gkErrorType=$err -gkCmdRdWr=$direction
                }
            }
        }
    }
}

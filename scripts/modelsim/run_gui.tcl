# Internal ModelSim GUI entry point. run_all.tcl compiles before launching this.
set here [file dirname [file normalize [info script]]]
source [file join $here common.tcl]
source [file join $::rover::build compile_status.tcl]
transcript file [file join $::rover::build simulation.log]
set suites {camera buses control video audio peripherals supplied}
if {[info exists ::env(ROVER_SIM_SUITES)]} {set suites $::env(ROVER_SIM_SUITES)}
foreach suite $suites {
    if {[file exists [file join $here ${suite}.tcl]]} {
        source [file join $here ${suite}.tcl]
        rover::case_run $suite [list rover::${suite}_tests]
    }
}
set f [open [file join $::rover::build results.tcl] w]
puts $f [list set results $::rover::results]
puts $f [list set checks $::rover::checks]
puts $f [list set failures $::rover::failures]
puts $f [list set check_results $::rover::check_results]
close $f
source [file join $here report.tcl]
if {[catch {rover::write_coverage} msg]} {
    puts "COVERAGE WRITE ERROR: $msg"
    quit -f -code 1
}
puts "COMPLETE: [llength $::rover::results] results, $::rover::checks checks, $::rover::failures failed checks"
quit -f

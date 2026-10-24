# Recovery used after the host interrupted a PNG capture and preference write.
# Preserves completed results, replays that one frame, then runs remaining suites.
# Usage: ROVER_SIM_BUILD=<original scratch> tclsh scripts/modelsim/resume_capture.tcl
set here [file dirname [file normalize [info script]]]
if {![llength [info commands vsim]]} {
    set ::env(ROVER_SIM_VERSION) [exec vsim -version]
    cd $::env(ROVER_SIM_BUILD)
    exec vsim -gui -do [list source [file join $here resume_capture.tcl]] >[file join $::env(ROVER_SIM_BUILD) resumed_stdout.log] 2>@1
    exit
}
source [file join $here common.tcl]
source [file join $::rover::build compile_status.tcl]
source [file join $::rover::build partial_results.tcl]
set ::rover::results $results
set ::rover::checks $checks
set ::rover::failures $failures
set ::rover::check_results $check_results
transcript file [file join $::rover::build simulation.log]
set category {HLS scaler_vhdl - 8x4 frame, startup_hold=1 stall=0 gaps=1}
set kept {}
foreach r $::rover::results {
    if {([dict exists $r category] && [dict get $r category] eq $category) || ([dict exists $r case] && [dict get $r case] eq "video_scaler_vhdl_0_1")} {
        if {[dict exists $r screenshot]} {
            set old [file join $::rover::root [dict get $r screenshot]]
            if {[file exists $old]} {file delete $old}
        }
    } else {lappend kept $r}
}
set ::rover::results $kept
set kept {}
foreach c $::rover::check_results {
    if {[dict get $c category] eq $category} {
        incr ::rover::checks -1
        if {![dict get $c passed]} {incr ::rover::failures -1}
    } else {lappend kept $c}
}
set ::rover::check_results $kept
source [file join $here video.tcl]
rover::case_run video_scaler_vhdl_0_1 {rover::hls_frame scaler_vhdl video_scaler_vhdl.video_scaler 8 4 {} 0 1}
foreach suite {audio peripherals supplied} {
    source [file join $here ${suite}.tcl]
    rover::case_run $suite [list rover::${suite}_tests]
}
rover::checkpoint
source [file join $here report.tcl]
rover::write_coverage
puts "COMPLETE: $::rover::checks checks; $::rover::failures failed checks"
quit -f

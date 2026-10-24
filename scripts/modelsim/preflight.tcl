# Discover and elaborate every supplied HDL testbench. Never accept an unbound
# component as a simulated DUT. Blockers are saved outside Git for the GUI run.
proc rover::preflight {} {
    variable build; variable libraries; variable compile_status; variable root
    set catalog {}
    cd $build
    foreach {lib files} $libraries {
        foreach src $files {
            if {[string first $root/ $src] != 0} {continue}
            set f [open $src r]; set text [read $f]; close $f
            set tops {}
            if {[file extension $src] in {.v .sv}} {
                set matches [regexp -all -inline -nocase {module\s+([a-zA-Z0-9_]*(?:testbench|_tb))\s*(?:\(|;)} $text]
            } else {
                set matches [regexp -all -inline -nocase {entity\s+(tb_[a-zA-Z0-9_]+)\s+is} $text]
            }
            foreach {whole top} $matches {
                set design $lib.$top
                set logfile [file join $build preflight_${lib}_${top}.log]
                set code [catch {exec timeout 25s vsim -c -l $logfile -wlf [file join $build preflight.wlf] -L unisim $design -do {set NumericStdNoWarnings 1; set StdArithNoWarnings 1; run 0ns; quit -f} 2>@1} msg]
                set status ready
                if {$code || [regexp {\*\* (Error|Fatal):|not bound|not found|has no architecture|Failed to (find|open)} $msg]} {set status blocked}
                lappend catalog [dict create design $design source $src status $status diagnostic $msg]
                puts "PREFLIGHT $design $status"
            }
        }
    }
    set f [open [file join $build testbench_catalog.tcl] w]
    puts $f [list set ::rover::testbench_catalog $catalog]
    close $f
}

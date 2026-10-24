# Usage: tclsh scripts/modelsim/run_all.tcl
# Requires ModelSim/Questa, a working X11 display and ImageMagick `import`.
# Optional ROVER_SIM_BUILD: external scratch directory; ROVER_SIM_SUITES:
# space-separated suites (camera buses control video audio peripherals supplied).
# This is directed functional simulation, not exhaustive state-space coverage.
# Only Tcl and PNG files are added to the repository. Original RTL is preserved.
set here [file dirname [file normalize [info script]]]
source [file join $here compile.tcl]
set ::env(ROVER_SIM_BUILD) $::rover::build
set ::env(ROVER_SIM_VERSION) [exec vsim -version]
rover::compile_all
source [file join $here benches.tcl]
rover::generate_benches
source [file join $here preflight.tcl]
rover::preflight
cd $::rover::build
puts "Running ModelSim GUI; scratch files: $::rover::build"
exec vsim -gui -do [list source [file join $here run_gui.tcl]] >[file join $::rover::build gui_stdout.log] 2>@1

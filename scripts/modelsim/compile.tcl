# Run with tclsh, outside ModelSim. run_all.tcl invokes this automatically.
# All compiled libraries, generated HDL, transcripts and WLFs live outside Git.
namespace eval rover {
    variable root [file normalize [file join [file dirname [info script]] ../..]]
    variable build
    if {[info exists ::env(ROVER_SIM_BUILD)]} {
        set build [file normalize $::env(ROVER_SIM_BUILD)]
    } else {
        set build [file normalize [file join /tmp rover-modelsim-[pid]]]
    }
    file mkdir $build
    variable compile_status [dict create]
    variable libraries [dict create]
}
proc rover::tool {name args} {
    # The host's proot wrapper breaks VHDL STANDARD source reads. Invoke the
    # unchanged vendor compiler through its bundled ELF loader when available.
    set base [file normalize ~/intelFPGA/20.1]
    set loader $base/modelsim_compat/usr/lib/i386-linux-gnu/ld-linux.so.2
    set binary $base/modelsim_ase/linux/$name
    if {[file exists $loader] && [file exists $binary]} {
        set ld $base/modelsim_compat/usr/lib/i386-linux-gnu:$base/modelsim_compat/lib/i386-linux-gnu
        return [exec env LD_LIBRARY_PATH=$ld $loader $binary {*}$args 2>@1]
    }
    return [exec $name {*}$args 2>@1]
}
proc rover::compile_group {lib files} {
    variable build
    variable compile_status
    variable libraries
    set dir [file join $build $lib]
    if {![file exists $dir]} {tool vlib $dir}
    tool vmap $lib $dir
    dict set libraries $lib $files
    set remaining $files
    # Multiple passes resolve direct entity and package dependencies. Failures
    # are retained in the audit; no unbound black boxes count as simulations.
    for {set pass 0} {$pass < 8 && [llength $remaining]} {incr pass} {
        set pending {}
        set progressed 0
        foreach f $remaining {
            if {[string tolower [file extension $f]] in {.v .sv}} {
                set cmd [list vlog -sv -work $lib $f]
            } else {
                set edition -2008
                # USB Context is an identifier in this older source; VHDL-2008
                # made it a keyword. Compile those two original units as 2002.
                if {$lib eq "usb2device_v1_0" && [file tail $f] in {Context.vhd DMA_Transfer_Manager.vhd}} {set edition -2002}
                set cmd [list vcom $edition -explicit -work $lib $f]
            }
            set code [catch {tool {*}$cmd} msg]
            set log [open [file join $build compile.log] a]
            puts $log $msg
            close $log
            dict set compile_status $f [dict create compiled [expr {!$code}] diagnostic $msg]
            if {$code} {lappend pending $f} else {incr progressed}
        }
        set remaining $pending
        if {!$progressed} {break}
    }
    puts "$lib: [expr {[llength $files]-[llength $remaining]}]/[llength $files] sources compiled"
}
proc rover::walk_hdl {dir} {
    set found {}
    foreach p [glob -nocomplain -directory $dir *] {
        if {[file isdirectory $p]} {
            if {[file tail $p] ni {example_designs vhdl .git}} {
                lappend found {*}[walk_hdl $p]
            }
        } elseif {[string tolower [file extension $p]] in {.v .sv .vhd .vhdl}} {
            lappend found $p
        }
    }
    return [lsort $found]
}
proc rover::compile_all {} {
    variable root
    variable build
    variable libraries
    variable compile_status
    cd $build
    if {![file exists modelsim.ini]} {tool vmap -c}
    # Digital tri-state buffer only. This is not the Xilinx timing library.
    # Used by the supplied Zmod SPI/I2C testbenches. No SERDES/PLL/FIFO stubs.
    set f [open [file join $build iobuf.vhd] w]
    puts $f {library ieee; use ieee.std_logic_1164.all;
package vcomponents is
 component IOBUF generic(DRIVE:integer:=12; IOSTANDARD:string:="DEFAULT"; SLEW:string:="SLOW");
 port(O:out std_logic; IO:inout std_logic; I:in std_logic; T:in std_logic); end component;
end package;
library ieee; use ieee.std_logic_1164.all;
entity IOBUF is
 generic(DRIVE:integer:=12; IOSTANDARD:string:="DEFAULT"; SLEW:string:="SLOW");
 port(O:out std_logic; IO:inout std_logic; I:in std_logic; T:in std_logic);
end; architecture digital_only of IOBUF is begin
 IO <= I when T='0' else 'Z' when T='1' else 'X'; O <= IO;
end;}
    close $f
    compile_group unisim [list [file join $build iobuf.vhd]]
    # Some Digilent sources declare this library without importing any units.
    if {![file exists [file join $build digilent]]} {tool vlib [file join $build digilent]}
    tool vmap digilent [file join $build digilent]
    set ip [file join $root vivado-library-master ip]
    foreach p [lsort [glob -directory $ip *]] {
        if {![file isdirectory $p]} {continue}
        set name [file tail $p]
        if {$name in {Pmods Zmods}} {
            foreach sub [lsort [glob -directory $p *]] {
                if {![file isdirectory $sub]} {continue}
                set lib [string tolower [regsub -all {[^A-Za-z0-9_]} [file tail $sub] _]]
                set files [walk_hdl $sub]
                if {[llength $files]} {compile_group $lib $files}
            }
        } else {
            set lib [string tolower [regsub -all {[^A-Za-z0-9_]} $name _]]
            set files [walk_hdl $p]
            if {[llength $files]} {compile_group $lib $files}
            set vh [file join $p hdl vhdl]
            if {[file isdirectory $vh]} {compile_group ${lib}_vhdl [walk_hdl $vh]}
        }
    }
    compile_group synchronizers [walk_hdl [file join $root vivado-library-master module synchronizers]]
    compile_group board_imports [glob -directory [file join $root Vivado_IPs_Used] *.vhd]
    # Include the board wrapper and duplicate imports in the audit as well.
    set board [file join $root HDMI_Camera_Passthrough Vivado_Project2_CV Zybo-Z7-HW Zybo-Z7-HW.srcs sources_1 imports hdl]
    compile_group board_wrapper [walk_hdl $board]
    foreach src [glob -nocomplain -directory $ip */example_designs/bfm_design/*_tb.v] {
        set lib bfm_[string tolower [regsub -all {[^A-Za-z0-9_]} [file tail [file dirname [file dirname [file dirname $src]]]] _]]
        compile_group $lib [list $src]
    }
    set f [open [file join $build compile_status.tcl] w]
    puts $f [list set ::rover::libraries $libraries]
    puts $f [list set ::rover::compile_status $compile_status]
    close $f
}
if {[file normalize $::argv0] eq [file normalize [info script]]} {
    rover::compile_all
    puts "BUILD=$::rover::build"
}

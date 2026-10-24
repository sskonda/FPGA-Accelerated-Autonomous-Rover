# Frame stimulus is generated as HDL by benches.tcl, avoiding GUI IPC per cycle.
proc rover::hls_frame {name design width height registers {stall 0} {gaps 0} {hold 1}} {
    variable libraries; variable build; variable checks; variable failures
    set lib [lindex [split $design .] 0]
    foreach src [dict get $libraries $lib] {
        foreach dat [glob -nocomplain -directory [file dirname $src] *.dat] {
            file copy -force $dat [file join $build [file tail $dat]]
        }
    }
    set base [lindex [split $name _] 0]
    begin rover_video_benches.${base}_frame_tb "HLS $name - 8x4 frame, startup_hold=$hold stall=$stall gaps=$gaps" {clk resetn awaddr awvalid wvalid awready wready in_data in_user in_last in_valid in_ready out_data out_user out_last out_valid out_ready in_count out_count frame_starts line_ends unknown_words done timed_out} -L $lib -gSTALL=$stall -gGAPS=$gaps
    # Verification fixture: hold only the free-running dataflow start net while
    # the real AXI endpoint is programmed. This is a datapath unit test, not a
    # claim that cold boot through external ports succeeds. The separate cold
    # start run below uses no internal forces and records the startup stall.
    if {$hold && $base ne "scaler"} {
        set startnet Block_Mat_exit1573_p_U0_ap_start
        if {$base eq "gamma"} {set startnet Block_Mat_exit570_pr_U0_ap_start}
        force -freeze [path dut/$startnet] 0 0ns, 1 500ns
    }
    step 100000
    set state protocol_checked
    if {$hold || $base eq "scaler"} {
        foreach {s expected} {in_count 32 out_count 32 frame_starts 1 line_ends 4 unknown_words 0} {
            if {![check $s $expected]} {set state check_failed}
        }
    } else {set state cold_start_observed}
    set suffix configured_contiguous
    if {$stall} {set suffix configured_output_backpressure}
    if {$gaps} {set suffix configured_input_gaps}
    if {!$hold} {set suffix cold_start}
    set count [value out_count]
    set end 10000
    if {[value done] ne "1"} {set end 100000; if {$hold} {set state timed_out}}
    snap video_${name}_8x4_$suffix 0 $end $state
    for {set i 0} {$i<$count && $i<32} {incr i} {
        set time [value [format {out_time[%d]} $i]]
        snap [format video_%s_%s_output_pixel_%02d $name $suffix $i] [expr {$time-20}] [expr {$time+30}] $state
    }
}
proc rover::video_tests {} {
    variable t
    case_run tmds {
        begin rgb2dvi.TMDS_Encoder {HDMI TMDS encoder - all control symbols and 256 pixel values} {PixelClk SerialClk aRst pDataOut pC0 pC1 pVde pDataOutRaw cnt_t_3}
        clock PixelClk 10; clock SerialClk 2; drive aRst 1; drive pDataOut 0; drive pC0 0; drive pC1 0; drive pVde 0; step 40
        drive aRst 0
        foreach {ctl token} {0 852 1 171 2 340 3 683} {
            set start $t; drive pC0 [expr {$ctl&1}]; drive pC1 [expr {($ctl>>1)&1}]; step 60
            check pDataOutRaw $token
            snap tmds_control_symbol_$ctl $start {} checked
        }
        drive pVde 1
        for {set base 0} {$base<256} {incr base 16} {
            set start $t
            for {set val $base} {$val<$base+16} {incr val} {drive pDataOut $val; step 10}
            step 40
            snap [format tmds_pixel_values_%02x_to_%02x $base [expr {$base+15}]] $start
        }
        set start $t; drive pVde 0; step 80; snap tmds_active_video_to_blanking $start
    }
    case_run vga {
        begin rgb2vga_v1_0.rgb2vga {RGB to VGA - RGB565 colors, blanking and sync} {PixelClk rgb_pData rgb_pVDE rgb_pHSync rgb_pVSync vga_pRed vga_pGreen vga_pBlue vga_pHSync vga_pVSync}
        clock PixelClk; drive rgb_pVDE 1; drive rgb_pHSync 0; drive rgb_pVSync 0
        foreach {name rgb} {black 0 white 16777215 red 16711680 green 255 blue 65280 gray 8421504} {
            set start $t; drive rgb_pData $rgb; step 30
            snap vga_rgb565_color_$name $start
        }
        set start $t; drive rgb_pVDE 0; step 30; check vga_pRed 0; check vga_pGreen 0; check vga_pBlue 0
        snap vga_blanking_black $start {} checked
        foreach {name h v} {horizontal_sync 1 0 vertical_sync 0 1 combined_sync 1 1} {
            set start $t; drive rgb_pHSync $h; drive rgb_pVSync $v; step 30
            check vga_pHSync $h; check vga_pVSync $v
            snap vga_$name $start {} checked
        }
    }
    case_run displayport {
        begin rgb2dpvid_v1_0.rgb2dpvid {RGB to DisplayPort video mapping} {PixelClk pData pHSync pVSync pVde pVidPixel0 pVidHSync pVidVSync pVidEnable pVidOddEven pVidRst}
        clock PixelClk; drive pHSync 0; drive pVSync 0; drive pVde 1
        foreach word {0 16777215 16711680 65280 255 1193046} {
            set start $t; drive pData $word; step 30
            check pVidEnable 1
            snap displayport_rgb_word_$word $start {} checked
        }
        set start $t; drive pVde 0; drive pHSync 1; drive pVSync 1; step 30
        check pVidEnable 0; check pVidHSync 1; check pVidVSync 1
        snap displayport_blanking_and_sync $start {} checked
    }
    foreach {name design regs} {
        gamma hls_gamma_correction_1_0.hls_gamma_correction {16 10 24 4 32 8}
        contrast hls_contrast_stretch_1_0.hls_contrast_stretch {16 4 24 8 32 0 40 255}
        saturation hls_saturation_enhance_1_0.hls_saturation_enhance {16 4 24 8 32 128}
        scaler video_scaler.video_scaler {16 8 24 4 32 8 40 4}
    } {
        if {$name ne "scaler"} {
            case_run video_${name}_cold_start {hls_frame $name $design 8 4 $regs 0 0 0}
        }
        foreach implementation {verilog vhdl} {
            set target $design
            set label $name
            if {$implementation eq "vhdl"} {
                set lib [lindex [split $design .] 0]
                set target ${lib}_vhdl.[lindex [split $design .] 1]
                set label ${name}_vhdl
            }
            foreach {stall gaps} {0 0 1 0 0 1} {
                case_run video_${label}_${stall}_${gaps} {hls_frame $label $target 8 4 $regs $stall $gaps}
            }
        }
    }
}

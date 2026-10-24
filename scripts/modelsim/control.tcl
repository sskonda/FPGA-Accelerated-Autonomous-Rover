proc rover::control_tests {} {
    variable t
    case_run pwm_four_channels {
        begin pwm_2_0.PWM_v2_0 {PWM v2 - four independent channels (not the missing motor_ctrl_4ch)} {pwm_axi_aclk pwm_axi_aresetn pwm_axi_awaddr pwm_axi_awvalid pwm_axi_awready pwm_axi_wdata pwm_axi_wvalid pwm_axi_wready pwm_axi_bvalid pwm count max enable period_reg} -gNUM_PWM=4
        axi_init pwm_axi_ pwm_axi_aclk pwm_axi_aresetn
        axi_write 8 9
        foreach {a v} {64 0 68 2 72 5 76 10} {axi_write $a $v}
        axi_write 0 1
        step 250
        set start $t
        set highs {0 0 0 0}
        for {set n 0} {$n<10} {incr n} {
            set v [value pwm]
            for {set ch 0} {$ch<4} {incr ch} {
                lset highs $ch [expr {[lindex $highs $ch]+(($v>>$ch)&1)}]
            }
            step 10
        }
        variable checks; variable failures
        incr checks
        set state checked
        if {$highs ne {0 2 5 10}} {incr failures; set state check_failed; puts "PWM high counts: $highs"}
        snap pwm_v2_four_channels_0_20_50_100_percent $start {} $state
        foreach duty {0 1 5 9 10 12} {
            set start $t
            axi_write 64 $duty
            step 400
            snap pwm_v2_channel0_duty_${duty}_period9 $start
        }
        set start $t; axi_write 0 0; step 100; check pwm 0
        snap pwm_v2_disabled_all_channels_low $start {} checked
        set start $t; axi_write 0 1; axi_write 8 19; step 700
        snap pwm_v2_period_change_at_boundary $start
        set start $t; drive pwm_axi_aresetn 0; step 40; drive pwm_axi_aresetn 1; step 100
        snap pwm_v2_reset_while_running $start
    }
    case_run pwm_original {
        begin pwm_1_0.PWM_v1_0 {PWM v1 - original output polarity and interrupt} {pwm_axi_aclk pwm_axi_aresetn pwm_axi_wdata pwm count max enable interrupt duty_reg duty_reg_latch}
        axi_init pwm_axi_ pwm_axi_aclk pwm_axi_aresetn
        axi_write 4 9; axi_write 8 1
        foreach duty {0 1 4 8 9 10} {
            set start $t; axi_write 0 $duty; step 400
            snap pwm_v1_original_polarity_duty_${duty}_period9 $start
        }
    }
    case_run feedback_counter {
        begin motorfeedback_1_0.posCounter {Wheel feedback counter - edge counting, clears, subtraction, wrap} {clk sensor clear subtract distance pos1 pos2}
        clock clk; drive sensor 0; drive clear 3; drive subtract 0; drive distance 3; step 20
        drive clear 0
        for {set n 1} {$n<=6} {incr n} {
            set start $t; drive sensor 1; step 30; drive sensor 0; step 20
            check pos1 $n; check pos2 $n
            snap wheel_feedback_rising_edge_$n $start {} checked
        }
        set start $t; drive subtract 1; step 10; drive subtract 0; step 10
        check pos2 3; snap wheel_feedback_subtract_distance $start {} checked
        set start $t; drive clear 1; step 10; drive clear 0; step 10
        check pos1 0; check pos2 3; snap wheel_feedback_clear_speed_only $start {} checked
        set start $t; drive clear 2; step 10; drive clear 0; step 10
        check pos2 0; snap wheel_feedback_clear_position_only $start {} checked
        set start $t; drive subtract 1; step 10; drive subtract 0; step 10
        check pos2 65533; snap wheel_feedback_subtraction_underflow $start {} checked
        drive clear 3; step 10; drive clear 0
        clock sensor 20; step 1310680
        set start $t; step 100
        snap wheel_feedback_16bit_counter_wrap $start
        noforce [path sensor]; drive sensor 0
    }
    case_run wheel_pair {
        begin motorfeedback_1_0.posManager {Two wheel feedback - matched, unequal and stopped wheels} {clk m1 m2 clear pos11 pos12 pos21 pos22 pos_diff count_clk}
        clock clk; drive clear 3; drive m1 0; drive m2 0; step 30; drive clear 0
        foreach {name p1 p2} {matched 40 40 left_faster 40 80 right_faster 80 40} {
            set start $t; clock m1 $p1; clock m2 $p2; step 800
            snap wheel_pair_$name $start
        }
        set start $t; noforce [path m1]; drive m1 0; step 400
        snap wheel_pair_left_stopped $start
        set start $t; drive clear 3; step 30; check pos_diff 0
        snap wheel_pair_clear_both $start {} checked
    }
    case_run pulse_length {
        begin pwm_analyzer_1_0.pulseLength {PWM pulse measurement - duty, period, stuck input, reset} {clk reset pwm pw_low pw_high pwm_per ps}
        clock clk; drive reset 1; drive pwm 0; step 30; drive reset 0
        foreach {name high low} {quarter 40 120 half 80 80 three_quarters 120 40 short_high 10 150 short_low 150 10} {
            set start $t
            for {set n 0} {$n<4} {incr n} {drive pwm 1; step $high; drive pwm 0; step $low}
            snap pwm_measurement_$name $start
        }
        set start $t; drive pwm 1; step 400; snap pwm_measurement_stuck_high $start
        set start $t; drive pwm 0; step 400; snap pwm_measurement_stuck_low $start
        set start $t; drive reset 1; step 30; drive reset 0; step 30
        snap pwm_measurement_reset $start
    }
    foreach stages {2 3 4} {
        case_run synchronizer_$stages {
            begin synchronizers.SyncAsync "Asynchronous input synchronizer - $stages stages" {OutClk aoReset aIn oOut oSyncStages} -gkStages=$stages
            clock OutClk; drive aoReset 1; drive aIn 0; step 30
            drive aoReset 0; step 30
            foreach {name v} {rising 1 falling 0} {
                set start $t; drive aIn $v; step 70; check oOut $v
                snap sync_${stages}_stages_$name $start {} checked
            }
            set start $t; drive aIn 1; step 70; step 2; drive aoReset 1; step 1
            check oOut 0; step 27
            snap sync_${stages}_stages_async_reset $start {} checked
        }
    }
    case_run handshake_cdc {
        begin synchronizers.HandshakeData {Clock domain handshake - unrelated clocks, acknowledge stall} {InClk OutClk aiReset aoReset iData iPush iRdy oData oValid oAck}
        clock InClk 10; clock OutClk 14
        drive aiReset 1; drive aoReset 1; drive iPush 0; drive iData 0; drive oAck 1; step 70
        drive aiReset 0; drive aoReset 0; step 100
        foreach word {0 85 170 255 60} {
            set start $t; drive iData $word; drive iPush 1; step 10; drive iPush 0
            step 140; check oData $word
            snap cdc_handshake_data_$word $start {} checked
        }
        set start $t; drive oAck 0; drive iData 90; drive iPush 1; step 10; drive iPush 0; step 140
        snap cdc_handshake_acknowledge_stalled $start
        set start $t; drive oAck 1; step 140; check iRdy 1
        snap cdc_handshake_acknowledge_released $start {} checked
    }
    case_run glitch_filter {
        begin dvi2rgb.GlitchFilter {Input glitch filter - rejection and stable transitions} {SampleClk sRst sIn sOut cntPeriods} -gkNoOfPeriodsToFilter=4
        clock SampleClk; drive sRst 1; drive sIn 0; step 30; drive sRst 0; step 70
        foreach width {10 20 30 70} {
            set start $t; drive sIn 1; step $width; drive sIn 0; step 90
            check sOut 0; snap input_glitch_filter_pulse_${width}ns $start {} checked
        }
        set start $t; drive sIn 1; step 90; check sOut 1
        snap input_glitch_filter_stable_high $start {} checked
    }
    case_run csi_fifo {
        begin mipi_csi_2_rx.SimpleFIFO {CSI lane deskew FIFO - original unusual pointer behavior} {InClk iRst iDataIn iWrEn iRdEn iFull iEmpty iDataOut iRdA iWrA} -gkDataWidth=8
        clock InClk; drive iRst 1; drive iDataIn 0; drive iWrEn 0; drive iRdEn 0; step 30
        drive iRst 0; step 20
        set start $t
        drive iWrEn 1
        for {set i 0} {$i<40} {incr i} {drive iDataIn $i; step 10}
        drive iWrEn 0; step 20
        snap csi2_deskew_fifo_fill_and_pointer_wrap $start
        set start $t; drive iRdEn 1; step 400; drive iRdEn 0; step 20
        snap csi2_deskew_fifo_drain_and_empty $start
        set start $t; drive iWrEn 1; drive iRdEn 1; drive iDataIn 165; step 100
        snap csi2_deskew_fifo_simultaneous_read_write $start
    }
}

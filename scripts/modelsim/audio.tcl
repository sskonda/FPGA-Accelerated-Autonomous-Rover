proc rover::audio_tests {} {
    variable t
    foreach width {8 16 24} {
        case_run audio_loopback_$width {
            begin rover_benches.audio_loopback "I2S $width bit TX/RX loopback - real serializer, receiver, clock generator" {clk resetn enable tick bclk lrclk channel_sync frame_sync txdata serial txack rxstb rxack rxdata} -gslot_width=$width
            drive resetn 0; drive enable false; drive txdata 0; drive rxack 1; step 100
            drive resetn 1; drive enable true
            foreach word [list 0 [expr {1<<($width-1)}] [expr {(1<<($width-1))-1}] [expr {(1<<$width)-1}] [expr {0x555555&((1<<$width)-1)}] [expr {0xAAAAAA&((1<<$width)-1)}]] {
                set start $t; drive txdata $word; step 8000
                check rxdata $word
                snap audio_i2s_${width}bit_word_$word [expr {$t-4000}] {} checked
            }
            set start $t; drive rxack 0; step 5000
            snap audio_i2s_${width}bit_receiver_backpressure $start
            set start $t; drive rxack 1; step 5000
            snap audio_i2s_${width}bit_receiver_resume $start
            set start $t; drive tick 0; step 200; drive tick 1; step 1000
            snap audio_i2s_${width}bit_clock_tick_pause $start
            set start $t; drive enable false; step 200; check serial 0
            snap audio_i2s_${width}bit_disable_mutes_output $start {} checked
            set start $t; drive resetn 0; step 100; drive resetn 1; drive enable true; step 4000
            snap audio_i2s_${width}bit_reset_and_restart $start
        }
    }
    case_run audio_clock_dividers {
        begin axi_i2s_adi_1_2.i2s_clkgen {I2S bit and frame clock divider programming} {clk resetn enable tick bclk_div_rate lrclk_div_rate bclk lrclk channel_sync frame_sync bclk_count lrclk_count}
        clock clk; drive resetn 0; drive enable false; drive tick 1; drive bclk_div_rate 1; drive lrclk_div_rate 7; step 30
        drive resetn 1; drive enable true
        foreach div {0 1 3 7} {
            set start $t; drive resetn 0; drive bclk_div_rate $div; step 30
            drive resetn 1; step 2000
            snap audio_i2s_bit_clock_divider_$div $start
        }
        set start $t; drive bclk_div_rate 0; step 3000
        snap audio_i2s_live_divider_decrease $start
    }
}

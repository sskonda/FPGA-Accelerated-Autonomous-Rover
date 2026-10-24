proc rover::i2c_start {} {
    drive SDA_I 1; drive SCL_I 1; step 1250
    drive SDA_I 0; step 1250; drive SCL_I 0; step 1250
}
proc rover::i2c_stop {} {
    drive SCL_I 0; drive SDA_I 0; step 1250
    drive SCL_I 1; step 1250; drive SDA_I 1; step 1250
}
proc rover::i2c_byte {byte {ack 1}} {
    for {set bit 7} {$bit>=0} {incr bit -1} {
        drive SCL_I 0; drive SDA_I [expr {($byte>>$bit)&1}]; step 1250
        drive SCL_I 1; step 1250; drive SCL_I 0; step 1250
    }
    drive SDA_I 1; step 1250; drive SCL_I 1; step 1250
    check SDA_T [expr {!$ack}]
    drive SCL_I 0; step 1250
}
proc rover::peripherals_tests {} {
    variable t
    foreach {label ack stretch} {write_ack 1 0 write_nack 0 0 clock_stretch 1 1} {
        case_run i2c_master_$label {
            begin rover_i2c_benches.i2c_master_tb "I2C master $label - original TWI_Ctl with digital slave fixture" {clk rst stb scl sda scl_t sda_t slave_drive done error completion_error bytes last_received finished} -L zmoddigitizercontroller -gACK=$ack -gSTRETCH=$stretch
            step 500000
            check finished 1
            check completion_error [expr {!$ack}]
            if {$ack} {check last_received 90}
            snap i2c_master_$label 0 250000 checked
        }
    }
    case_run i2c_slave {
        begin dvi2rgb.TWI_SlaveCtl {I2C slave - addressing, writes, ACK/NACK and repeated START} {SampleClk SRST SDA_I SCL_I SDA_O SDA_T D_I D_O STB_I DONE_O END_O RD_WRN_O state bitCount}
        clock SampleClk; drive SRST 1; drive SDA_I 1; drive SCL_I 1; drive STB_I 1; drive D_I 165; step 100
        drive SRST 0; step 200
        foreach byte {0 255 165 90} {
            set start $t; i2c_start; i2c_byte 160; i2c_byte $byte
            check D_O $byte; i2c_stop
            snap i2c_slave_write_byte_$byte $start {} checked
        }
        set start $t; i2c_start; i2c_byte 162 0; i2c_stop
        snap i2c_slave_unmatched_address_nack $start {} checked
        set start $t; i2c_start; i2c_byte 160; i2c_byte 18; i2c_start; i2c_byte 161
        drive SDA_I 1
        set got 0
        for {set bit 7} {$bit>=0} {incr bit -1} {
            drive SCL_I 0; step 1250; drive SCL_I 1; step 1250
            set got [expr {($got<<1)|[value SDA_T]}]
            drive SCL_I 0; step 1250
        }
        drive SDA_I 1; step 1250; drive SCL_I 1; step 1250; drive SCL_I 0; step 1250
        i2c_stop
        snap i2c_slave_repeated_start_read_a5_master_nack $start
        puts "I2C read byte observed $got"
    }
    case_run ps2_receiver {
        begin axi_ps2_1_0.Ps2Interface {PS/2 device frames - make, break, extended and parity error} {clk rst PS2_Clk_I PS2_Data_I PS2_Clk_T PS2_Data_T tx_data write_data rx_data read_data ack busy err_par err_nack state bit_count frame}
        clock clk; drive rst 1; drive PS2_Clk_I 1; drive PS2_Data_I 1; drive tx_data 0; drive write_data 0; step 100
        drive rst 0; step 150000
        foreach {name byte bad} {make_1c 28 0 break_prefix_f0 240 0 extended_prefix_e0 224 0 arrow_up_75 117 0 all_zero 0 0 all_one 255 0 bad_parity_a5 165 1} {
            set start $t; set parity 1; set bits {0}
            for {set b 0} {$b<8} {incr b} {set v [expr {($byte>>$b)&1}]; lappend bits $v; set parity [expr {$parity^$v}]}
            lappend bits [expr {$parity^$bad}] 1
            foreach v $bits {
                drive PS2_Data_I $v; drive PS2_Clk_I 1; step 30000
                drive PS2_Clk_I 0; step 30000
            }
            drive PS2_Clk_I 1; drive PS2_Data_I 1; step 60000
            if {!$bad} {check rx_data $byte}
            snap ps2_receive_$name $start {} [expr {$bad ? "error_injection" : "checked"}]
        }
    }
    case_run adc_spi {
        begin rover_adc_benches.adc_spi_tb {Pmod AD1 dual ADC SPI - four complete, checked serial transactions} {clk rst cs sclk sdin0 sdin1 word0 word1 drdy dout0 dout1 tested bad finished} -L pmodad1_v1_0
        step 60000
        check tested 4; check bad 0; check finished 1
        foreach {index label} {0 zero_ffff 1 ffff_zero 2 a55a_5aa5 3 1234_abcd} {
            set start [value [format {start_time[%d]} $index]]
            set end [value [format {end_time[%d]} $index]]
            snap adc_spi_channels_$label [expr {$start-100}] [expr {$end+100}] checked
        }
    }
    case_run usb_crc5 {
        begin usb2device_v1_0.crc5 {USB CRC5 - address/endpoint and SOF token fields} {data_in crc_out}
        foreach address {0 1 64 127} {
            for {set ep 0} {$ep<16} {incr ep} {
                set start $t; drive data_in [expr {$address|($ep<<7)}]; step 20
                snap [format usb_crc5_address_%03d_endpoint_%02d $address $ep] $start
            }
        }
        foreach frame {0 1 1023 2047} {
            set start $t; drive data_in $frame; step 20
            snap usb_crc5_sof_frame_$frame $start
        }
    }
    case_run usb_crc16 {
        begin usb2device_v1_0.crc16 {USB CRC16 - empty, short, alternating and test-string payloads} {clk rst crc_en data_in crc_out_res crc_out}
        clock clk; drive rst 1; drive crc_en 0; drive data_in 0; step 30; drive rst 0
        foreach {name bytes} {empty {} zero {0} ff {255} alternating {85 170 85 170} usb_test_string {49 50 51 52 53 54 55 56 57} incrementing {0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15}} {
            set start $t; drive rst 1; step 10; drive rst 0; drive crc_en 1
            foreach byte $bytes {drive data_in $byte; step 10}
            drive crc_en 0; step 20
            snap usb_crc16_payload_$name $start
        }
        set start $t; set crc [value crc_out_res]; drive data_in 90; step 80; check crc_out_res $crc
        snap usb_crc16_enable_low_holds_state $start {} checked
    }
    case_run camera_crc16 {
        begin mipi_csi_2_rx.CRC16 {CSI-2 CRC16 - 1/2/3/4 valid bytes and enable hold} {ByteClk bRst bData bKeep bDataEnable bCRC} -gkLaneCount=4
        clock ByteClk; drive bRst 1; drive bData 0; drive bKeep 0; drive bDataEnable 0; step 30
        drive bRst 0
        foreach {keep count} {0 0 1 1 3 2 7 3 15 4} {
            set start $t; drive bRst 1; step 10; drive bRst 0
            drive bData 0x44332211; drive bKeep $keep; drive bDataEnable 1; step 10; drive bDataEnable 0
            set crc 65535
            for {set byte 0} {$byte<$count} {incr byte} {
                set data [expr {(0x44332211>>($byte*8))&255}]
                for {set bit 0} {$bit<8} {incr bit} {
                    set crc [expr {(($crc^($data>>$bit))&1) ? (($crc>>1)^0x8408) : ($crc>>1)}]
                }
            }
            check bCRC $crc; step 20
            snap csi2_crc16_valid_byte_count_$count $start {} checked
        }
        set start $t; set crc [value bCRC]; drive bData 0xDEADBEEF; step 80; check bCRC $crc
        snap csi2_crc16_disabled_holds_state $start {} checked
    }
    case_run dpti_tx {
        begin axi_dpti_1_0.AXI_S_to_DPTI_converter {DPTI AXI stream to byte output - keep masks and FTDI stalls} {PROG_CLK pResetnTx pTxe pInTdata pInTkeep pInTvalid pOutTready pWr pDataOut pTxLengthEmpty}
        clock PROG_CLK; drive pResetnTx 0
        foreach {s v} {pTxe 0 pInTdata 0 pInTvalid 0 pInTlast 0 pInTkeep 0 pAXI_L_Length 4 pOvalidLength 0 pAXI_L_Control 1 pOvalidControl 0} {drive $s $v}
        step 30; drive pResetnTx 1; drive pOvalidControl 1; step 10; drive pOvalidControl 0
        foreach keep {1 3 7 15 5 10} {
            set start $t; set count 0
            for {set bit 0} {$bit<4} {incr bit} {incr count [expr {($keep>>$bit)&1}]}
            drive pAXI_L_Length $count; drive pOvalidLength 1; step 10; drive pOvalidLength 0
            drive pInTdata 0x44332211; drive pInTkeep $keep; drive pInTvalid 1; drive pInTlast 1
            wait_high pOutTready; step 10; drive pInTvalid 0; step 120
            snap dpti_transmit_keep_mask_$keep $start
        }
        set start $t; drive pTxe 1; drive pAXI_L_Length 4; drive pOvalidLength 1; step 10; drive pOvalidLength 0
        drive pInTkeep 15; drive pInTvalid 1; step 80; check pOutTready 0
        drive pTxe 0; step 120
        snap dpti_transmit_backpressure_and_resume $start {} checked
    }
    case_run dpti_rx {
        begin axi_dpti_1_0.DPTI_to_AXI_S_converter {DPTI byte input to AXI stream - partial words, ready stalls} {PROG_CLK pResetnRx pRxf pRd pOe pDataIn pInTready pOutTdata pOutTkeep pOutTvalid pOutTlast pRxLengthEmpty}
        clock PROG_CLK; drive pResetnRx 0
        foreach {s v} {pRxf 1 pDataIn 0 pInTready 1 pAXI_L_Length 4 pOvalidLength 0 pAXI_L_Control 2 pOvalidControl 0} {drive $s $v}
        step 30; drive pResetnRx 1; drive pOvalidControl 1; step 10; drive pOvalidControl 0
        foreach count {1 2 3 4 7 8} {
            set start $t; drive pAXI_L_Length $count; drive pOvalidLength 1; step 10; drive pOvalidLength 0; drive pRxf 0
            for {set n 0} {$n<$count+3} {incr n} {drive pDataIn [expr {17+$n}]; step 10}
            drive pRxf 1; step 100
            snap dpti_receive_length_$count $start
        }
    }
}

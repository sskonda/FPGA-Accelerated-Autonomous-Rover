# Protocol checks on real AXI-Lite endpoints. Side-effect and read-only registers
# are observed; equality checks are made only for documented writable fields.
proc rover::bus_capture {name body} {
    variable t; variable failures
    set start $t; set before $failures
    if {[catch {uplevel 1 $body} msg]} {
        snap $name $start {} incomplete
        error $msg
    }
    snap $name $start {} [expr {$before==$failures ? "protocol_checked" : "check_failed"}]
}
proc rover::buses_tests {} {
    set targets {
        {pwm_v1 pwm_1_0.PWM_v1_0 pwm_axi_ pwm_axi_aclk pwm_axi_aresetn {0 4 8 12} {} {pwm interrupt} {}}
        {pwm_v2 pwm_2_0.PWM_v2_0 pwm_axi_ pwm_axi_aclk pwm_axi_aresetn {0 4 8 64 68 72 76} {} {pwm} {-gNUM_PWM=4}}
        {pwm_analyzer pwm_analyzer_1_0.PWM_Analyzer_v1_0_S00_AXI S_AXI_ S_AXI_ACLK S_AXI_ARESETN {0 4 8 12} {pwm 0} {pwm} {}}
        {motor_feedback motorfeedback_1_0.MotorFeedback_v1_0_S00_AXI S_AXI_ S_AXI_ACLK S_AXI_ARESETN {0 4 8 12 16 20 24 28} {m1_feedback 0 m2_feedback 0} {m1_feedback m2_feedback} {}}
        {csi2_registers mipi_csi_2_rx.MIPI_CSI_2_RX_S_AXI_LITE S_AXI_ S_AXI_ACLK S_AXI_ARESETN {0 4 8 12} {} {xEnable xRst} {}}
        {dphy_registers mipi_d_phy_rx.MIPI_DPHY_Receiver_S_AXI_Lite S_AXI_ S_AXI_ACLK S_AXI_ARESETN {0 4 8 12} {} {xEnable xRst} {}}
        {dynamic_clock_registers axi_dynclk.axi_dynclk_S00_AXI S_AXI_ S_AXI_ACLK S_AXI_ARESETN {0 4 8 12 16 20 24 28} {STAT_REG 0} {CTRL_REG STAT_REG CLK_O_REG} {}}
        {dpti_registers axi_dpti_1_0.axi_dpti_v1_0_AXI_LITE S_AXI_ S_AXI_ACLK S_AXI_ARESETN {0 4 8 12} {lRdyLength 1 lRdyControl 1 lAckLength 1 lAckControl 1 TxLengthEmpty 1 RxLengthEmpty 1 prog_spien 0} {lPushLength lPushControl} {}}
        {ps2_registers axi_ps2_1_0.axi_ps2_v1_0_S_AXI S_AXI_ S_AXI_ACLK S_AXI_ARESETN {0 4 8 12 16 20 24 28} {lRxDataReg 90 lStatusReg 0 IsrBitTxNoAck 0 IsrBitTxAck 0 IsrBitRxOvf 0 IsrBitRxErr 0 IsrBitRxFull 0} {lTxDataReg lTxTrig IntrOut} {}}
        {i2s_registers axi_i2s_adi_1_2.axi_i2s_adi_S_AXI S_AXI_ S_AXI_ACLK S_AXI_ARESETN {0 4 8 12 16 20 24 28 32 36 40 44} {rd_data 305419896} {rd_addr wr_addr wr_data wr_stb} {}}
        {i2s_registers_5bit_address axi_i2s_adi_1_2.axi_i2s_adi_S_AXI S_AXI_ S_AXI_ACLK S_AXI_ARESETN {0 4 8 12 16 20 24 28} {rd_data 305419896} {rd_addr wr_addr wr_data wr_stb} {-gC_S_AXI_ADDR_WIDTH=5}}
    }
    foreach target $targets {
        lassign $target name design prefix clk reset addresses extras outs opts
        case_run $name {
            set signals [list $clk $reset]
            foreach s {AWADDR AWVALID AWREADY WDATA WSTRB WVALID WREADY BVALID BREADY BRESP ARADDR ARVALID ARREADY RDATA RVALID RREADY RRESP} {
                lappend signals ${prefix}$s
            }
            lappend signals {*}$outs
            begin $design "AXI-Lite: $name - real register endpoint" $signals {*}$opts
            foreach {s v} $extras {drive $s $v}
            axi_init $prefix $clk $reset
            bus_capture axi_${name}_reset {axi_read [lindex $addresses 0]}
            foreach addr $addresses {
                set data [expr {0x12345600 + $addr}]
                bus_capture [format axi_%s_register_%02x_write_read $name $addr] {
                    axi_write $addr $data
                    set expected {}
                    if {$name in {pwm_v1 pwm_v2}} {set expected $data}
                    axi_read $addr $expected
                }
            }
            set addr [lindex $addresses 0]
            set previous {}
            if {$name in {pwm_v1 pwm_v2}} {set previous [expr {0x12345600}]}
            foreach strobe {0 1 2 4 8 5 10 15} {
                bus_capture [format axi_%s_byte_strobes_%x $name $strobe] {
                    axi_write $addr 0xA5C35A96 $strobe
                    if {$previous ne {}} {
                        set mask 0
                        for {set byte 0} {$byte<4} {incr byte} {if {($strobe>>$byte)&1} {set mask [expr {$mask|(255<<($byte*8))}]}}
                        set previous [expr {(($previous&(~$mask))|(0xA5C35A96&$mask))&0xFFFFFFFF}]
                    }
                    axi_read $addr $previous
                }
            }
            bus_capture axi_${name}_address_before_data {axi_write $addr 0x11223344 15 30; axi_read $addr}
            bus_capture axi_${name}_data_before_address {axi_write $addr 0x55667788 15 -30; axi_read $addr}
            bus_capture axi_${name}_write_response_backpressure {axi_write $addr 0xCAFEBABE 15 0 60}
            bus_capture axi_${name}_read_response_backpressure {axi_read $addr {} 60}
            bus_capture axi_${name}_reset_during_pending_request {
                drive ${prefix}AWVALID 1; drive ${prefix}WVALID 0; step 20
                drive $reset 0; step 30
                drive ${prefix}AWVALID 0; drive $reset 1; step 30
                check ${prefix}BVALID 0; check ${prefix}RVALID 0
                axi_read $addr
            }
        }
    }
    # Generated HLS register interfaces have independent AW/W channels and
    # active-high reset. Test the register modules and the datapaths separately.
    foreach {name design addresses} {
        gamma hls_gamma_correction_1_0.hls_gamma_correction_AXILiteS_s_axi {16 24 32}
        contrast hls_contrast_stretch_1_0.hls_contrast_stretch_AXILiteS_s_axi {16 24 32}
        saturation hls_saturation_enhance_1_0.hls_saturation_enhance_AXILiteS_s_axi {16 24 32}
        scaler video_scaler.video_scaler_ctrl_s_axi {4 8 16 24 32 40}
    } {
        case_run hls_${name}_registers {
            begin $design "HLS $name AXI-Lite register transactions" {ACLK ARESET ACLK_EN AWADDR AWVALID AWREADY WDATA WSTRB WVALID WREADY BVALID BREADY BRESP ARADDR ARVALID ARREADY RDATA RVALID RREADY RRESP}
            variable ax; set ax {}
            clock ACLK; drive ARESET 1; drive ACLK_EN 1
            foreach {s v} {AWADDR 0 AWVALID 0 WDATA 0 WSTRB 15 WVALID 0 BREADY 1 ARADDR 0 ARVALID 0 RREADY 1} {drive $s $v}
            if {$name eq "scaler"} {foreach {s v} {ap_done 0 ap_ready 0 ap_idle 1} {drive $s $v}}
            step 40; drive ARESET 0; step 20
            foreach addr $addresses {
                bus_capture [format axi_hls_%s_register_%02x_write_read $name $addr] {axi_write $addr 0x12345678; axi_read $addr}
            }
            set addr [lindex $addresses end]
            foreach strobe {0 1 2 4 8 5 10 15} {
                bus_capture [format axi_hls_%s_byte_strobes_%x $name $strobe] {axi_write $addr 0xA5C35A96 $strobe; axi_read $addr}
            }
            bus_capture axi_hls_${name}_address_before_data {axi_write $addr 291 15 40; axi_read $addr}
            bus_capture axi_hls_${name}_data_before_address {axi_write $addr 564 15 -40; axi_read $addr}
            bus_capture axi_hls_${name}_response_backpressure {axi_write $addr 837 15 0 60; axi_read $addr {} 60}
        }
    }
}

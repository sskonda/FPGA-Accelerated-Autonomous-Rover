# CSI-2 header ECC: every protected single bit and every unordered double-bit
# combination. Duplicate orderings are omitted; each distinct case is captured.
proc rover::camera_tests {} {
    begin mipi_csi_2_rx.ECC {CSI-2 header ECC - original Digilent RTL} {StreamClk sRst sCE sHeaderIn sReady sHeaderOut sValid sError sState}
    clock StreamClk
    drive sRst 1; drive sCE 0; drive sHeaderIn 0; step 50
    drive sRst 0; step 20
    variable t; variable failures
    set header 0x3F01F037
    set vectors [list [list valid_header $header 0 1]]
    for {set bit 0} {$bit < 30} {incr bit} {
        lappend vectors [list [format single_bit_%02d_corrected $bit] [expr {$header ^ (1<<$bit)}] 1 1]
    }
    for {set a 0} {$a < 30} {incr a} {
        for {set b [expr {$a+1}]} {$b < 30} {incr b} {
            lappend vectors [list [format double_bit_%02d_%02d_detected $a $b] [expr {$header ^ (1<<$a) ^ (1<<$b)}] 1 0]
        }
    }
    foreach v $vectors {
        lassign $v name data err valid
        set start $t
        set before $failures
        check sReady 1
        drive sHeaderIn [format 16#%08X $data]; drive sCE 1; step 10
        drive sCE 0; step 20
        check sValid $valid; check sError $err
        if {$valid} {check sHeaderOut [expr {$header}]}
        step 30
        snap csi2_ecc_$name $start {} [expr {$before==$failures ? "checked" : "check_failed"}]
    }
    # Replay original self-checking ECC TB (two complete repeated sweeps).
    begin mipi_csi_2_rx.tb_ECC {CSI-2 original ECC testbench - 1802 headers} {StreamClk sRst sCE sHeaderIn sHeaderOut sValid sError}
    step 100000
    snap csi2_ecc_original_testbench_complete 0 100000
    # Lane merging TB works without generated Xilinx FIFOs.
    case_run csi2_lane_merge {
        begin mipi_csi_2_rx.tb_LM {CSI-2 lane merge - original testbench} {RxByteClkHS rbRst rbEn RxDataHS RxValidHS RxActiveHS m_axis_tdata m_axis_tkeep m_axis_tvalid m_axis_tready m_axis_tlast}
        step 5000
        snap csi2_lane_merge_original_testbench 0 5000
    }
}

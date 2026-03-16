source "C:/Users/sanat/FPGA_Car_Project/Vitis_Vid_Pass/pcam_passthrough/_ide/psinit/ps7_init.tcl"
ps7_init
ps7_post_config


proc generate {drv_handle} {
	xdefine_include_file $drv_handle "xparameters.h" "MIPI_CSI_2_RX" "NUM_INSTANCES" "DEVICE_ID"  "C_S_AXI_LITE_BASEADDR" "C_S_AXI_LITE_HIGHADDR"
}

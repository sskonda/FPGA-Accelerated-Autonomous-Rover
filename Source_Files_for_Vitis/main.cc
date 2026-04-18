// #include "../assert_filename_fix.h"
// #include "xparameters.h"
// #include "../xparams_compat.h"

// #include "../platform/platform.h"
// #include "../ov5640/OV5640.h"
// #include "../ov5640/ScuGicInterruptController.h"
// #include "../ov5640/PS_GPIO.h"
// #include "../ov5640/AXI_VDMA.h"
// #include "../ov5640/PS_IIC.h"
// #include "../hdmi/VideoOutput.h"

// #include "../MIPI_D_PHY_RX_v1_0/src/MIPI_D_PHY_RX.h"
// #include "../MIPI_CSI_2_RX_v1_0/src/MIPI_CSI_2_RX.h"

// #define IRPT_CTL_DEVID          XPAR_PS7_SCUGIC_0_DEVICE_ID
// #define GPIO_DEVID              XPAR_PS7_GPIO_0_DEVICE_ID
// #define GPIO_IRPT_ID            XPAR_PS7_GPIO_0_INTR
// #define CAM_I2C_DEVID           XPAR_PS7_I2C_0_DEVICE_ID
// #define CAM_I2C_IRPT_ID         XPAR_PS7_I2C_0_INTR
// #define VDMA_DEVID              XPAR_AXIVDMA_0_DEVICE_ID
// #define VDMA_MM2S_IRPT_ID       XPAR_FABRIC_AXI_VDMA_0_MM2S_INTROUT_INTR
// #define VDMA_S2MM_IRPT_ID       XPAR_FABRIC_AXI_VDMA_0_S2MM_INTROUT_INTR

// #define DDR_BASE_ADDR           XPAR_DDR_MEM_BASEADDR
// #define MEM_BASE_ADDR           (DDR_BASE_ADDR + 0x0A000000)

// #define GAMMA_BASE_ADDR         XPAR_AXI_GAMMACORRECTION_0_BASEADDR

// using namespace digilent;

// static void pipeline_mode_change(
//     AXI_VDMA<ScuGicInterruptController>& vdma_driver,
//     OV5640& cam,
//     VideoOutput& vid,
//     Resolution res,
//     OV5640_cfg::mode_t mode)
// {
//     {
//         vdma_driver.resetWrite();
//         MIPI_CSI_2_RX_mWriteReg(
//             XPAR_MIPI_CSI_2_RX_0_S_AXI_LITE_BASEADDR,
//             CR_OFFSET,
//             (CR_RESET_MASK & ~CR_ENABLE_MASK));
//         MIPI_D_PHY_RX_mWriteReg(
//             XPAR_MIPI_D_PHY_RX_0_S_AXI_LITE_BASEADDR,
//             CR_OFFSET,
//             (CR_RESET_MASK & ~CR_ENABLE_MASK));
//         cam.reset();
//     }

//     {
//         vdma_driver.configureWrite(
//             timing[static_cast<int>(res)].h_active,
//             timing[static_cast<int>(res)].v_active);
//         Xil_Out32(GAMMA_BASE_ADDR, 3);
//         cam.init();
//     }

//     {
//         vdma_driver.enableWrite();
//         MIPI_CSI_2_RX_mWriteReg(
//             XPAR_MIPI_CSI_2_RX_0_S_AXI_LITE_BASEADDR,
//             CR_OFFSET,
//             CR_ENABLE_MASK);
//         MIPI_D_PHY_RX_mWriteReg(
//             XPAR_MIPI_D_PHY_RX_0_S_AXI_LITE_BASEADDR,
//             CR_OFFSET,
//             CR_ENABLE_MASK);
//         cam.set_mode(mode);
//         cam.set_awb(OV5640_cfg::awb_t::AWB_ADVANCED);
//     }

//     {
//         vid.reset();
//         vdma_driver.resetRead();
//     }

//     {
//         vid.configure(res);
//         vdma_driver.configureRead(
//             timing[static_cast<int>(res)].h_active,
//             timing[static_cast<int>(res)].v_active);
//     }

//     {
//         vid.enable();
//         vdma_driver.enableRead();
//     }
// }

// int main()
// {
//     init_platform();

//     xil_printf("Starting video passthrough init...\r\n");

//     try {
//         ScuGicInterruptController irpt_ctl(IRPT_CTL_DEVID);
//         PS_GPIO<ScuGicInterruptController> gpio_driver(GPIO_DEVID, irpt_ctl, GPIO_IRPT_ID);
//         PS_IIC<ScuGicInterruptController> iic_driver(CAM_I2C_DEVID, irpt_ctl, CAM_I2C_IRPT_ID, 100000);

//         OV5640 cam(iic_driver, gpio_driver);
//         AXI_VDMA<ScuGicInterruptController> vdma_driver(
//             VDMA_DEVID,
//             MEM_BASE_ADDR,
//             irpt_ctl,
//             VDMA_MM2S_IRPT_ID,
//             VDMA_S2MM_IRPT_ID);

//         VideoOutput vid(XPAR_VTC_0_DEVICE_ID, XPAR_VIDEO_DYNCLK_DEVICE_ID);

//         pipeline_mode_change(
//             vdma_driver,
//             cam,
//             vid,
//             Resolution::R1920_1080_60_PP,
//             OV5640_cfg::mode_t::MODE_1080P_1920_1080_30fps);

//         xil_printf("Video init done.\r\n");
//     }
//     catch (...) {
//         xil_printf("Video init failed.\r\n");
//         while (1) {
//         }
//     }

//     while (1) {
//     }

//     cleanup_platform();
//     return 0;
// }
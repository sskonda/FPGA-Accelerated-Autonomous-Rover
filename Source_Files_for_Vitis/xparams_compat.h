#pragma once
#include "xparameters.h"

/*
  v1_xparameters.h (SDT-style) doesn’t define many of the classic macros
  your Digilent example code expects. This header maps them.
*/

/* -------------------- Device IDs -------------------- */
/* SDT-style headers often omit *_DEVICE_ID macros; if you only have one instance, ID=0 is correct. */
#ifndef XPAR_PS7_SCUGIC_0_DEVICE_ID
#define XPAR_PS7_SCUGIC_0_DEVICE_ID 0
#endif

#ifndef XPAR_PS7_GPIO_0_DEVICE_ID
#define XPAR_PS7_GPIO_0_DEVICE_ID 0
#endif

#ifndef XPAR_PS7_I2C_0_DEVICE_ID
#define XPAR_PS7_I2C_0_DEVICE_ID 0
#endif

#ifndef XPAR_AXIVDMA_0_DEVICE_ID
#define XPAR_AXIVDMA_0_DEVICE_ID 0
#endif

#ifndef XPAR_VTC_0_DEVICE_ID
#define XPAR_VTC_0_DEVICE_ID 0
#endif

#ifndef XPAR_VIDEO_DYNCLK_DEVICE_ID
#define XPAR_VIDEO_DYNCLK_DEVICE_ID 0
#endif

/* -------------------- MIPI base address naming -------------------- */
#ifndef XPAR_MIPI_CSI_2_RX_0_S_AXI_LITE_BASEADDR
#define XPAR_MIPI_CSI_2_RX_0_S_AXI_LITE_BASEADDR XPAR_MIPI_CSI_2_RX_0_BASEADDR
#endif

#ifndef XPAR_MIPI_D_PHY_RX_0_S_AXI_LITE_BASEADDR
#define XPAR_MIPI_D_PHY_RX_0_S_AXI_LITE_BASEADDR XPAR_MIPI_D_PHY_RX_0_BASEADDR
#endif

/* -------------------- VDMA interrupt naming -------------------- */
/* v1 uses INTR / INTR_1 instead of MM2S_INTROUT_INTR / S2MM_INTROUT_INTR */
#ifndef XPAR_FABRIC_AXI_VDMA_0_MM2S_INTROUT_INTR
#define XPAR_FABRIC_AXI_VDMA_0_MM2S_INTROUT_INTR XPAR_FABRIC_AXI_VDMA_0_INTR
#endif

#ifndef XPAR_FABRIC_AXI_VDMA_0_S2MM_INTROUT_INTR
#define XPAR_FABRIC_AXI_VDMA_0_S2MM_INTROUT_INTR XPAR_FABRIC_AXI_VDMA_0_INTR_1
#endif

/* -------------------- PS interrupt IDs -------------------- */
/*
  SDT-style *_INTERRUPTS values are encoded like 0x4000 | (SPI_ID - 32).
  Convert them into the numeric GIC interrupt ID your code expects.
*/
#ifndef XPAR_PS7_GPIO_0_INTR
#define XPAR_PS7_GPIO_0_INTR  ((XPAR_XGPIOPS_0_INTERRUPTS & 0xFFF) + 32)
#endif

#ifndef XPAR_PS7_I2C_0_INTR
#define XPAR_PS7_I2C_0_INTR   ((XPAR_XIICPS_0_INTERRUPTS & 0xFFF) + 32)
#endif

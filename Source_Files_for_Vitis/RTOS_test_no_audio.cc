// #include <stdint.h>
// #include <stdbool.h>

// extern "C" {
// #include "../assert_filename_fix.h"
// #include "xparameters.h"
// #include "xstatus.h"
// #include "xil_printf.h"
// #include "xil_types.h"
// #include "xil_io.h"
// #include "sleep.h"
// #include "xiicps.h"
// #include "xuartlite.h"
// #include "FreeRTOS.h"
// #include "task.h"
// #include "queue.h"
// }

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

// using namespace digilent;

// namespace {

// /* ============================================================
//  * Camera / video pipeline defines
//  * ============================================================
//  */
// #define IRPT_CTL_DEVID          XPAR_PS7_SCUGIC_0_DEVICE_ID
// #define GPIO_DEVID              XPAR_PS7_GPIO_0_DEVICE_ID
// #define GPIO_IRPT_ID            XPAR_PS7_GPIO_0_INTR
// #define CAM_I2C_DEVID           XPAR_PS7_I2C_0_DEVICE_ID
// #define CAM_I2C_IRPT_ID         XPAR_PS7_I2C_0_INTR
// #define VDMA_DEVID              XPAR_AXIVDMA_0_DEVICE_ID
// #define VDMA_MM2S_IRPT_ID       XPAR_FABRIC_AXI_VDMA_0_MM2S_INTROUT_INTR
// #define VDMA_S2MM_IRPT_ID       XPAR_FABRIC_AXI_VDMA_0_S2MM_INTROUT_INTR
// #define DDR_BASE_ADDR           XPAR_DDR_MEM_BASEADDR
// #define MEM_BASE_ADDR           (DDR_BASE_ADDR + 0x0A000000U)
// #define GAMMA_BASE_ADDR         XPAR_AXI_GAMMACORRECTION_0_BASEADDR

// /* ============================================================
//  * Motor control defines
//  * ============================================================
//  */
// #ifndef XPAR_MOTOR_CTRL_4CH_0_BASEADDR
// #error "XPAR_MOTOR_CTRL_4CH_0_BASEADDR is not defined"
// #endif

// #define MOTOR_BASE              XPAR_MOTOR_CTRL_4CH_0_BASEADDR
// #define REG_CTRL                0x00U
// #define REG_PERIOD              0x04U
// #define REG_DUTY0               0x08U
// #define M0_EN                   (1u << 0)
// #define M0_DIR                  (1u << 4)
// #define M0_BRAKE                (1u << 8)

// /* ============================================================
//  * PS I2C1 shared sensor bus
//  * ============================================================
//  */
// #ifndef XPAR_XIICPS_1_BASEADDR
// #error "XPAR_XIICPS_1_BASEADDR is not defined"
// #endif

// #define SENSOR_I2C_SCLK_HZ      50000U
// #define BNO055_I2C_ADDR         0x28U

// #define BNO055_CHIP_ID_REG      0x00U
// #define BNO055_PAGE_ID_REG      0x07U
// #define BNO055_ACC_DATA_X_LSB   0x08U
// #define BNO055_GYR_DATA_X_LSB   0x14U
// #define BNO055_UNIT_SEL_REG     0x3BU
// #define BNO055_OPR_MODE_REG     0x3DU
// #define BNO055_PWR_MODE_REG     0x3EU
// #define BNO055_CHIP_ID_VAL      0xA0U
// #define BNO055_PWR_NORMAL       0x00U
// #define BNO055_OPR_CONFIG       0x00U
// #define BNO055_OPR_AMG          0x07U

// /* ============================================================
//  * Sonar UART
//  * ============================================================
//  */
// #ifndef XPAR_XUARTLITE_0_BASEADDR
// #error "XPAR_XUARTLITE_0_BASEADDR is not defined"
// #endif

// #define SONAR_UART_BASEADDR     XPAR_XUARTLITE_0_BASEADDR

// /* ============================================================
//  * Behavior knobs
//  * ============================================================
//  */
// #define MOTOR_PWM_PERIOD        5000U
// #define MOTOR_DUTY_LOW          500U
// #define MOTOR_DUTY_HIGH         1000U
// #define SONAR_STOP_INCHES       18
// #define SONAR_ALERT_INCHES      6
// #define SENSOR_TASK_PERIOD_MS   100U
// #define MOTOR_TASK_PERIOD_MS    50U
// #define TELEMETRY_PERIOD_MS     500U

// #define STACK_LOGGER_TASK       2048U
// #define STACK_SENSOR_TASK       2048U
// #define STACK_SONAR_TASK        1024U
// #define STACK_MOTOR_TASK        1024U
// #define STACK_TELEM_TASK        2048U

// #define PRIO_SENSOR_TASK        (tskIDLE_PRIORITY + 3)
// #define PRIO_MOTOR_TASK         (tskIDLE_PRIORITY + 2)
// #define PRIO_LOGGER_TASK        (tskIDLE_PRIORITY + 2)
// #define PRIO_SONAR_TASK         (tskIDLE_PRIORITY + 1)
// #define PRIO_TELEM_TASK         (tskIDLE_PRIORITY + 1)

// #define LOG_QUEUE_LENGTH        16U

// struct SharedState {
//     bool video_ok;
//     bool bno_ok;
//     bool sonar_ok;

//     int16_t ax;
//     int16_t ay;
//     int16_t az;
//     int16_t gx;
//     int16_t gy;
//     int16_t gz;

//     int sonar_inches;
// };

// enum LogType : uint8_t {
//     LOG_SENSOR_INIT_START = 0,
//     LOG_SENSOR_BNO_OK,
//     LOG_SONAR_INIT_START,
//     LOG_SONAR_UART_OK,
//     LOG_MOTOR_INIT_START,
//     LOG_MOTOR_ACTIVE,
//     LOG_OBSTACLE_DETECTED,
//     LOG_TELEMETRY
// };

// struct LogEvent {
//     LogType type;
//     SharedState snap;
// };

// static SharedState g_state{};
// static XIicPs g_iic{};
// static XUartLite g_uart{};
// static QueueHandle_t g_log_queue = nullptr;

// /* Forward declarations */
// static void vLoggerTask(void *);
// static void vSensorTask(void *);
// static void vSonarTask(void *);
// static void vMotorTask(void *);
// static void vTelemetryTask(void *);

// static inline void motor_wr(unsigned offset, unsigned value)
// {
//     Xil_Out32(MOTOR_BASE + offset, value);
// }

// static void motor0_stop(void)
// {
//     motor_wr(REG_DUTY0, 0U);
//     motor_wr(REG_CTRL, 0U);
// }

// static void motor0_brake(void)
// {
//     motor_wr(REG_DUTY0, 0U);
//     motor_wr(REG_CTRL, M0_EN | M0_BRAKE);
// }

// static void motor0_forward(unsigned duty)
// {
//     motor_wr(REG_DUTY0, duty);
//     motor_wr(REG_CTRL, M0_EN);
// }

// static int16_t le16_to_s16(uint8_t lsb, uint8_t msb)
// {
//     return static_cast<int16_t>((static_cast<uint16_t>(msb) << 8) | static_cast<uint16_t>(lsb));
// }

// static void update_bno(int16_t ax, int16_t ay, int16_t az, int16_t gx, int16_t gy, int16_t gz, bool ok)
// {
//     taskENTER_CRITICAL();
//     g_state.bno_ok = ok;
//     g_state.ax = ax;
//     g_state.ay = ay;
//     g_state.az = az;
//     g_state.gx = gx;
//     g_state.gy = gy;
//     g_state.gz = gz;
//     taskEXIT_CRITICAL();
// }

// static void update_sonar(int sonar_inches, bool ok)
// {
//     taskENTER_CRITICAL();
//     g_state.sonar_ok = ok;
//     g_state.sonar_inches = sonar_inches;
//     taskEXIT_CRITICAL();
// }

// static void update_video(bool ok)
// {
//     taskENTER_CRITICAL();
//     g_state.video_ok = ok;
//     taskEXIT_CRITICAL();
// }

// static SharedState snapshot_state(void)
// {
//     SharedState snap;

//     taskENTER_CRITICAL();
//     snap = g_state;
//     taskEXIT_CRITICAL();

//     return snap;
// }

// static void log_send_type(LogType type)
// {
//     if (g_log_queue == nullptr)
//         return;

//     LogEvent ev{};
//     ev.type = type;
//     (void)xQueueSend(g_log_queue, &ev, 0);
// }

// static void log_send_telem(const SharedState &snap)
// {
//     if (g_log_queue == nullptr)
//         return;

//     LogEvent ev{};
//     ev.type = LOG_TELEMETRY;
//     ev.snap = snap;
//     (void)xQueueSend(g_log_queue, &ev, 0);
// }

// static int wait_bus_idle(XIicPs *iic)
// {
//     int timeout = 1000000;

//     while (XIicPs_BusIsBusy(iic) && timeout > 0)
//         timeout--;

//     return (timeout > 0) ? XST_SUCCESS : XST_FAILURE;
// }

// static int sensor_i2c_init(void)
// {
//     XIicPs_Config *cfg;
//     int status;

// #ifdef SDT
//     cfg = XIicPs_LookupConfig(XPAR_XIICPS_1_BASEADDR);
// #else
//     cfg = XIicPs_LookupConfig(XPAR_XIICPS_1_DEVICE_ID);
// #endif
//     if (cfg == nullptr)
//         return XST_FAILURE;

//     status = XIicPs_CfgInitialize(&g_iic, cfg, cfg->BaseAddress);
//     if (status != XST_SUCCESS)
//         return status;

//     status = XIicPs_SelfTest(&g_iic);
//     if (status != XST_SUCCESS)
//         return status;

//     status = XIicPs_SetSClk(&g_iic, SENSOR_I2C_SCLK_HZ);
//     if (status != XST_SUCCESS)
//         return status;

//     status = XIicPs_SetOptions(&g_iic, XIICPS_7_BIT_ADDR_OPTION);
//     if (status != XST_SUCCESS)
//         return status;

//     return XST_SUCCESS;
// }

// static int i2c_write_reg8(uint16_t addr, uint8_t reg, uint8_t value)
// {
//     uint8_t tx[2];
//     int status;

//     tx[0] = reg;
//     tx[1] = value;

//     status = XIicPs_MasterSendPolled(&g_iic, tx, 2, addr);
//     if (status != XST_SUCCESS)
//         return status;

//     return wait_bus_idle(&g_iic);
// }

// static int i2c_read_bytes(uint16_t addr, uint8_t reg, uint8_t *rx, int len)
// {
//     int status;

//     status = XIicPs_SetOptions(&g_iic, XIICPS_7_BIT_ADDR_OPTION | XIICPS_REP_START_OPTION);
//     if (status != XST_SUCCESS)
//         return status;

//     status = XIicPs_MasterSendPolled(&g_iic, &reg, 1, addr);
//     if (status != XST_SUCCESS) {
//         XIicPs_ClearOptions(&g_iic, XIICPS_REP_START_OPTION);
//         return status;
//     }

//     status = XIicPs_MasterRecvPolled(&g_iic, rx, len, addr);
//     XIicPs_ClearOptions(&g_iic, XIICPS_REP_START_OPTION);

//     if (status != XST_SUCCESS)
//         return status;

//     return wait_bus_idle(&g_iic);
// }

// static int bno055_init(void)
// {
//     uint8_t chip_id = 0;
//     int status;

//     vTaskDelay(pdMS_TO_TICKS(800));

//     for (int tries = 0; tries < 10; tries++) {
//         status = i2c_read_bytes(BNO055_I2C_ADDR, BNO055_CHIP_ID_REG, &chip_id, 1);
//         if ((status == XST_SUCCESS) && (chip_id == BNO055_CHIP_ID_VAL))
//             break;
//         vTaskDelay(pdMS_TO_TICKS(50));
//     }

//     if (chip_id != BNO055_CHIP_ID_VAL)
//         return XST_FAILURE;

//     status = i2c_write_reg8(BNO055_I2C_ADDR, BNO055_PAGE_ID_REG, 0x00U);
//     if (status != XST_SUCCESS)
//         return status;

//     status = i2c_write_reg8(BNO055_I2C_ADDR, BNO055_OPR_MODE_REG, BNO055_OPR_CONFIG);
//     if (status != XST_SUCCESS)
//         return status;

//     vTaskDelay(pdMS_TO_TICKS(25));

//     status = i2c_write_reg8(BNO055_I2C_ADDR, BNO055_PWR_MODE_REG, BNO055_PWR_NORMAL);
//     if (status != XST_SUCCESS)
//         return status;

//     vTaskDelay(pdMS_TO_TICKS(10));

//     status = i2c_write_reg8(BNO055_I2C_ADDR, BNO055_UNIT_SEL_REG, 0x00U);
//     if (status != XST_SUCCESS)
//         return status;

//     vTaskDelay(pdMS_TO_TICKS(10));

//     status = i2c_write_reg8(BNO055_I2C_ADDR, BNO055_OPR_MODE_REG, BNO055_OPR_AMG);
//     if (status != XST_SUCCESS)
//         return status;

//     vTaskDelay(pdMS_TO_TICKS(20));
//     return XST_SUCCESS;
// }

// static int sonar_uart_init(void)
// {
//     int status;

// #ifdef SDT
//     status = XUartLite_Initialize(&g_uart, SONAR_UART_BASEADDR);
// #else
//     status = XUartLite_Initialize(&g_uart, XPAR_UARTLITE_0_DEVICE_ID);
// #endif
//     if (status != XST_SUCCESS)
//         return status;

//     status = XUartLite_SelfTest(&g_uart);
//     if (status != XST_SUCCESS)
//         return status;

//     return XST_SUCCESS;
// }

// static int uartlite_get_byte(uint8_t *byte_out)
// {
//     const unsigned rx_count = XUartLite_Recv(&g_uart, byte_out, 1);
//     return (rx_count == 1U) ? 1 : 0;
// }

// static int is_digit_u8(uint8_t c)
// {
//     return (c >= '0' && c <= '9');
// }

// static void pipeline_mode_change(
//     AXI_VDMA<ScuGicInterruptController> &vdma_driver,
//     OV5640 &cam,
//     VideoOutput &vid,
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
//         Xil_Out32(GAMMA_BASE_ADDR, 3U);
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

// static int video_init_once(void)
// {
//     try {
//         static ScuGicInterruptController irpt_ctl(IRPT_CTL_DEVID);
//         static PS_GPIO<ScuGicInterruptController> gpio_driver(GPIO_DEVID, irpt_ctl, GPIO_IRPT_ID);
//         static PS_IIC<ScuGicInterruptController> iic_driver(CAM_I2C_DEVID, irpt_ctl, CAM_I2C_IRPT_ID, 100000);
//         static OV5640 cam(iic_driver, gpio_driver);
//         static AXI_VDMA<ScuGicInterruptController> vdma_driver(
//             VDMA_DEVID,
//             MEM_BASE_ADDR,
//             irpt_ctl,
//             VDMA_MM2S_IRPT_ID,
//             VDMA_S2MM_IRPT_ID);
//         static VideoOutput vid(XPAR_VTC_0_DEVICE_ID, XPAR_VIDEO_DYNCLK_DEVICE_ID);

//         pipeline_mode_change(
//             vdma_driver,
//             cam,
//             vid,
//             Resolution::R1920_1080_60_PP,
//             OV5640_cfg::mode_t::MODE_1080P_1920_1080_30fps);

//         return XST_SUCCESS;
//     }
//     catch (...) {
//         return XST_FAILURE;
//     }
// }

// static void print_fixed_2(const char *name, int32_t whole, int32_t frac, int negative)
// {
//     xil_printf("%s=%s%d.%02d", name, negative ? "-" : "", static_cast<int>(whole), static_cast<int>(frac));
// }

// static void print_accel_mps2(const char *name, int16_t raw)
// {
//     int32_t mag = raw;
//     int negative = 0;

//     if (mag < 0) {
//         negative = 1;
//         mag = -mag;
//     }

//     print_fixed_2(name, mag / 100, mag % 100, negative);
// }

// static void print_gyro_dps(const char *name, int16_t raw)
// {
//     int32_t mag = raw;
//     int negative = 0;
//     int32_t whole;
//     int32_t frac;

//     if (mag < 0) {
//         negative = 1;
//         mag = -mag;
//     }

//     whole = mag / 16;
//     frac = ((mag % 16) * 100) / 16;
//     print_fixed_2(name, whole, frac, negative);
// }

// static void print_telem(const SharedState &snap)
// {
//     xil_printf("\r\n[telemetry] video=%d bno=%d sonar=%d ",
//                snap.video_ok ? 1 : 0,
//                snap.bno_ok ? 1 : 0,
//                snap.sonar_ok ? 1 : 0);

//     if (snap.bno_ok) {
//         print_accel_mps2("AX", snap.ax);
//         xil_printf(" ");
//         print_accel_mps2("AY", snap.ay);
//         xil_printf(" ");
//         print_accel_mps2("AZ", snap.az);
//         xil_printf(" m/s^2 | ");
//         print_gyro_dps("GX", snap.gx);
//         xil_printf(" ");
//         print_gyro_dps("GY", snap.gy);
//         xil_printf(" ");
//         print_gyro_dps("GZ", snap.gz);
//         xil_printf(" dps | ");
//     } else {
//         xil_printf("BNO055 unavailable | ");
//     }

//     if (snap.sonar_ok)
//         xil_printf("Range=%d in", snap.sonar_inches);
//     else
//         xil_printf("Sonar unavailable");

//     xil_printf("\r\n");
// }

// static void vLoggerTask(void *)
// {
//     LogEvent ev{};

//     xil_printf("\r\n========================================\r\n");
//     xil_printf("FreeRTOS concurrent integration test\r\n");
//     xil_printf("Video init runs before the scheduler. Runtime prints come only from logger.\r\n");
//     xil_printf("Motor base     = 0x%08x\r\n", static_cast<unsigned>(MOTOR_BASE));
//     xil_printf("I2C1 base      = 0x%08x\r\n", static_cast<unsigned>(XPAR_XIICPS_1_BASEADDR));
//     xil_printf("UARTLite base  = 0x%08x\r\n", static_cast<unsigned>(SONAR_UART_BASEADDR));
//     xil_printf("DDR video base = 0x%08x\r\n", static_cast<unsigned>(MEM_BASE_ADDR));
//     if (g_state.video_ok)
//         xil_printf("[video] passthrough init ok\r\n");
//     else
//         xil_printf("[video] passthrough init failed\r\n");
//     xil_printf("[rtos] starting scheduler\r\n");

//     for (;;) {
//         if (xQueueReceive(g_log_queue, &ev, portMAX_DELAY) != pdPASS)
//             continue;

//         switch (ev.type) {
//         case LOG_SENSOR_INIT_START:
//             xil_printf("[sensor] init start\r\n");
//             break;

//         case LOG_SENSOR_BNO_OK:
//             xil_printf("[sensor] BNO055 init ok\r\n");
//             break;

//         case LOG_SONAR_INIT_START:
//             xil_printf("[sonar] init start\r\n");
//             break;

//         case LOG_SONAR_UART_OK:
//             xil_printf("[sonar] UARTLite init ok\r\n");
//             break;

//         case LOG_MOTOR_INIT_START:
//             xil_printf("[motor] init start\r\n");
//             break;

//         case LOG_MOTOR_ACTIVE:
//             xil_printf("[motor] channel 0 test control active\r\n");
//             break;

//         case LOG_OBSTACLE_DETECTED:
//             xil_printf("Obstacle Detected!\r\n");
//             break;

//         case LOG_TELEMETRY:
//             print_telem(ev.snap);
//             break;

//         default:
//             break;
//         }
//     }
// }

// static void vSensorTask(void *)
// {
//     TickType_t last_wake = xTaskGetTickCount();
//     const TickType_t period = pdMS_TO_TICKS(SENSOR_TASK_PERIOD_MS);
//     int status;

//     log_send_type(LOG_SENSOR_INIT_START);

//     for (;;) {
//         status = sensor_i2c_init();
//         if (status == XST_SUCCESS)
//             break;
//         vTaskDelay(pdMS_TO_TICKS(1000));
//     }

//     for (;;) {
//         status = bno055_init();
//         if (status == XST_SUCCESS)
//             break;
//         update_bno(0, 0, 0, 0, 0, 0, false);
//         vTaskDelay(pdMS_TO_TICKS(1000));
//     }

//     log_send_type(LOG_SENSOR_BNO_OK);

//     for (;;) {
//         uint8_t acc_buf[6];
//         uint8_t gyr_buf[6];

//         status = i2c_read_bytes(BNO055_I2C_ADDR, BNO055_ACC_DATA_X_LSB, acc_buf, 6);
//         if (status == XST_SUCCESS) {
//             status = i2c_read_bytes(BNO055_I2C_ADDR, BNO055_GYR_DATA_X_LSB, gyr_buf, 6);
//             if (status == XST_SUCCESS) {
//                 update_bno(
//                     le16_to_s16(acc_buf[0], acc_buf[1]),
//                     le16_to_s16(acc_buf[2], acc_buf[3]),
//                     le16_to_s16(acc_buf[4], acc_buf[5]),
//                     le16_to_s16(gyr_buf[0], gyr_buf[1]),
//                     le16_to_s16(gyr_buf[2], gyr_buf[3]),
//                     le16_to_s16(gyr_buf[4], gyr_buf[5]),
//                     true);
//             } else {
//                 update_bno(0, 0, 0, 0, 0, 0, false);
//             }
//         } else {
//             update_bno(0, 0, 0, 0, 0, 0, false);
//         }

//         vTaskDelayUntil(&last_wake, period);
//     }
// }

// static void vSonarTask(void *)
// {
//     int status;
//     uint8_t c = 0;
//     uint8_t d1 = 0;
//     uint8_t d2 = 0;
//     uint8_t d3 = 0;
//     int state = 0;
//     bool obstacle_latched = false;

//     log_send_type(LOG_SONAR_INIT_START);

//     for (;;) {
//         status = sonar_uart_init();
//         if (status == XST_SUCCESS)
//             break;
//         vTaskDelay(pdMS_TO_TICKS(1000));
//     }

//     log_send_type(LOG_SONAR_UART_OK);

//     for (;;) {
//         if (!uartlite_get_byte(&c)) {
//             vTaskDelay(pdMS_TO_TICKS(1));
//             continue;
//         }

//         switch (state) {
//         case 0:
//             if (c == 'R')
//                 state = 1;
//             break;

//         case 1:
//             if (is_digit_u8(c)) {
//                 d1 = c;
//                 state = 2;
//             } else if (c == 'R') {
//                 state = 1;
//             } else {
//                 state = 0;
//             }
//             break;

//         case 2:
//             if (is_digit_u8(c)) {
//                 d2 = c;
//                 state = 3;
//             } else if (c == 'R') {
//                 state = 1;
//             } else {
//                 state = 0;
//             }
//             break;

//         case 3:
//             if (is_digit_u8(c)) {
//                 d3 = c;
//                 state = 4;
//             } else if (c == 'R') {
//                 state = 1;
//             } else {
//                 state = 0;
//             }
//             break;

//         case 4:
//             if (c == '\r') {
//                 const int inches = (d1 - '0') * 100 + (d2 - '0') * 10 + (d3 - '0');
//                 update_sonar(inches, true);

//                 if ((inches > 0) && (inches <= SONAR_ALERT_INCHES)) {
//                     if (!obstacle_latched) {
//                         obstacle_latched = true;
//                         log_send_type(LOG_OBSTACLE_DETECTED);
//                     }
//                 } else {
//                     obstacle_latched = false;
//                 }
//             }
//             state = 0;
//             break;

//         default:
//             state = 0;
//             break;
//         }

//         vTaskDelay(pdMS_TO_TICKS(1));
//     }
// }

// static void vMotorTask(void *)
// {
//     TickType_t last_wake = xTaskGetTickCount();
//     const TickType_t period = pdMS_TO_TICKS(MOTOR_TASK_PERIOD_MS);
//     uint32_t phase_ticks = 0U;

//     log_send_type(LOG_MOTOR_INIT_START);

//     motor_wr(REG_PERIOD, MOTOR_PWM_PERIOD);
//     motor0_stop();
//     log_send_type(LOG_MOTOR_ACTIVE);

//     for (;;) {
//         const SharedState snap = snapshot_state();

//         if (snap.sonar_ok && (snap.sonar_inches > 0) && (snap.sonar_inches <= SONAR_STOP_INCHES)) {
//             motor0_brake();
//         } else {
//             const unsigned duty = ((phase_ticks / pdMS_TO_TICKS(1000)) & 0x1U) ? MOTOR_DUTY_HIGH : MOTOR_DUTY_LOW;
//             motor0_forward(duty);
//         }

//         phase_ticks += period;
//         vTaskDelayUntil(&last_wake, period);
//     }
// }

// static void vTelemetryTask(void *)
// {
//     TickType_t last_wake = xTaskGetTickCount();
//     const TickType_t period = pdMS_TO_TICKS(TELEMETRY_PERIOD_MS);

//     for (;;) {
//         log_send_telem(snapshot_state());
//         vTaskDelayUntil(&last_wake, period);
//     }
// }

// } // namespace

// extern "C" void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
// {
//     (void)xTask;
//     taskDISABLE_INTERRUPTS();
//     xil_printf("\r\n[FATAL] stack overflow in task: %s\r\n",
//                (pcTaskName != nullptr) ? pcTaskName : "unknown");
//     for (;;) {
//     }
// }

// extern "C" void vApplicationMallocFailedHook(void)
// {
//     taskDISABLE_INTERRUPTS();
//     xil_printf("\r\n[FATAL] pvPortMalloc failed\r\n");
//     for (;;) {
//     }
// }

// int main()
// {
//     BaseType_t ok;
//     int status;

//     init_platform();

//     status = video_init_once();
//     update_video(status == XST_SUCCESS);

// if (status == XST_SUCCESS) {
//     /* --- Source-side: kill VDMA frame IRQs. --- */
//     const uintptr_t vdma = XPAR_XAXIVDMA_0_BASEADDR;
//     uint32_t cr;
//     cr = Xil_In32(vdma + 0x00);
//     Xil_Out32(vdma + 0x00, cr & ~((1u << 12) | (1u << 13) | (1u << 14)));
//     cr = Xil_In32(vdma + 0x30);
//     Xil_Out32(vdma + 0x30, cr & ~((1u << 12) | (1u << 13) | (1u << 14)));
//     Xil_Out32(vdma + 0x04, 0x00007000);
//     Xil_Out32(vdma + 0x34, 0x00007000);

//     /* --- Source-side: kill VTC (Video Timing Controller) IRQs. --- */
//     /* VTC base = 0x43C10000 per the XSA. IER=0x0C, ISR=0x04 (W1C). */
//     const uintptr_t vtc = 0x43C10000u;
//     Xil_Out32(vtc + 0x0C, 0u);          /* IER = 0 (disable all VTC IRQs) */
//     Xil_Out32(vtc + 0x04, 0xFFFFFFFFu); /* ISR: clear any pending */

//     /* --- GIC-side: disable the specific SPI lines Digilent wired up. --- */
//     #define GIC_ICDICER(id) (0xF8F01180u + 4u * ((id) / 32u))
//     #define GIC_DISABLE_IRQ(id) \
//         Xil_Out32(GIC_ICDICER(id), 1u << ((id) % 32u))

//     /* IRQ_F2P[0]=vtg_irq=61, [1]=VDMA MM2S=62, [2]=VDMA S2MM=63 on Zynq-7000 */
//     GIC_DISABLE_IRQ(61);  /* vtg_irq */
//     GIC_DISABLE_IRQ(XPAR_FABRIC_AXI_VDMA_0_MM2S_INTROUT_INTR);
//     GIC_DISABLE_IRQ(XPAR_FABRIC_AXI_VDMA_0_S2MM_INTROUT_INTR);
//     GIC_DISABLE_IRQ(XPAR_PS7_GPIO_0_INTR);
//     GIC_DISABLE_IRQ(XPAR_PS7_I2C_0_INTR);

//     xil_printf("[video] VTG + VDMA + PS I2C0 + PS GPIO IRQs disabled\r\n");
// }

//     g_log_queue = xQueueCreate(LOG_QUEUE_LENGTH, sizeof(LogEvent));
//     if (g_log_queue == nullptr) {
//         xil_printf("\r\n[FATAL] log queue create failed\r\n");
//         for (;;) {
//         }
//     }

//     ok = xTaskCreate(vLoggerTask, "logger", STACK_LOGGER_TASK, nullptr, PRIO_LOGGER_TASK, nullptr);
//     if (ok != pdPASS) {
//         xil_printf("\r\n[FATAL] logger task create failed\r\n");
//         for (;;) {
//         }
//     }

//     ok = xTaskCreate(vSensorTask, "sensor", STACK_SENSOR_TASK, nullptr, PRIO_SENSOR_TASK, nullptr);
//     if (ok != pdPASS) {
//         xil_printf("\r\n[FATAL] sensor task create failed\r\n");
//         for (;;) {
//         }
//     }

//     ok = xTaskCreate(vSonarTask, "sonar", STACK_SONAR_TASK, nullptr, PRIO_SONAR_TASK, nullptr);
//     if (ok != pdPASS) {
//         xil_printf("\r\n[FATAL] sonar task create failed\r\n");
//         for (;;) {
//         }
//     }

//     ok = xTaskCreate(vMotorTask, "motor", STACK_MOTOR_TASK, nullptr, PRIO_MOTOR_TASK, nullptr);
//     if (ok != pdPASS) {
//         xil_printf("\r\n[FATAL] motor task create failed\r\n");
//         for (;;) {
//         }
//     }

//     ok = xTaskCreate(vTelemetryTask, "telem", STACK_TELEM_TASK, nullptr, PRIO_TELEM_TASK, nullptr);
//     if (ok != pdPASS) {
//         xil_printf("\r\n[FATAL] telemetry task create failed\r\n");
//         for (;;) {
//         }
//     }

//     vTaskStartScheduler();

//     xil_printf("\r\n[FATAL] scheduler returned\r\n");
//     for (;;) {
//     }
// }


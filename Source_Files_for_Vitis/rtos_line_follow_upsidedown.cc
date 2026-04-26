// #include <stdint.h>
// #include <stdbool.h>

// extern "C" {
// #include "../assert_filename_fix.h"
// #include "xparameters.h"
// #include "xstatus.h"
// #include "xil_printf.h"
// #include "xil_types.h"
// #include "xil_io.h"
// #include "xil_cache.h"
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
//  * ============================================================ */
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

// #define FRAME_W                 1920
// #define FRAME_H                 1080
// #define BYTES_PER_PIXEL         3
// #define FRAME_STRIDE            (FRAME_W * BYTES_PER_PIXEL)
// #define FRAME_SIZE_BYTES        (FRAME_STRIDE * FRAME_H)
// #define FRAME0_ADDR             (MEM_BASE_ADDR + 0U * FRAME_SIZE_BYTES)
// #define FRAME1_ADDR             (MEM_BASE_ADDR + 1U * FRAME_SIZE_BYTES)
// #define FRAME2_ADDR             (MEM_BASE_ADDR + 2U * FRAME_SIZE_BYTES)

// /* If your pipeline stores pixels as BGR instead of RGB, flip this to 1. */
// #define PIXEL_ORDER_BGR         0

// /* ============================================================
//  * Motor control IP (same reg map as square-path example)
//  *   enable bits: [3:0], direction bits: [7:4], brake bits: [11:8]
//  *   Channels 0,1 = LEFT pair; channels 2,3 = RIGHT pair.
//  * ============================================================ */
// #ifndef XPAR_MOTOR_CTRL_4CH_0_BASEADDR
// #error "XPAR_MOTOR_CTRL_4CH_0_BASEADDR is not defined"
// #endif
// #define MOTOR_BASE              XPAR_MOTOR_CTRL_4CH_0_BASEADDR
// #define REG_CTRL                0x00U
// #define REG_PERIOD              0x04U
// #define REG_DUTY0               0x08U
// #define REG_DUTY1               0x0CU
// #define REG_DUTY2               0x10U
// #define REG_DUTY3               0x14U
// #define MOTOR_EN_BIT(ch)        (1u << (ch))
// #define MOTOR_DIR_BIT(ch)       (1u << (4u + (ch)))
// #define MOTOR_BRAKE_BIT(ch)     (1u << (8u + (ch)))

// #define LEFT_FORWARD_DIR_TRUE   0
// #define RIGHT_FORWARD_DIR_TRUE  1

// /* ============================================================
//  * PS I2C1 / BNO055 (telemetry only; line-following uses camera)
//  * ============================================================ */
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
//  * ============================================================ */
// #ifndef XPAR_XUARTLITE_0_BASEADDR
// #error "XPAR_XUARTLITE_0_BASEADDR is not defined"
// #endif
// #define SONAR_UART_BASEADDR     XPAR_XUARTLITE_0_BASEADDR

// /* ============================================================
//  * Line-following / vision tuning knobs
//  * ============================================================ */
// /* ROI = top band of the frame because the camera is mounted upside down.
//  * In the rotated image, the floor region nearest the robot appears at the top. */
// #define ROI_Y_TOP               0       /* 0..359 = top 360 rows */
// #define ROI_Y_BOTTOM            359
// #define ROI_STRIDE_X            24      /* sample every Nth column */
// #define ROI_STRIDE_Y            12      /* sample every Nth row */

// /* Brightness threshold for classifying a pixel as line.
//  * Luma = (R + 2G + B) >> 2  (range 0..255)
//  * For DARK line on LIGHT floor: LINE_IS_DARK=1, LINE_LUMA_THRESH=80.
//  * For LIGHT line on DARK floor: LINE_IS_DARK=0, LINE_LUMA_THRESH=160. */
// #define LINE_IS_DARK            1
// #define LINE_LUMA_THRESH        80

// /* Minimum line-pixel count for detection to be considered valid. */
// #define MIN_LINE_PIXELS         30

// /* Steering P-gain (pixel_error -> duty delta). 1/2 maps +/-960 px
//  * error to +/-480 duty delta around MOTOR_DUTY_BASE. */
// #define STEER_KP_NUM            1
// #define STEER_KP_DEN            4

// /* ============================================================
//  * Motor / behavior knobs
//  * ============================================================ */
// #define MOTOR_PWM_PERIOD        5000U
// #define MOTOR_DUTY_BASE         4750U
// #define MOTOR_DUTY_MIN          4500U
// #define MOTOR_DUTY_MAX          5000U

// #define SONAR_STOP_INCHES       18

// #define SENSOR_TASK_PERIOD_MS   100U
// #define VISION_TASK_PERIOD_MS   100U
// #define MOTOR_TASK_PERIOD_MS    50U
// #define TELEMETRY_PERIOD_MS     500U

// /* Debug / stability knobs */
// #define CACHELINE_BYTES         32U
// #define VISION_DRAW_HIT_BOXES   0   /* 0 = only draw center/reference lines */
// #define VISION_DRAW_ALL_FRAMES  1   /* 1 = stamp overlay into all 3 buffers */

// #define STACK_LOGGER_TASK       2048U
// #define STACK_SENSOR_TASK       2048U
// #define STACK_SONAR_TASK        1024U
// #define STACK_VISION_TASK       4096U
// #define STACK_MOTOR_TASK        2048U
// #define STACK_TELEM_TASK        2048U

// #define PRIO_SENSOR_TASK        (tskIDLE_PRIORITY + 3)
// #define PRIO_MOTOR_TASK         (tskIDLE_PRIORITY + 3)
// #define PRIO_LOGGER_TASK        (tskIDLE_PRIORITY + 2)
// #define PRIO_VISION_TASK        (tskIDLE_PRIORITY + 2)
// #define PRIO_SONAR_TASK         (tskIDLE_PRIORITY + 1)
// #define PRIO_TELEM_TASK         (tskIDLE_PRIORITY + 1)

// #define LOG_QUEUE_LENGTH        24U

// enum DriveMode : uint8_t {
//     DRIVE_IDLE = 0,
//     DRIVE_FOLLOWING,
//     DRIVE_OBSTACLE_STOP,
//     DRIVE_SEARCHING,
// };

// struct SharedState {
//     bool     video_ok;
//     bool     bno_ok;
//     bool     sonar_ok;

//     int16_t  ax, ay, az;
//     int16_t  gx, gy, gz;

//     int      sonar_inches;
//     bool     obstacle;

//     bool     lane_valid;
//     int32_t  lane_center_x;
//     int32_t  lane_error_px;
//     uint32_t line_pixels;

//     uint8_t  drive_mode;
//     uint32_t left_duty;
//     uint32_t right_duty;
// };

// enum LogType : uint8_t {
//     LOG_SENSOR_INIT_START = 0,
//     LOG_SENSOR_BNO_OK,
//     LOG_SENSOR_BNO_FAIL,
//     LOG_SONAR_INIT_START,
//     LOG_SONAR_UART_OK,
//     LOG_VISION_READY,
//     LOG_MOTOR_INIT_START,
//     LOG_MOTOR_ACTIVE,
//     LOG_OBSTACLE_DETECTED,
//     LOG_OBSTACLE_CLEARED,
//     LOG_TELEMETRY,
// };

// struct LogEvent {
//     LogType     type;
//     SharedState snap;
// };

// struct MotorChannelCmd {
//     uint16_t duty;
//     bool     enable;
//     bool     dir;
//     bool     brake;
// };

// static SharedState   g_state{};
// static XIicPs        g_iic{};
// static XUartLite     g_uart{};
// static QueueHandle_t g_log_queue = nullptr;

// static TaskHandle_t  g_logger_task = nullptr;
// static TaskHandle_t  g_sensor_task = nullptr;
// static TaskHandle_t  g_sonar_task  = nullptr;
// static TaskHandle_t  g_vision_task = nullptr;
// static TaskHandle_t  g_motor_task  = nullptr;
// static TaskHandle_t  g_telem_task  = nullptr;

// /* Forward declarations */
// static void vLoggerTask(void *);
// static void vSensorTask(void *);
// static void vSonarTask(void *);
// static void vVisionTask(void *);
// static void vMotorTask(void *);
// static void vTelemetryTask(void *);
// static int  video_init_once(void);

// /* ============================================================
//  * Small utility helpers
//  * ============================================================ */
// static inline int32_t i_clamp(int32_t x, int32_t lo, int32_t hi) {
//     if (x < lo) return lo;
//     if (x > hi) return hi;
//     return x;
// }
// static inline int16_t le16_to_s16(uint8_t lsb, uint8_t msb) {
//     return (int16_t)(((uint16_t)msb << 8) | (uint16_t)lsb);
// }
// static inline uintptr_t cacheline_floor(uintptr_t addr) {
//     return addr & ~((uintptr_t)CACHELINE_BYTES - 1u);
// }
// static inline uint32_t cacheline_span(uintptr_t start, uint32_t len) {
//     const uintptr_t end = start + (uintptr_t)len;
//     const uintptr_t end_aligned = (end + (CACHELINE_BYTES - 1u)) & ~((uintptr_t)CACHELINE_BYTES - 1u);
//     const uintptr_t start_aligned = cacheline_floor(start);
//     return (uint32_t)(end_aligned - start_aligned);
// }

// /* ============================================================
//  * Shared-state updaters
//  * ============================================================ */
// static void update_video(bool ok) {
//     taskENTER_CRITICAL();
//     g_state.video_ok = ok;
//     taskEXIT_CRITICAL();
// }
// static void update_bno(int16_t ax, int16_t ay, int16_t az,
//                        int16_t gx, int16_t gy, int16_t gz, bool ok) {
//     taskENTER_CRITICAL();
//     g_state.bno_ok = ok;
//     g_state.ax = ax; g_state.ay = ay; g_state.az = az;
//     g_state.gx = gx; g_state.gy = gy; g_state.gz = gz;
//     taskEXIT_CRITICAL();
// }
// static void update_sonar(int inches, bool ok, bool obstacle) {
//     taskENTER_CRITICAL();
//     g_state.sonar_ok     = ok;
//     g_state.sonar_inches = inches;
//     g_state.obstacle     = obstacle;
//     taskEXIT_CRITICAL();
// }
// static void update_vision(bool valid, int32_t center_x,
//                           int32_t error_px, uint32_t line_pixels) {
//     taskENTER_CRITICAL();
//     g_state.lane_valid    = valid;
//     g_state.lane_center_x = center_x;
//     g_state.lane_error_px = error_px;
//     g_state.line_pixels   = line_pixels;
//     taskEXIT_CRITICAL();
// }
// static void update_drive(DriveMode mode, uint32_t left_duty, uint32_t right_duty) {
//     taskENTER_CRITICAL();
//     g_state.drive_mode = (uint8_t)mode;
//     g_state.left_duty  = left_duty;
//     g_state.right_duty = right_duty;
//     taskEXIT_CRITICAL();
// }
// static SharedState snapshot_state(void) {
//     SharedState snap;
//     taskENTER_CRITICAL();
//     snap = g_state;
//     taskEXIT_CRITICAL();
//     return snap;
// }

// /* ============================================================
//  * Log queue helpers
//  * ============================================================ */
// static void log_send_type(LogType type) {
//     if (g_log_queue == nullptr) return;
//     LogEvent ev{};
//     ev.type = type;
//     (void)xQueueSend(g_log_queue, &ev, 0);
// }
// static void log_send_telem(const SharedState &snap) {
//     if (g_log_queue == nullptr) return;
//     LogEvent ev{};
//     ev.type = LOG_TELEMETRY;
//     ev.snap = snap;
//     (void)xQueueSend(g_log_queue, &ev, 0);
// }

// /* ============================================================
//  * PS I2C1 / BNO055
//  * ============================================================ */
// static int wait_bus_idle(XIicPs *iic) {
//     int timeout = 1000000;
//     while (XIicPs_BusIsBusy(iic) && timeout > 0) timeout--;
//     return (timeout > 0) ? XST_SUCCESS : XST_FAILURE;
// }
// static int sensor_i2c_init(void) {
//     XIicPs_Config *cfg;
//     int status;
// #ifdef SDT
//     cfg = XIicPs_LookupConfig(XPAR_XIICPS_1_BASEADDR);
// #else
//     cfg = XIicPs_LookupConfig(XPAR_XIICPS_1_DEVICE_ID);
// #endif
//     if (cfg == nullptr) return XST_FAILURE;
//     status = XIicPs_CfgInitialize(&g_iic, cfg, cfg->BaseAddress); if (status != XST_SUCCESS) return status;
//     status = XIicPs_SelfTest(&g_iic);                             if (status != XST_SUCCESS) return status;
//     status = XIicPs_SetSClk(&g_iic, SENSOR_I2C_SCLK_HZ);          if (status != XST_SUCCESS) return status;
//     status = XIicPs_SetOptions(&g_iic, XIICPS_7_BIT_ADDR_OPTION); if (status != XST_SUCCESS) return status;
//     return XST_SUCCESS;
// }
// static int i2c_write_reg8(uint16_t addr, uint8_t reg, uint8_t value) {
//     uint8_t tx[2] = {reg, value};
//     int status = XIicPs_MasterSendPolled(&g_iic, tx, 2, addr);
//     if (status != XST_SUCCESS) return status;
//     return wait_bus_idle(&g_iic);
// }
// static int i2c_read_bytes(uint16_t addr, uint8_t reg, uint8_t *rx, int len) {
//     int status = XIicPs_SetOptions(&g_iic, XIICPS_7_BIT_ADDR_OPTION | XIICPS_REP_START_OPTION);
//     if (status != XST_SUCCESS) return status;
//     status = XIicPs_MasterSendPolled(&g_iic, &reg, 1, addr);
//     if (status != XST_SUCCESS) { XIicPs_ClearOptions(&g_iic, XIICPS_REP_START_OPTION); return status; }
//     status = XIicPs_MasterRecvPolled(&g_iic, rx, len, addr);
//     XIicPs_ClearOptions(&g_iic, XIICPS_REP_START_OPTION);
//     if (status != XST_SUCCESS) return status;
//     return wait_bus_idle(&g_iic);
// }
// static int bno055_init(void) {
//     uint8_t chip_id = 0;
//     int status;
//     vTaskDelay(pdMS_TO_TICKS(800));
//     for (int tries = 0; tries < 10; tries++) {
//         status = i2c_read_bytes(BNO055_I2C_ADDR, BNO055_CHIP_ID_REG, &chip_id, 1);
//         if ((status == XST_SUCCESS) && (chip_id == BNO055_CHIP_ID_VAL)) break;
//         vTaskDelay(pdMS_TO_TICKS(50));
//     }
//     if (chip_id != BNO055_CHIP_ID_VAL) return XST_FAILURE;
//     status = i2c_write_reg8(BNO055_I2C_ADDR, BNO055_PAGE_ID_REG, 0x00U);              if (status != XST_SUCCESS) return status;
//     status = i2c_write_reg8(BNO055_I2C_ADDR, BNO055_OPR_MODE_REG, BNO055_OPR_CONFIG); if (status != XST_SUCCESS) return status;
//     vTaskDelay(pdMS_TO_TICKS(25));
//     status = i2c_write_reg8(BNO055_I2C_ADDR, BNO055_PWR_MODE_REG, BNO055_PWR_NORMAL); if (status != XST_SUCCESS) return status;
//     vTaskDelay(pdMS_TO_TICKS(10));
//     status = i2c_write_reg8(BNO055_I2C_ADDR, BNO055_UNIT_SEL_REG, 0x00U);             if (status != XST_SUCCESS) return status;
//     vTaskDelay(pdMS_TO_TICKS(10));
//     status = i2c_write_reg8(BNO055_I2C_ADDR, BNO055_OPR_MODE_REG, BNO055_OPR_AMG);    if (status != XST_SUCCESS) return status;
//     vTaskDelay(pdMS_TO_TICKS(20));
//     return XST_SUCCESS;
// }

// /* ============================================================
//  * Sonar (AXI UARTLite)
//  * ============================================================ */
// static int sonar_uart_init(void) {
//     int status;
// #ifdef SDT
//     status = XUartLite_Initialize(&g_uart, SONAR_UART_BASEADDR);
// #else
//     status = XUartLite_Initialize(&g_uart, XPAR_UARTLITE_0_DEVICE_ID);
// #endif
//     if (status != XST_SUCCESS) return status;
//     return XUartLite_SelfTest(&g_uart);
// }
// static int uartlite_get_byte(uint8_t *byte_out) {
//     const unsigned n = XUartLite_Recv(&g_uart, byte_out, 1);
//     return (n == 1U) ? 1 : 0;
// }
// static int is_digit_u8(uint8_t c) { return (c >= '0' && c <= '9'); }

// /* ============================================================
//  * Motor helpers
//  * ============================================================ */
// static inline void motor_wr(unsigned offset, unsigned value) {
//     Xil_Out32(MOTOR_BASE + offset, value);
// }
// static void motor_apply(const MotorChannelCmd cmd[4]) {
//     uint32_t ctrl = 0;
//     motor_wr(REG_DUTY0, cmd[0].duty);
//     motor_wr(REG_DUTY1, cmd[1].duty);
//     motor_wr(REG_DUTY2, cmd[2].duty);
//     motor_wr(REG_DUTY3, cmd[3].duty);
//     for (unsigned ch = 0; ch < 4; ch++) {
//         if (cmd[ch].enable) ctrl |= MOTOR_EN_BIT(ch);
//         if (cmd[ch].dir)    ctrl |= MOTOR_DIR_BIT(ch);
//         if (cmd[ch].brake)  ctrl |= MOTOR_BRAKE_BIT(ch);
//     }
//     motor_wr(REG_CTRL, ctrl);
// }
// static void motor_all_stop(void) {
//     const MotorChannelCmd cmd[4] = {
//         {0, false, false, false}, {0, false, false, false},
//         {0, false, false, false}, {0, false, false, false},
//     };
//     motor_apply(cmd);
// }
// static void motor_all_brake(void) {
//     const MotorChannelCmd cmd[4] = {
//         {0, true, false, true}, {0, true, false, true},
//         {0, true, false, true}, {0, true, false, true},
//     };
//     motor_apply(cmd);
// }
// /* Differential drive: independent duty for left and right sides. */
// static void motor_drive_lr(uint16_t left_duty, uint16_t right_duty) {
//     const bool left_dir  = (LEFT_FORWARD_DIR_TRUE  != 0);
//     const bool right_dir = (RIGHT_FORWARD_DIR_TRUE != 0);
//     const MotorChannelCmd cmd[4] = {
//         {left_duty,  true, left_dir,  false},
//         {left_duty,  true, left_dir,  false},
//         {right_duty, true, right_dir, false},
//         {right_duty, true, right_dir, false},
//     };
//     motor_apply(cmd);
// }

// /* ============================================================
//  * Video pipeline init (unchanged from Phase 4 working code)
//  * ============================================================ */
// static void pipeline_mode_change(
//     AXI_VDMA<ScuGicInterruptController> &vdma_driver,
//     OV5640 &cam,
//     VideoOutput &vid,
//     Resolution res,
//     OV5640_cfg::mode_t mode)
// {
//     { vdma_driver.resetWrite();
//       MIPI_CSI_2_RX_mWriteReg(XPAR_MIPI_CSI_2_RX_0_S_AXI_LITE_BASEADDR, CR_OFFSET, (CR_RESET_MASK & ~CR_ENABLE_MASK));
//       MIPI_D_PHY_RX_mWriteReg(XPAR_MIPI_D_PHY_RX_0_S_AXI_LITE_BASEADDR, CR_OFFSET, (CR_RESET_MASK & ~CR_ENABLE_MASK));
//       cam.reset(); }
//     { vdma_driver.configureWrite(timing[(int)res].h_active, timing[(int)res].v_active);
//       Xil_Out32(GAMMA_BASE_ADDR, 3U); cam.init(); }
//     { vdma_driver.enableWrite();
//       MIPI_CSI_2_RX_mWriteReg(XPAR_MIPI_CSI_2_RX_0_S_AXI_LITE_BASEADDR, CR_OFFSET, CR_ENABLE_MASK);
//       MIPI_D_PHY_RX_mWriteReg(XPAR_MIPI_D_PHY_RX_0_S_AXI_LITE_BASEADDR, CR_OFFSET, CR_ENABLE_MASK);
//       cam.set_mode(mode); cam.set_awb(OV5640_cfg::awb_t::AWB_ADVANCED); }
//     { vid.reset(); vdma_driver.resetRead(); }
//     { vid.configure(res); vdma_driver.configureRead(timing[(int)res].h_active, timing[(int)res].v_active); }
//     { vid.enable(); vdma_driver.enableRead(); }
// }

// static int video_init_once(void) {
//     try {
//         static ScuGicInterruptController irpt_ctl(IRPT_CTL_DEVID);
//         static PS_GPIO<ScuGicInterruptController> gpio_driver(GPIO_DEVID, irpt_ctl, GPIO_IRPT_ID);
//         static PS_IIC<ScuGicInterruptController> iic_driver(CAM_I2C_DEVID, irpt_ctl, CAM_I2C_IRPT_ID, 100000);
//         static OV5640 cam(iic_driver, gpio_driver);
//         static AXI_VDMA<ScuGicInterruptController> vdma_driver(VDMA_DEVID, MEM_BASE_ADDR, irpt_ctl, VDMA_MM2S_IRPT_ID, VDMA_S2MM_IRPT_ID);
//         static VideoOutput vid(XPAR_VTC_0_DEVICE_ID, XPAR_VIDEO_DYNCLK_DEVICE_ID);
//         pipeline_mode_change(vdma_driver, cam, vid,
//             Resolution::R1920_1080_60_PP,
//             OV5640_cfg::mode_t::MODE_1080P_1920_1080_30fps);
//         return XST_SUCCESS;
//     } catch (...) { return XST_FAILURE; }
// }

// /* ============================================================
//  * Vision helpers
//  * Frame layout: 1920 x 1080, RGB24 packed (3 bytes per pixel).
//  * ============================================================ */
// static inline uint8_t* pixel_ptr(uintptr_t frame_base, int x, int y) {
//     return (uint8_t*)(frame_base + (uintptr_t)y * FRAME_STRIDE + (uintptr_t)x * BYTES_PER_PIXEL);
// }
// static inline uint8_t pixel_luma(uintptr_t frame_base, int x, int y) {
//     const uint8_t *p = pixel_ptr(frame_base, x, y);
// #if PIXEL_ORDER_BGR
//     const uint8_t b = p[0], g = p[1], r = p[2];
// #else
//     const uint8_t r = p[0], g = p[1], b = p[2];
// #endif
//     return (uint8_t)((r + (g << 1) + b) >> 2);
// }
// static inline void stamp_pixel_all_frames(int x, int y, uint8_t r, uint8_t g, uint8_t b) {
//     if (x < 0 || x >= FRAME_W || y < 0 || y >= FRAME_H) return;
// #if VISION_DRAW_ALL_FRAMES
//     const uintptr_t frames[3] = { FRAME0_ADDR, FRAME1_ADDR, FRAME2_ADDR };
//     const int frame_count = 3;
// #else
//     const uintptr_t frames[1] = { FRAME0_ADDR };
//     const int frame_count = 1;
// #endif
//     for (int i = 0; i < frame_count; i++) {
//         uint8_t *p = pixel_ptr(frames[i], x, y);
// #if PIXEL_ORDER_BGR
//         p[0] = b; p[1] = g; p[2] = r;
// #else
//         p[0] = r; p[1] = g; p[2] = b;
// #endif
//     }
// }
// static void stamp_box(int cx, int cy, int half_w, int half_h,
//                       uint8_t r, uint8_t g, uint8_t b) {
//     for (int dy = -half_h; dy <= half_h; dy++)
//         for (int dx = -half_w; dx <= half_w; dx++)
//             stamp_pixel_all_frames(cx + dx, cy + dy, r, g, b);
// }
// static void stamp_vline(int cx, int line_width, uint8_t r, uint8_t g, uint8_t b) {
//     const int half = line_width / 2;
//     for (int y = ROI_Y_TOP; y <= ROI_Y_BOTTOM; y++)
//         for (int dx = -half; dx <= half; dx++)
//             stamp_pixel_all_frames(cx + dx, y, r, g, b);
// }

// /* Core line-detection + overlay pass.
//  * Returns true if a valid line was detected.
//  * Out: *center_x = detected column, *err_px = signed error from image center. */
// static bool vision_process_frame(int32_t *center_x,
//                                  int32_t *err_px,
//                                  uint32_t *line_pixel_count) {
//     const uintptr_t read_base = FRAME0_ADDR;
//     const uintptr_t roi_start = read_base + (uintptr_t)ROI_Y_TOP * FRAME_STRIDE;
//     const uint32_t  roi_bytes = (uint32_t)(ROI_Y_BOTTOM - ROI_Y_TOP + 1) * FRAME_STRIDE;
//     const uintptr_t roi_start_aligned = cacheline_floor(roi_start);
//     const uint32_t  roi_bytes_aligned = cacheline_span(roi_start, roi_bytes);
//     Xil_DCacheInvalidateRange(roi_start_aligned, roi_bytes_aligned);

//     int64_t  sum_x      = 0;
//     uint32_t total_hits = 0;

//     for (int y = ROI_Y_TOP; y <= ROI_Y_BOTTOM; y += ROI_STRIDE_Y) {
//         for (int x = 0; x < FRAME_W; x += ROI_STRIDE_X) {
//             const uint8_t luma = pixel_luma(read_base, x, y);
// #if LINE_IS_DARK
//             const bool is_line = (luma < LINE_LUMA_THRESH);
// #else
//             const bool is_line = (luma > LINE_LUMA_THRESH);
// #endif
//             if (is_line) {
// #if VISION_DRAW_HIT_BOXES
//                 stamp_box(x, y, ROI_STRIDE_X / 2, ROI_STRIDE_Y / 2, 0x00, 0xFF, 0x00);
// #endif
//                 sum_x += x;
//                 total_hits++;
//             }
//         }
//     }

//     *line_pixel_count = total_hits;

//     bool valid = false;
//     int32_t cx  = FRAME_W / 2;
//     int32_t err = 0;
//     if (total_hits >= MIN_LINE_PIXELS) {
//         cx    = (int32_t)(sum_x / (int64_t)total_hits);
//         /* Camera is upside down, so left/right image interpretation is reversed. */
//         err   = (FRAME_W / 2) - cx;
//         valid = true;
//     }

//     /* Overlay reference lines: BLUE = image center, RED = detected center. */
//     stamp_vline(FRAME_W / 2, 3, 0x00, 0x40, 0xFF);
//     if (valid) stamp_vline(cx, 3, 0xFF, 0x20, 0x20);

//     /* Push overlay to DRAM so VDMA read side sees it. */
//     Xil_DCacheFlushRange(roi_start_aligned, roi_bytes_aligned);
// #if VISION_DRAW_ALL_FRAMES
//     Xil_DCacheFlushRange(cacheline_floor(FRAME1_ADDR + (uintptr_t)ROI_Y_TOP * FRAME_STRIDE), roi_bytes_aligned);
//     Xil_DCacheFlushRange(cacheline_floor(FRAME2_ADDR + (uintptr_t)ROI_Y_TOP * FRAME_STRIDE), roi_bytes_aligned);
// #endif

//     *center_x = cx;
//     *err_px   = err;
//     return valid;
// }

// /* ============================================================
//  * Logger + telemetry printing
//  * ============================================================ */
// static const char *drive_mode_name(uint8_t m) {
//     switch ((DriveMode)m) {
//     case DRIVE_IDLE:          return "IDLE";
//     case DRIVE_FOLLOWING:     return "FOLLOW";
//     case DRIVE_OBSTACLE_STOP: return "OBSTACLE";
//     case DRIVE_SEARCHING:     return "SEARCH";
//     default:                  return "?";
//     }
// }

// static void print_telem(const SharedState &s) {
//     xil_printf("\r\n[telem] video=%d bno=%d sonar=%d ",
//                s.video_ok ? 1 : 0, s.bno_ok ? 1 : 0, s.sonar_ok ? 1 : 0);
//     if (s.sonar_ok) xil_printf("range=%d in%s | ",
//                                s.sonar_inches, s.obstacle ? " [OBSTACLE]" : "");
//     else            xil_printf("sonar unavail | ");

//     if (s.lane_valid) {
//         xil_printf("lane: cx=%d err=%d hits=%u | ",
//                    (int)s.lane_center_x, (int)s.lane_error_px, (unsigned)s.line_pixels);
//     } else {
//         xil_printf("lane: NOT FOUND (hits=%u) | ", (unsigned)s.line_pixels);
//     }

//     xil_printf("mode=%s L=%u R=%u\r\n",
//                drive_mode_name(s.drive_mode),
//                (unsigned)s.left_duty, (unsigned)s.right_duty);
// }

// static void print_runtime_health(void) {
// #if (INCLUDE_uxTaskGetStackHighWaterMark == 1)
//     const UBaseType_t log_hwm    = g_logger_task ? uxTaskGetStackHighWaterMark(g_logger_task) : 0;
//     const UBaseType_t sensor_hwm = g_sensor_task ? uxTaskGetStackHighWaterMark(g_sensor_task) : 0;
//     const UBaseType_t sonar_hwm  = g_sonar_task  ? uxTaskGetStackHighWaterMark(g_sonar_task)  : 0;
//     const UBaseType_t vision_hwm = g_vision_task ? uxTaskGetStackHighWaterMark(g_vision_task) : 0;
//     const UBaseType_t motor_hwm  = g_motor_task  ? uxTaskGetStackHighWaterMark(g_motor_task)  : 0;
//     const UBaseType_t telem_hwm  = g_telem_task  ? uxTaskGetStackHighWaterMark(g_telem_task)  : 0;
//     xil_printf("[stack] log=%u sensor=%u sonar=%u vision=%u motor=%u telem=%u words\r\n",
//                (unsigned)log_hwm, (unsigned)sensor_hwm, (unsigned)sonar_hwm,
//                (unsigned)vision_hwm, (unsigned)motor_hwm, (unsigned)telem_hwm);
// #endif

// #if (configSUPPORT_DYNAMIC_ALLOCATION == 1)
//     xil_printf("[heap] free=%u min_ever=%u bytes\r\n",
//                (unsigned)xPortGetFreeHeapSize(),
//                (unsigned)xPortGetMinimumEverFreeHeapSize());
// #endif
// }

// static void vLoggerTask(void *) {
//     LogEvent ev{};

//     xil_printf("\r\n========================================\r\n");
//     xil_printf("FreeRTOS line-following robot\r\n");
//     xil_printf("Video base  = 0x%08x (3 frames, RGB24 1920x1080)\r\n", (unsigned)MEM_BASE_ADDR);
//     xil_printf("Motor base  = 0x%08x\r\n", (unsigned)MOTOR_BASE);
//     xil_printf("UARTLite    = 0x%08x\r\n", (unsigned)SONAR_UART_BASEADDR);
//     if (g_state.video_ok) xil_printf("[video] passthrough init ok\r\n");
//     else                  xil_printf("[video] passthrough init FAILED\r\n");
//     xil_printf("[rtos] starting scheduler\r\n");

//     for (;;) {
//         if (xQueueReceive(g_log_queue, &ev, portMAX_DELAY) != pdPASS) continue;
//         switch (ev.type) {
//         case LOG_SENSOR_INIT_START: xil_printf("[sensor] init start\r\n");               break;
//         case LOG_SENSOR_BNO_OK:     xil_printf("[sensor] BNO055 init ok\r\n");           break;
//         case LOG_SENSOR_BNO_FAIL:   xil_printf("[sensor] BNO055 unavailable "
//                                                "(telemetry only, continuing)\r\n");     break;
//         case LOG_SONAR_INIT_START:  xil_printf("[sonar] init start\r\n");                break;
//         case LOG_SONAR_UART_OK:     xil_printf("[sonar] UARTLite init ok\r\n");          break;
//         case LOG_VISION_READY:      xil_printf("[vision] detection active, "
//                                                "overlay visible on HDMI\r\n");          break;
//         case LOG_MOTOR_INIT_START:  xil_printf("[motor] init start\r\n");                break;
//         case LOG_MOTOR_ACTIVE:      xil_printf("[motor] line-follow control active\r\n");break;
//         case LOG_OBSTACLE_DETECTED: xil_printf("obstacle detected help please!\r\n");    break;
//         case LOG_OBSTACLE_CLEARED:  xil_printf("obstacle cleared, resuming\r\n");        break;
//         case LOG_TELEMETRY:         print_telem(ev.snap);                                break;
//         default: break;
//         }
//     }
// }

// /* ============================================================
//  * Sensor task (BNO055 telemetry only, non-critical)
//  * ============================================================ */
// static void vSensorTask(void *) {
//     TickType_t last_wake = xTaskGetTickCount();
//     const TickType_t period = pdMS_TO_TICKS(SENSOR_TASK_PERIOD_MS);

//     log_send_type(LOG_SENSOR_INIT_START);

//     int status;
//     for (;;) {
//         status = sensor_i2c_init();
//         if (status == XST_SUCCESS) break;
//         vTaskDelay(pdMS_TO_TICKS(1000));
//     }

//     int bno_tries = 0;
//     bool bno_ready = false;
//     while (bno_tries < 5) {
//         if (bno055_init() == XST_SUCCESS) { bno_ready = true; break; }
//         update_bno(0, 0, 0, 0, 0, 0, false);
//         bno_tries++;
//         vTaskDelay(pdMS_TO_TICKS(500));
//     }
//     if (bno_ready) log_send_type(LOG_SENSOR_BNO_OK);
//     else           log_send_type(LOG_SENSOR_BNO_FAIL);

//     for (;;) {
//         if (bno_ready) {
//             uint8_t acc[6], gyr[6];
//             int s1 = i2c_read_bytes(BNO055_I2C_ADDR, BNO055_ACC_DATA_X_LSB, acc, 6);
//             int s2 = (s1 == XST_SUCCESS)
//                    ? i2c_read_bytes(BNO055_I2C_ADDR, BNO055_GYR_DATA_X_LSB, gyr, 6)
//                    : s1;
//             if (s2 == XST_SUCCESS) {
//                 update_bno(le16_to_s16(acc[0], acc[1]),
//                            le16_to_s16(acc[2], acc[3]),
//                            le16_to_s16(acc[4], acc[5]),
//                            le16_to_s16(gyr[0], gyr[1]),
//                            le16_to_s16(gyr[2], gyr[3]),
//                            le16_to_s16(gyr[4], gyr[5]),
//                            true);
//             } else {
//                 update_bno(0, 0, 0, 0, 0, 0, false);
//             }
//         }
//         vTaskDelayUntil(&last_wake, period);
//     }
// }

// /* ============================================================
//  * Sonar task -- parse RXXX\r frames, set obstacle flag
//  * ============================================================ */
// static void vSonarTask(void *) {
//     int status;
//     uint8_t c = 0, d1 = 0, d2 = 0, d3 = 0;
//     int state = 0;
//     bool last_obstacle = false;

//     log_send_type(LOG_SONAR_INIT_START);

//     for (;;) {
//         status = sonar_uart_init();
//         if (status == XST_SUCCESS) break;
//         vTaskDelay(pdMS_TO_TICKS(1000));
//     }
//     log_send_type(LOG_SONAR_UART_OK);

//     for (;;) {
//         if (!uartlite_get_byte(&c)) { vTaskDelay(pdMS_TO_TICKS(1)); continue; }
//         switch (state) {
//         case 0: if (c == 'R') state = 1; break;
//         case 1: if (is_digit_u8(c)) { d1 = c; state = 2; } else if (c == 'R') state = 1; else state = 0; break;
//         case 2: if (is_digit_u8(c)) { d2 = c; state = 3; } else if (c == 'R') state = 1; else state = 0; break;
//         case 3: if (is_digit_u8(c)) { d3 = c; state = 4; } else if (c == 'R') state = 1; else state = 0; break;
//         case 4:
//             if (c == '\r') {
//                 const int inches = (d1 - '0') * 100 + (d2 - '0') * 10 + (d3 - '0');
//                 const bool obstacle = (inches > 0) && (inches <= SONAR_STOP_INCHES);
//                 update_sonar(inches, true, obstacle);
//                 /* Edge-triggered: "obstacle detected help please!" prints once
//                  * on entering obstacle state, and "obstacle cleared" prints
//                  * once when the range opens back up. */
//                 if (obstacle && !last_obstacle) log_send_type(LOG_OBSTACLE_DETECTED);
//                 if (!obstacle && last_obstacle) log_send_type(LOG_OBSTACLE_CLEARED);
//                 last_obstacle = obstacle;
//             }
//             state = 0;
//             break;
//         default: state = 0; break;
//         }
//         vTaskDelay(pdMS_TO_TICKS(1));
//     }
// }

// /* ============================================================
//  * Vision task -- detect line, update shared state, draw overlay
//  * ============================================================ */
// static void vVisionTask(void *) {
//     TickType_t last_wake = xTaskGetTickCount();
//     const TickType_t period = pdMS_TO_TICKS(VISION_TASK_PERIOD_MS);

//     vTaskDelay(pdMS_TO_TICKS(500));  /* let video pipeline settle */
//     last_wake = xTaskGetTickCount();
//     log_send_type(LOG_VISION_READY);

//     for (;;) {
//         int32_t  center_x  = FRAME_W / 2;
//         int32_t  err_px    = 0;
//         uint32_t hit_count = 0;
//         const bool valid = vision_process_frame(&center_x, &err_px, &hit_count);
//         update_vision(valid, center_x, err_px, hit_count);
//         vTaskDelayUntil(&last_wake, period);
//     }
// }

// /* ============================================================
//  * Motor task -- line-following P-controller with obstacle stop
//  * ============================================================ */
// static void vMotorTask(void *) {
//     TickType_t last_wake = xTaskGetTickCount();
//     const TickType_t period = pdMS_TO_TICKS(MOTOR_TASK_PERIOD_MS);

//     log_send_type(LOG_MOTOR_INIT_START);
//     motor_wr(REG_PERIOD, MOTOR_PWM_PERIOD);
//     motor_all_stop();
//     log_send_type(LOG_MOTOR_ACTIVE);

//     for (;;) {
//         const SharedState s = snapshot_state();

//         /* Priority: sonar obstacle > lane-follow > search */
//         if (s.sonar_ok && s.obstacle) {
//             motor_all_brake();
//             update_drive(DRIVE_OBSTACLE_STOP, 0, 0);
//         } else if (s.lane_valid) {
//             /* For the upside-down camera build, err_px has already been sign-corrected
//              * in vision_process_frame() so the same steering law still works here. */
//             const int32_t delta = (s.lane_error_px * STEER_KP_NUM) / STEER_KP_DEN;
//             int32_t left  = (int32_t)MOTOR_DUTY_BASE + delta;
//             int32_t right = (int32_t)MOTOR_DUTY_BASE - delta;
//             left  = i_clamp(left,  MOTOR_DUTY_MIN, MOTOR_DUTY_MAX);
//             right = i_clamp(right, MOTOR_DUTY_MIN, MOTOR_DUTY_MAX);
//             motor_drive_lr((uint16_t)left, (uint16_t)right);
//             update_drive(DRIVE_FOLLOWING, (uint32_t)left, (uint32_t)right);
//         } else {
//             /* No line: wait in place. */
//             motor_all_brake();
//             update_drive(DRIVE_SEARCHING, 0, 0);
//         }

//         vTaskDelayUntil(&last_wake, period);
//     }
// }

// /* ============================================================
//  * Telemetry task
//  * ============================================================ */
// static void vTelemetryTask(void *) {
//     TickType_t last_wake = xTaskGetTickCount();
//     const TickType_t period = pdMS_TO_TICKS(TELEMETRY_PERIOD_MS);
//     uint32_t counter = 0;
//     for (;;) {
//         log_send_telem(snapshot_state());
//         counter++;
//         if ((counter % 10u) == 0u) print_runtime_health();
//         vTaskDelayUntil(&last_wake, period);
//     }
// }

// } // namespace

// extern "C" void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
//     (void)xTask;
//     taskDISABLE_INTERRUPTS();
//     xil_printf("\r\n[FATAL] stack overflow in task: %s\r\n",
//                pcTaskName ? pcTaskName : "unknown");
// #if (configSUPPORT_DYNAMIC_ALLOCATION == 1)
//     xil_printf("[heap] free=%u min_ever=%u bytes\r\n",
//                (unsigned)xPortGetFreeHeapSize(),
//                (unsigned)xPortGetMinimumEverFreeHeapSize());
// #endif
//     for (;;) {}
// }
// extern "C" void vApplicationMallocFailedHook(void) {
//     taskDISABLE_INTERRUPTS();
//     xil_printf("\r\n[FATAL] pvPortMalloc failed\r\n");
// #if (configSUPPORT_DYNAMIC_ALLOCATION == 1)
//     xil_printf("[heap] free=%u min_ever=%u bytes\r\n",
//                (unsigned)xPortGetFreeHeapSize(),
//                (unsigned)xPortGetMinimumEverFreeHeapSize());
// #endif
//     for (;;) {}
// }

// int main() {
//     BaseType_t ok;
//     int status;

//     init_platform();

//     status = video_init_once();
//     update_video(status == XST_SUCCESS);

//     xil_printf("[boot] line-follow build: reduced overlay writes + stack/heap health enabled\r\n");

//     if (status == XST_SUCCESS) {
//         /* VDMA IRQs off */
//         const uintptr_t vdma = XPAR_XAXIVDMA_0_BASEADDR;
//         uint32_t cr;
//         cr = Xil_In32(vdma + 0x00);
//         Xil_Out32(vdma + 0x00, cr & ~((1u<<12)|(1u<<13)|(1u<<14)));
//         cr = Xil_In32(vdma + 0x30);
//         Xil_Out32(vdma + 0x30, cr & ~((1u<<12)|(1u<<13)|(1u<<14)));
//         Xil_Out32(vdma + 0x04, 0x00007000);
//         Xil_Out32(vdma + 0x34, 0x00007000);

//         /* VTC IRQs off */
//         const uintptr_t vtc = 0x43C10000u;
//         Xil_Out32(vtc + 0x0C, 0u);
//         Xil_Out32(vtc + 0x04, 0xFFFFFFFFu);

//         /* GIC-side kill for VTG/VDMA/PS-GPIO/PS-I2C0 */
//         #define GIC_ICDICER(id)     (0xF8F01180u + 4u * ((id) / 32u))
//         #define GIC_DISABLE_IRQ(id) Xil_Out32(GIC_ICDICER(id), 1u << ((id) % 32u))
//         GIC_DISABLE_IRQ(61);
//         GIC_DISABLE_IRQ(XPAR_FABRIC_AXI_VDMA_0_MM2S_INTROUT_INTR);
//         GIC_DISABLE_IRQ(XPAR_FABRIC_AXI_VDMA_0_S2MM_INTROUT_INTR);
//         GIC_DISABLE_IRQ(XPAR_PS7_GPIO_0_INTR);
//         GIC_DISABLE_IRQ(XPAR_PS7_I2C_0_INTR);
//         xil_printf("[video] VTG + VDMA + PS I2C0 + PS GPIO IRQs disabled\r\n");
//     }

//     g_log_queue = xQueueCreate(LOG_QUEUE_LENGTH, sizeof(LogEvent));
//     if (g_log_queue == nullptr) { xil_printf("\r\n[FATAL] log queue create failed\r\n"); for (;;) {} }

//     ok = xTaskCreate(vLoggerTask,    "logger",  STACK_LOGGER_TASK,  nullptr, PRIO_LOGGER_TASK, &g_logger_task);
//     if (ok != pdPASS) { xil_printf("\r\n[FATAL] logger task create failed\r\n"); for (;;) {} }

//     ok = xTaskCreate(vSensorTask,    "sensor",  STACK_SENSOR_TASK,  nullptr, PRIO_SENSOR_TASK, &g_sensor_task);
//     if (ok != pdPASS) { xil_printf("\r\n[FATAL] sensor task create failed\r\n"); for (;;) {} }

//     ok = xTaskCreate(vSonarTask,     "sonar",   STACK_SONAR_TASK,   nullptr, PRIO_SONAR_TASK,  &g_sonar_task);
//     if (ok != pdPASS) { xil_printf("\r\n[FATAL] sonar task create failed\r\n"); for (;;) {} }

//     ok = xTaskCreate(vVisionTask,    "vision",  STACK_VISION_TASK,  nullptr, PRIO_VISION_TASK, &g_vision_task);
//     if (ok != pdPASS) { xil_printf("\r\n[FATAL] vision task create failed\r\n"); for (;;) {} }

//     ok = xTaskCreate(vMotorTask,     "motor",   STACK_MOTOR_TASK,   nullptr, PRIO_MOTOR_TASK,  &g_motor_task);
//     if (ok != pdPASS) { xil_printf("\r\n[FATAL] motor task create failed\r\n"); for (;;) {} }

//     ok = xTaskCreate(vTelemetryTask, "telem",   STACK_TELEM_TASK,   nullptr, PRIO_TELEM_TASK,  &g_telem_task);
//     if (ok != pdPASS) { xil_printf("\r\n[FATAL] telemetry task create failed\r\n"); for (;;) {} }

//     vTaskStartScheduler();
//     xil_printf("\r\n[FATAL] scheduler returned\r\n");
//     for (;;) {}
// }
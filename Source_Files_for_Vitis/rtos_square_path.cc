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
//  * Assumption: the 4-channel motor IP exposes contiguous duty
//  * registers and shared control bits laid out as:
//  *   enable bits: [3:0]
//  *   direction bits: [7:4]
//  *   brake bits: [11:8]
//  * If your IP uses a different map, only this section needs to change.
//  * ============================================================
//  */
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

// #define MOTOR_FL                0U
// #define MOTOR_BL                1U
// #define MOTOR_FR                2U
// #define MOTOR_BR                3U

// /* ============================================================
//  * PS I2C1 shared sensor bus: BNO055 + INA260
//  * ============================================================
//  */
// #ifndef XPAR_XIICPS_1_BASEADDR
// #error "XPAR_XIICPS_1_BASEADDR is not defined"
// #endif

// #define SENSOR_I2C_SCLK_HZ      50000U

// #define BNO055_I2C_ADDR         0x28U
// #define BNO055_CHIP_ID_REG      0x00U
// #define BNO055_PAGE_ID_REG      0x07U
// #define BNO055_GYR_DATA_X_LSB   0x14U
// #define BNO055_LIA_DATA_X_LSB   0x28U
// #define BNO055_UNIT_SEL_REG     0x3BU
// #define BNO055_OPR_MODE_REG     0x3DU
// #define BNO055_PWR_MODE_REG     0x3EU
// #define BNO055_CHIP_ID_VAL      0xA0U
// #define BNO055_PWR_NORMAL       0x00U
// #define BNO055_OPR_CONFIG       0x00U
// #define BNO055_OPR_IMUPLUS      0x08U

// #define INA260_I2C_ADDR         0x40U
// #define INA260_REG_CONFIG       0x00U
// #define INA260_REG_CURRENT      0x01U
// #define INA260_REG_BUS_VOLT     0x02U
// #define INA260_REG_POWER        0x03U
// #define INA260_REG_MANUF_ID     0xFEU
// #define INA260_REG_DIE_ID       0xFFU
// #define INA260_MANUF_ID_VAL     0x5449U
// #define INA260_DIE_ID_VAL       0x2270U

// /* ============================================================
//  * Sonar UART
//  * ============================================================
//  */
// #ifndef XPAR_XUARTLITE_0_BASEADDR
// #error "XPAR_XUARTLITE_0_BASEADDR is not defined"
// #endif

// #define SONAR_UART_BASEADDR     XPAR_XUARTLITE_0_BASEADDR

// /* ============================================================
//  * Navigation / tuning knobs
//  * ============================================================
//  */
// #define MOTOR_PWM_PERIOD            5000U
// #define MOTOR_DUTY_DRIVE            950U
// #define MOTOR_DUTY_REVERSE          900U
// #define MOTOR_DUTY_TURN             900U

// #define SONAR_STOP_INCHES           18
// #define SENSOR_TASK_PERIOD_MS       20U
// #define MOTOR_TASK_PERIOD_MS        20U
// #define TELEMETRY_PERIOD_MS         250U

// #define SQUARE_LEG_TARGET_MM        500
// #define RIGHT_TURN_TARGET_DEG       90
// #define NUM_SQUARE_LEGS             4U
// #define TURN_AFTER_FINAL_LEG        0

// #define FORWARD_CRUISE_MMPS         220
// #define REVERSE_CRUISE_MMPS         180
// #define MAX_TRACKED_SPEED_MMPS      450
// #define ACCEL_DEADBAND_CMS2         8
// #define GYRO_DEADBAND_CDPS          150
// #define VELOCITY_BLEND_PCT          12
// #define FINISH_SETTLE_MS            500U

// /* ------------------------------------------------------------
//  * Wiring / mounting knobs you may need to flip on hardware
//  * ------------------------------------------------------------
//  * LEFT/RIGHT *_DIR_TRUE values define which control-bit value makes
//  * each side move forward for your actual wiring.
//  *
//  * IMU_FORWARD_AXIS selects which BNO055 linear-acceleration axis is
//  * robot-forward. IMU_YAW_AXIS selects which gyro axis is the robot yaw.
//  * Change the sign macros if the mounting is reversed.
//  */
// #define LEFT_FORWARD_DIR_TRUE       0
// #define RIGHT_FORWARD_DIR_TRUE      1

// #define IMU_AXIS_X                  0
// #define IMU_AXIS_Y                  1
// #define IMU_AXIS_Z                  2
// #define IMU_FORWARD_AXIS            IMU_AXIS_Y
// #define IMU_FORWARD_SIGN            (+1)
// #define IMU_YAW_AXIS                IMU_AXIS_Z
// #define IMU_YAW_SIGN                (+1)

// #define STACK_LOGGER_TASK           2048U
// #define STACK_SENSOR_TASK           2048U
// #define STACK_SONAR_TASK            1024U
// #define STACK_MOTOR_TASK            2048U
// #define STACK_TELEM_TASK            2048U

// #define PRIO_SENSOR_TASK            (tskIDLE_PRIORITY + 3)
// #define PRIO_MOTOR_TASK             (tskIDLE_PRIORITY + 2)
// #define PRIO_LOGGER_TASK            (tskIDLE_PRIORITY + 2)
// #define PRIO_SONAR_TASK             (tskIDLE_PRIORITY + 1)
// #define PRIO_TELEM_TASK             (tskIDLE_PRIORITY + 1)

// #define LOG_QUEUE_LENGTH            32U

// enum NavMode : uint8_t {
//     NAV_WAIT_FOR_IMU = 0,
//     NAV_DRIVE_LEG,
//     NAV_TURN_RIGHT,
//     NAV_BACKTRACK,
//     NAV_RETRY_DELAY,
//     NAV_FINISHED
// };

// struct SharedState {
//     bool video_ok;
//     bool bno_ok;
//     bool sonar_ok;
//     bool ina_ok;

//     int16_t lax;
//     int16_t lay;
//     int16_t laz;
//     int16_t gx;
//     int16_t gy;
//     int16_t gz;

//     int sonar_inches;

//     int32_t ina_bus_mv;
//     int32_t ina_current_ma;
//     int32_t ina_power_mw;

//     uint8_t nav_mode;
//     uint8_t current_leg;
//     int32_t leg_progress_mm;
//     int32_t heading_cdeg;
// };

// enum LogType : uint8_t {
//     LOG_SENSOR_INIT_START = 0,
//     LOG_SENSOR_BNO_OK,
//     LOG_SENSOR_INA_OK,
//     LOG_SONAR_INIT_START,
//     LOG_SONAR_UART_OK,
//     LOG_MOTOR_INIT_START,
//     LOG_MOTOR_ACTIVE,
//     LOG_OBSTACLE_BACKTRACK,
//     LOG_FINISHED,
//     LOG_TELEMETRY
// };

// struct LogEvent {
//     LogType type;
//     SharedState snap;
// };

// struct MotorChannelCmd {
//     uint16_t duty;
//     bool enable;
//     bool dir;
//     bool brake;
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

// static inline float f_abs(float x)
// {
//     return (x < 0.0f) ? -x : x;
// }

// static inline float f_clamp(float x, float lo, float hi)
// {
//     if (x < lo)
//         return lo;
//     if (x > hi)
//         return hi;
//     return x;
// }

// static inline int32_t i_abs32(int32_t x)
// {
//     return (x < 0) ? -x : x;
// }

// static inline int16_t le16_to_s16(uint8_t lsb, uint8_t msb)
// {
//     return static_cast<int16_t>((static_cast<uint16_t>(msb) << 8) | static_cast<uint16_t>(lsb));
// }

// static inline uint16_t be16_to_u16(uint8_t msb, uint8_t lsb)
// {
//     return static_cast<uint16_t>((static_cast<uint16_t>(msb) << 8) | static_cast<uint16_t>(lsb));
// }

// static inline int16_t pick_axis_s16(int16_t x, int16_t y, int16_t z, int axis)
// {
//     switch (axis) {
//     case IMU_AXIS_X:
//         return x;
//     case IMU_AXIS_Y:
//         return y;
//     default:
//         return z;
//     }
// }

// static inline float bno_lia_raw_to_mps2(int16_t raw)
// {
//     return static_cast<float>(raw) / 100.0f;
// }

// static inline float bno_gyro_raw_to_dps(int16_t raw)
// {
//     return static_cast<float>(raw) / 16.0f;
// }

// static inline int32_t mps_to_mmps(float mps)
// {
//     return static_cast<int32_t>(mps * 1000.0f);
// }

// static inline int32_t dps_to_cdegps(float dps)
// {
//     return static_cast<int32_t>(dps * 100.0f);
// }

// static void update_bno(int16_t lax, int16_t lay, int16_t laz, int16_t gx, int16_t gy, int16_t gz, bool ok)
// {
//     taskENTER_CRITICAL();
//     g_state.bno_ok = ok;
//     g_state.lax = lax;
//     g_state.lay = lay;
//     g_state.laz = laz;
//     g_state.gx = gx;
//     g_state.gy = gy;
//     g_state.gz = gz;
//     taskEXIT_CRITICAL();
// }

// static void update_ina(int32_t bus_mv, int32_t current_ma, int32_t power_mw, bool ok)
// {
//     taskENTER_CRITICAL();
//     g_state.ina_ok = ok;
//     g_state.ina_bus_mv = bus_mv;
//     g_state.ina_current_ma = current_ma;
//     g_state.ina_power_mw = power_mw;
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

// static void update_nav(NavMode mode, uint8_t current_leg, int32_t leg_progress_mm, int32_t heading_cdeg)
// {
//     taskENTER_CRITICAL();
//     g_state.nav_mode = static_cast<uint8_t>(mode);
//     g_state.current_leg = current_leg;
//     g_state.leg_progress_mm = leg_progress_mm;
//     g_state.heading_cdeg = heading_cdeg;
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

// static int i2c_write_reg16_be(uint16_t addr, uint8_t reg, uint16_t value)
// {
//     uint8_t tx[3];
//     int status;

//     tx[0] = reg;
//     tx[1] = static_cast<uint8_t>((value >> 8) & 0xFFU);
//     tx[2] = static_cast<uint8_t>(value & 0xFFU);

//     status = XIicPs_MasterSendPolled(&g_iic, tx, 3, addr);
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

// static int i2c_read_reg16_be(uint16_t addr, uint8_t reg, uint16_t *value)
// {
//     uint8_t rx[2];
//     int status = i2c_read_bytes(addr, reg, rx, 2);

//     if (status != XST_SUCCESS)
//         return status;

//     *value = be16_to_u16(rx[0], rx[1]);
//     return XST_SUCCESS;
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

//     status = i2c_write_reg8(BNO055_I2C_ADDR, BNO055_OPR_MODE_REG, BNO055_OPR_IMUPLUS);
//     if (status != XST_SUCCESS)
//         return status;

//     vTaskDelay(pdMS_TO_TICKS(30));
//     return XST_SUCCESS;
// }

// static int ina260_init(void)
// {
//     uint16_t manuf = 0;
//     uint16_t die_id = 0;
//     int status;

//     status = i2c_read_reg16_be(INA260_I2C_ADDR, INA260_REG_MANUF_ID, &manuf);
//     if (status != XST_SUCCESS)
//         return status;

//     status = i2c_read_reg16_be(INA260_I2C_ADDR, INA260_REG_DIE_ID, &die_id);
//     if (status != XST_SUCCESS)
//         return status;

//     if ((manuf != INA260_MANUF_ID_VAL) || (die_id != INA260_DIE_ID_VAL))
//         return XST_FAILURE;

//     /* Optional explicit config write: continuous current + bus voltage mode. */
//     (void)i2c_write_reg16_be(INA260_I2C_ADDR, INA260_REG_CONFIG, 0x6127U);
//     vTaskDelay(pdMS_TO_TICKS(2));

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

// static const char *nav_mode_name(uint8_t mode)
// {
//     switch (static_cast<NavMode>(mode)) {
//     case NAV_WAIT_FOR_IMU:
//         return "WAIT";
//     case NAV_DRIVE_LEG:
//         return "DRIVE";
//     case NAV_TURN_RIGHT:
//         return "TURN";
//     case NAV_BACKTRACK:
//         return "BACKTRACK";
//     case NAV_RETRY_DELAY:
//         return "RETRY";
//     case NAV_FINISHED:
//         return "FINISHED";
//     default:
//         return "UNKNOWN";
//     }
// }

// static void print_fixed_2(const char *name, int32_t whole, int32_t frac, int negative)
// {
//     xil_printf("%s=%s%d.%02d", name, negative ? "-" : "", static_cast<int>(whole), static_cast<int>(frac));
// }

// static void print_bno_accel(const char *name, int16_t raw)
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

// static void print_millivolts(const char *name, int32_t mv)
// {
//     int32_t mag = mv;
//     int negative = 0;

//     if (mag < 0) {
//         negative = 1;
//         mag = -mag;
//     }

//     print_fixed_2(name, mag / 1000, (mag % 1000) / 10, negative);
// }

// static void print_milliunits(const char *name, int32_t value_ma_or_mw)
// {
//     int32_t mag = value_ma_or_mw;
//     int negative = 0;

//     if (mag < 0) {
//         negative = 1;
//         mag = -mag;
//     }

//     print_fixed_2(name, mag / 1000, (mag % 1000) / 10, negative);
// }

// static void print_heading_deg(int32_t cdeg)
// {
//     int32_t mag = cdeg;
//     int negative = 0;

//     if (mag < 0) {
//         negative = 1;
//         mag = -mag;
//     }

//     print_fixed_2("Heading", mag / 100, mag % 100, negative);
// }

// static void print_telem(const SharedState &snap)
// {
//     xil_printf("\r\n[telemetry] video=%d bno=%d ina=%d sonar=%d ",
//                snap.video_ok ? 1 : 0,
//                snap.bno_ok ? 1 : 0,
//                snap.ina_ok ? 1 : 0,
//                snap.sonar_ok ? 1 : 0);

//     if (snap.bno_ok) {
//         print_bno_accel("LAX", snap.lax);
//         xil_printf(" ");
//         print_bno_accel("LAY", snap.lay);
//         xil_printf(" ");
//         print_bno_accel("LAZ", snap.laz);
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

//     if (snap.ina_ok) {
//         print_millivolts("VBUS", snap.ina_bus_mv);
//         xil_printf(" V ");
//         print_milliunits("I", snap.ina_current_ma);
//         xil_printf(" A ");
//         print_milliunits("P", snap.ina_power_mw);
//         xil_printf(" W | ");
//     } else {
//         xil_printf("INA260 unavailable | ");
//     }

//     if (snap.sonar_ok)
//         xil_printf("Range=%d in | ", snap.sonar_inches);
//     else
//         xil_printf("Sonar unavailable | ");

//     xil_printf("Mode=%s Leg=%d/%d Seg=%d mm ",
//                nav_mode_name(snap.nav_mode),
//                static_cast<int>(snap.current_leg + 1),
//                static_cast<int>(NUM_SQUARE_LEGS),
//                static_cast<int>(snap.leg_progress_mm));
//     print_heading_deg(snap.heading_cdeg);
//     xil_printf(" deg\r\n");
// }

// static void vLoggerTask(void *)
// {
//     LogEvent ev{};

//     xil_printf("\r\n========================================\r\n");
//     xil_printf("FreeRTOS square-navigation test\r\n");
//     xil_printf("Video passthrough runs before scheduler. Runtime prints come only from logger.\r\n");
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
//             xil_printf("[sensor] I2C1 + BNO055 + INA260 init start\r\n");
//             break;

//         case LOG_SENSOR_BNO_OK:
//             xil_printf("[sensor] BNO055 init ok\r\n");
//             break;

//         case LOG_SENSOR_INA_OK:
//             xil_printf("[sensor] INA260 init ok\r\n");
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
//             xil_printf("[motor] square path control active\r\n");
//             break;

//         case LOG_OBSTACLE_BACKTRACK:
//             xil_printf("Obstacle detected going to last vertex\r\n");
//             break;

//         case LOG_FINISHED:
//             xil_printf("FINISHED!!!\r\n");
//             break;

//         case LOG_TELEMETRY:
//             print_telem(ev.snap);
//             break;

//         default:
//             break;
//         }
//     }
// }

// static void motor_apply(const MotorChannelCmd (&cmd)[4])
// {
//     uint32_t ctrl = 0U;

//     motor_wr(REG_DUTY0, cmd[0].duty);
//     motor_wr(REG_DUTY1, cmd[1].duty);
//     motor_wr(REG_DUTY2, cmd[2].duty);
//     motor_wr(REG_DUTY3, cmd[3].duty);

//     for (unsigned ch = 0; ch < 4; ch++) {
//         if (cmd[ch].enable)
//             ctrl |= MOTOR_EN_BIT(ch);
//         if (cmd[ch].dir)
//             ctrl |= MOTOR_DIR_BIT(ch);
//         if (cmd[ch].brake)
//             ctrl |= MOTOR_BRAKE_BIT(ch);
//     }

//     motor_wr(REG_CTRL, ctrl);
// }

// static void motor_all_stop(void)
// {
//     const MotorChannelCmd cmd[4] = {
//         {0U, false, false, false},
//         {0U, false, false, false},
//         {0U, false, false, false},
//         {0U, false, false, false},
//     };

//     motor_apply(cmd);
// }

// static void motor_all_brake(void)
// {
//     const MotorChannelCmd cmd[4] = {
//         {0U, true, false, true},
//         {0U, true, false, true},
//         {0U, true, false, true},
//         {0U, true, false, true},
//     };

//     motor_apply(cmd);
// }

// static void motor_drive_body(bool forward, uint16_t duty)
// {
//     const bool left_dir = forward ? (LEFT_FORWARD_DIR_TRUE != 0) : (LEFT_FORWARD_DIR_TRUE == 0);
//     const bool right_dir = forward ? (RIGHT_FORWARD_DIR_TRUE != 0) : (RIGHT_FORWARD_DIR_TRUE == 0);
//     const MotorChannelCmd cmd[4] = {
//         {duty, true, left_dir, false},
//         {duty, true, left_dir, false},
//         {duty, true, right_dir, false},
//         {duty, true, right_dir, false},
//     };

//     motor_apply(cmd);
// }

// static void motor_turn_right(uint16_t duty)
// {
//     const bool left_dir = (LEFT_FORWARD_DIR_TRUE != 0);
//     const bool right_dir = (RIGHT_FORWARD_DIR_TRUE == 0);
//     const MotorChannelCmd cmd[4] = {
//         {duty, true, left_dir, false},
//         {duty, true, left_dir, false},
//         {duty, true, right_dir, false},
//         {duty, true, right_dir, false},
//     };

//     motor_apply(cmd);
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
//         status = ina260_init();
//         if (status == XST_SUCCESS)
//             break;
//         update_ina(0, 0, 0, false);
//         vTaskDelay(pdMS_TO_TICKS(1000));
//     }
//     log_send_type(LOG_SENSOR_INA_OK);

//     for (;;) {
//         uint8_t lia_buf[6];
//         uint8_t gyr_buf[6];
//         uint16_t bus_raw = 0;
//         uint16_t current_raw = 0;
//         uint16_t power_raw = 0;

//         status = i2c_read_bytes(BNO055_I2C_ADDR, BNO055_LIA_DATA_X_LSB, lia_buf, 6);
//         if (status == XST_SUCCESS) {
//             status = i2c_read_bytes(BNO055_I2C_ADDR, BNO055_GYR_DATA_X_LSB, gyr_buf, 6);
//             if (status == XST_SUCCESS) {
//                 update_bno(
//                     le16_to_s16(lia_buf[0], lia_buf[1]),
//                     le16_to_s16(lia_buf[2], lia_buf[3]),
//                     le16_to_s16(lia_buf[4], lia_buf[5]),
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

//         status = i2c_read_reg16_be(INA260_I2C_ADDR, INA260_REG_BUS_VOLT, &bus_raw);
//         if (status == XST_SUCCESS)
//             status = i2c_read_reg16_be(INA260_I2C_ADDR, INA260_REG_CURRENT, &current_raw);
//         if (status == XST_SUCCESS)
//             status = i2c_read_reg16_be(INA260_I2C_ADDR, INA260_REG_POWER, &power_raw);

//         if (status == XST_SUCCESS) {
//             const int32_t bus_mv = (static_cast<int32_t>(bus_raw) * 125) / 100;
//             const int32_t current_ma = (static_cast<int32_t>(static_cast<int16_t>(current_raw)) * 125) / 100;
//             const int32_t power_mw = static_cast<int32_t>(power_raw) * 10;
//             update_ina(bus_mv, current_ma, power_mw, true);
//         } else {
//             update_ina(0, 0, 0, false);
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
//     const float dt_s = static_cast<float>(MOTOR_TASK_PERIOD_MS) / 1000.0f;

//     NavMode mode = NAV_WAIT_FOR_IMU;
//     NavMode retry_resume_mode = NAV_DRIVE_LEG;
//     TickType_t retry_deadline = 0;
//     uint8_t current_leg = 0U;
//     bool finished_logged = false;

//     float heading_deg = 0.0f;
//     float leg_progress_m = 0.0f;
//     float turn_progress_deg = 0.0f;
//     float backtrack_progress_m = 0.0f;
//     float backtrack_target_m = 0.0f;
//     float velocity_mps = 0.0f;

//     log_send_type(LOG_MOTOR_INIT_START);

//     motor_wr(REG_PERIOD, MOTOR_PWM_PERIOD);
//     motor_all_stop();
//     log_send_type(LOG_MOTOR_ACTIVE);

//     for (;;) {
//         const SharedState snap = snapshot_state();
//         const int16_t forward_acc_raw = IMU_FORWARD_SIGN * pick_axis_s16(snap.lax, snap.lay, snap.laz, IMU_FORWARD_AXIS);
//         const int16_t yaw_rate_raw = IMU_YAW_SIGN * pick_axis_s16(snap.gx, snap.gy, snap.gz, IMU_YAW_AXIS);

//         float forward_acc_mps2 = snap.bno_ok ? bno_lia_raw_to_mps2(forward_acc_raw) : 0.0f;
//         float yaw_rate_dps = snap.bno_ok ? bno_gyro_raw_to_dps(yaw_rate_raw) : 0.0f;

//         if (i_abs32(mps_to_mmps(forward_acc_mps2)) < ACCEL_DEADBAND_CMS2 * 10)
//             forward_acc_mps2 = 0.0f;
//         if (i_abs32(dps_to_cdegps(yaw_rate_dps)) < GYRO_DEADBAND_CDPS)
//             yaw_rate_dps = 0.0f;

//         switch (mode) {
//         case NAV_WAIT_FOR_IMU:
//             motor_all_brake();
//             velocity_mps = 0.0f;
//             leg_progress_m = 0.0f;
//             turn_progress_deg = 0.0f;
//             backtrack_progress_m = 0.0f;
//             backtrack_target_m = 0.0f;
//             if (snap.bno_ok && snap.sonar_ok) {
//                 current_leg = 0U;
//                 heading_deg = 0.0f;
//                 mode = NAV_DRIVE_LEG;
//             }
//             break;

//         case NAV_DRIVE_LEG:
//             if (!snap.bno_ok) {
//                 motor_all_brake();
//                 velocity_mps = 0.0f;
//                 break;
//             }

//             if (snap.sonar_ok && (snap.sonar_inches > 0) && (snap.sonar_inches <= SONAR_STOP_INCHES)) {
//                 motor_all_brake();
//                 velocity_mps = 0.0f;
//                 backtrack_target_m = leg_progress_m;
//                 backtrack_progress_m = 0.0f;
//                 retry_resume_mode = NAV_DRIVE_LEG;
//                 mode = NAV_BACKTRACK;
//                 log_send_type(LOG_OBSTACLE_BACKTRACK);
//                 break;
//             }

//             motor_drive_body(true, MOTOR_DUTY_DRIVE);
//             velocity_mps += forward_acc_mps2 * dt_s;
//             velocity_mps = velocity_mps * (1.0f - (static_cast<float>(VELOCITY_BLEND_PCT) / 100.0f))
//                          + (static_cast<float>(FORWARD_CRUISE_MMPS) / 1000.0f) * (static_cast<float>(VELOCITY_BLEND_PCT) / 100.0f);
//             velocity_mps = f_clamp(velocity_mps, 0.0f, static_cast<float>(MAX_TRACKED_SPEED_MMPS) / 1000.0f);
//             leg_progress_m += velocity_mps * dt_s;
//             heading_deg += yaw_rate_dps * dt_s;

//             if (leg_progress_m * 1000.0f >= static_cast<float>(SQUARE_LEG_TARGET_MM)) {
//                 motor_all_brake();
//                 velocity_mps = 0.0f;

//                 if ((current_leg + 1U) >= NUM_SQUARE_LEGS) {
//                     if (TURN_AFTER_FINAL_LEG) {
//                         turn_progress_deg = 0.0f;
//                         mode = NAV_TURN_RIGHT;
//                     } else {
//                         mode = NAV_FINISHED;
//                     }
//                 } else {
//                     turn_progress_deg = 0.0f;
//                     mode = NAV_TURN_RIGHT;
//                 }
//             }
//             break;

//         case NAV_TURN_RIGHT:
//             if (!snap.bno_ok) {
//                 motor_all_brake();
//                 break;
//             }

//             if (snap.sonar_ok && (snap.sonar_inches > 0) && (snap.sonar_inches <= SONAR_STOP_INCHES)) {
//                 motor_all_brake();
//                 velocity_mps = 0.0f;
//                 backtrack_target_m = 0.0f;
//                 backtrack_progress_m = 0.0f;
//                 retry_resume_mode = NAV_TURN_RIGHT;
//                 retry_deadline = xTaskGetTickCount() + pdMS_TO_TICKS(500);
//                 mode = NAV_RETRY_DELAY;
//                 log_send_type(LOG_OBSTACLE_BACKTRACK);
//                 break;
//             }

//             motor_turn_right(MOTOR_DUTY_TURN);
//             turn_progress_deg += f_abs(yaw_rate_dps) * dt_s;
//             heading_deg -= f_abs(yaw_rate_dps) * dt_s;

//             if (turn_progress_deg >= static_cast<float>(RIGHT_TURN_TARGET_DEG)) {
//                 motor_all_brake();
//                 velocity_mps = 0.0f;
//                 turn_progress_deg = 0.0f;

//                 if (current_leg + 1U < NUM_SQUARE_LEGS) {
//                     current_leg++;
//                     leg_progress_m = 0.0f;
//                     mode = NAV_DRIVE_LEG;
//                 } else {
//                     mode = NAV_FINISHED;
//                 }
//             }
//             break;

//         case NAV_BACKTRACK:
//             if (backtrack_target_m <= 0.0f) {
//                 motor_all_brake();
//                 retry_deadline = xTaskGetTickCount() + pdMS_TO_TICKS(500);
//                 leg_progress_m = 0.0f;
//                 velocity_mps = 0.0f;
//                 mode = NAV_RETRY_DELAY;
//                 break;
//             }

//             motor_drive_body(false, MOTOR_DUTY_REVERSE);
//             velocity_mps += (-forward_acc_mps2) * dt_s;
//             velocity_mps = velocity_mps * (1.0f - (static_cast<float>(VELOCITY_BLEND_PCT) / 100.0f))
//                          + (static_cast<float>(REVERSE_CRUISE_MMPS) / 1000.0f) * (static_cast<float>(VELOCITY_BLEND_PCT) / 100.0f);
//             velocity_mps = f_clamp(velocity_mps, 0.0f, static_cast<float>(MAX_TRACKED_SPEED_MMPS) / 1000.0f);
//             backtrack_progress_m += velocity_mps * dt_s;

//             if (backtrack_progress_m >= backtrack_target_m) {
//                 motor_all_brake();
//                 retry_deadline = xTaskGetTickCount() + pdMS_TO_TICKS(500);
//                 leg_progress_m = 0.0f;
//                 backtrack_progress_m = 0.0f;
//                 backtrack_target_m = 0.0f;
//                 velocity_mps = 0.0f;
//                 mode = NAV_RETRY_DELAY;
//             }
//             break;

//         case NAV_RETRY_DELAY:
//             motor_all_brake();
//             velocity_mps = 0.0f;
//             if (xTaskGetTickCount() >= retry_deadline) {
//                 leg_progress_m = 0.0f;
//                 turn_progress_deg = 0.0f;
//                 mode = retry_resume_mode;
//             }
//             break;

//         case NAV_FINISHED:
//             motor_all_brake();
//             velocity_mps = 0.0f;
//             if (!finished_logged) {
//                 finished_logged = true;
//                 log_send_type(LOG_FINISHED);
//                 vTaskDelay(pdMS_TO_TICKS(FINISH_SETTLE_MS));
//             }
//             break;

//         default:
//             motor_all_brake();
//             velocity_mps = 0.0f;
//             mode = NAV_WAIT_FOR_IMU;
//             break;
//         }

//         update_nav(
//             mode,
//             current_leg,
//             static_cast<int32_t>(leg_progress_m * 1000.0f),
//             static_cast<int32_t>(heading_deg * 100.0f));

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

//     if (status == XST_SUCCESS) {
//         /* Source-side: disable Digilent video-related interrupts before FreeRTOS. */
//         const uintptr_t vdma = XPAR_XAXIVDMA_0_BASEADDR;
//         const uintptr_t vtc = 0x43C10000u;
//         uint32_t cr;

//         cr = Xil_In32(vdma + 0x00);
//         Xil_Out32(vdma + 0x00, cr & ~((1u << 12) | (1u << 13) | (1u << 14)));
//         cr = Xil_In32(vdma + 0x30);
//         Xil_Out32(vdma + 0x30, cr & ~((1u << 12) | (1u << 13) | (1u << 14)));
//         Xil_Out32(vdma + 0x04, 0x00007000);
//         Xil_Out32(vdma + 0x34, 0x00007000);

//         Xil_Out32(vtc + 0x0C, 0u);
//         Xil_Out32(vtc + 0x04, 0xFFFFFFFFu);

// #define GIC_ICDICER(id) (0xF8F01180u + 4u * ((id) / 32u))
// #define GIC_DISABLE_IRQ(id) \
//         Xil_Out32(GIC_ICDICER(id), 1u << ((id) % 32u))

//         GIC_DISABLE_IRQ(61);
//         GIC_DISABLE_IRQ(XPAR_FABRIC_AXI_VDMA_0_MM2S_INTROUT_INTR);
//         GIC_DISABLE_IRQ(XPAR_FABRIC_AXI_VDMA_0_S2MM_INTROUT_INTR);
//         GIC_DISABLE_IRQ(XPAR_PS7_GPIO_0_INTR);
//         GIC_DISABLE_IRQ(XPAR_PS7_I2C_0_INTR);

//         xil_printf("[video] VTG + VDMA + PS I2C0 + PS GPIO IRQs disabled\r\n");
//     }

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

#include <stdint.h>
#include <stdbool.h>

extern "C" {
#include "../assert_filename_fix.h"
#include "xparameters.h"
#include "xstatus.h"
#include "xil_printf.h"
#include "xil_types.h"
#include "xil_io.h"
#include "sleep.h"
#include "xiicps.h"
#include "xuartlite.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
}

#include "../xparams_compat.h"
#include "../platform/platform.h"
#include "../ov5640/OV5640.h"
#include "../ov5640/ScuGicInterruptController.h"
#include "../ov5640/PS_GPIO.h"
#include "../ov5640/AXI_VDMA.h"
#include "../ov5640/PS_IIC.h"
#include "../hdmi/VideoOutput.h"
#include "../MIPI_D_PHY_RX_v1_0/src/MIPI_D_PHY_RX.h"
#include "../MIPI_CSI_2_RX_v1_0/src/MIPI_CSI_2_RX.h"

using namespace digilent;

namespace {

/* ============================================================
 * Camera / video pipeline defines
 * ============================================================
 */
#define IRPT_CTL_DEVID          XPAR_PS7_SCUGIC_0_DEVICE_ID
#define GPIO_DEVID              XPAR_PS7_GPIO_0_DEVICE_ID
#define GPIO_IRPT_ID            XPAR_PS7_GPIO_0_INTR
#define CAM_I2C_DEVID           XPAR_PS7_I2C_0_DEVICE_ID
#define CAM_I2C_IRPT_ID         XPAR_PS7_I2C_0_INTR
#define VDMA_DEVID              XPAR_AXIVDMA_0_DEVICE_ID
#define VDMA_MM2S_IRPT_ID       XPAR_FABRIC_AXI_VDMA_0_MM2S_INTROUT_INTR
#define VDMA_S2MM_IRPT_ID       XPAR_FABRIC_AXI_VDMA_0_S2MM_INTROUT_INTR
#define DDR_BASE_ADDR           XPAR_DDR_MEM_BASEADDR
#define MEM_BASE_ADDR           (DDR_BASE_ADDR + 0x0A000000U)
#define GAMMA_BASE_ADDR         XPAR_AXI_GAMMACORRECTION_0_BASEADDR

/* ============================================================
 * Motor control defines
 * Assumption: the 4-channel motor IP exposes contiguous duty
 * registers and shared control bits laid out as:
 *   enable bits: [3:0]
 *   direction bits: [7:4]
 *   brake bits: [11:8]
 * If your IP uses a different map, only this section needs to change.
 * ============================================================
 */
#ifndef XPAR_MOTOR_CTRL_4CH_0_BASEADDR
#error "XPAR_MOTOR_CTRL_4CH_0_BASEADDR is not defined"
#endif

#define MOTOR_BASE              XPAR_MOTOR_CTRL_4CH_0_BASEADDR
#define REG_CTRL                0x00U
#define REG_PERIOD              0x04U
#define REG_DUTY0               0x08U
#define REG_DUTY1               0x0CU
#define REG_DUTY2               0x10U
#define REG_DUTY3               0x14U

#define MOTOR_EN_BIT(ch)        (1u << (ch))
#define MOTOR_DIR_BIT(ch)       (1u << (4u + (ch)))
#define MOTOR_BRAKE_BIT(ch)     (1u << (8u + (ch)))

#define MOTOR_FL                0U
#define MOTOR_BL                1U
#define MOTOR_FR                2U
#define MOTOR_BR                3U

#define configUSE_TASK_FPU_SUPPORT 2

/* ============================================================
 * PS I2C1 shared sensor bus: BNO055 + INA260
 * ============================================================
 */
#ifndef XPAR_XIICPS_1_BASEADDR
#error "XPAR_XIICPS_1_BASEADDR is not defined"
#endif

#define SENSOR_I2C_SCLK_HZ      50000U

#define BNO055_I2C_ADDR         0x28U
#define BNO055_CHIP_ID_REG      0x00U
#define BNO055_PAGE_ID_REG      0x07U
#define BNO055_GYR_DATA_X_LSB   0x14U
#define BNO055_LIA_DATA_X_LSB   0x28U
#define BNO055_UNIT_SEL_REG     0x3BU
#define BNO055_OPR_MODE_REG     0x3DU
#define BNO055_PWR_MODE_REG     0x3EU
#define BNO055_CHIP_ID_VAL      0xA0U
#define BNO055_PWR_NORMAL       0x00U
#define BNO055_OPR_CONFIG       0x00U
#define BNO055_OPR_IMUPLUS      0x08U

#define INA260_I2C_ADDR         0x40U
#define INA260_REG_CONFIG       0x00U
#define INA260_REG_CURRENT      0x01U
#define INA260_REG_BUS_VOLT     0x02U
#define INA260_REG_POWER        0x03U
#define INA260_REG_MANUF_ID     0xFEU
#define INA260_REG_DIE_ID       0xFFU
#define INA260_MANUF_ID_VAL     0x5449U
#define INA260_DIE_ID_VAL       0x2270U

/* ============================================================
 * Sonar UART
 * ============================================================
 */
#ifndef XPAR_XUARTLITE_0_BASEADDR
#error "XPAR_XUARTLITE_0_BASEADDR is not defined"
#endif

#define SONAR_UART_BASEADDR     XPAR_XUARTLITE_0_BASEADDR

/* ============================================================
 * Navigation / tuning knobs
 * ============================================================
 */
#define MOTOR_PWM_PERIOD            5000U
#define MOTOR_DUTY_DRIVE            5000U   
#define MOTOR_DUTY_REVERSE          4500U
#define MOTOR_DUTY_TURN             4500U

#define SONAR_STOP_INCHES           18
#define SENSOR_TASK_PERIOD_MS       20U
#define MOTOR_TASK_PERIOD_MS        20U
#define TELEMETRY_PERIOD_MS         250U

#define SQUARE_LEG_TARGET_MM        500
#define RIGHT_TURN_TARGET_DEG       90
#define NUM_SQUARE_LEGS             4U
#define TURN_AFTER_FINAL_LEG        0

#define FORWARD_CRUISE_MMPS         220
#define REVERSE_CRUISE_MMPS         180
#define MAX_TRACKED_SPEED_MMPS      450
#define ACCEL_DEADBAND_CMS2         8
#define GYRO_DEADBAND_CDPS          150
#define VELOCITY_BLEND_PCT          12
#define FINISH_SETTLE_MS            500U

/* ------------------------------------------------------------
 * Wiring / mounting knobs you may need to flip on hardware
 * ------------------------------------------------------------
 * LEFT/RIGHT *_DIR_TRUE values define which control-bit value makes
 * each side move forward for your actual wiring.
 *
 * IMU_FORWARD_AXIS selects which BNO055 linear-acceleration axis is
 * robot-forward. IMU_YAW_AXIS selects which gyro axis is the robot yaw.
 * Change the sign macros if the mounting is reversed.
 */
#define LEFT_FORWARD_DIR_TRUE       0
#define RIGHT_FORWARD_DIR_TRUE      1

#define IMU_AXIS_X                  0
#define IMU_AXIS_Y                  1
#define IMU_AXIS_Z                  2
#define IMU_FORWARD_AXIS            IMU_AXIS_Y
#define IMU_FORWARD_SIGN            (+1)
#define IMU_YAW_AXIS                IMU_AXIS_Z
#define IMU_YAW_SIGN                (+1)

#define STACK_LOGGER_TASK           2048U
#define STACK_SENSOR_TASK           2048U
#define STACK_SONAR_TASK            1024U
#define STACK_MOTOR_TASK            2048U
#define STACK_TELEM_TASK            2048U

#define PRIO_SENSOR_TASK            (tskIDLE_PRIORITY + 3)
#define PRIO_MOTOR_TASK             (tskIDLE_PRIORITY + 2)
#define PRIO_LOGGER_TASK            (tskIDLE_PRIORITY + 2)
#define PRIO_SONAR_TASK             (tskIDLE_PRIORITY + 1)
#define PRIO_TELEM_TASK             (tskIDLE_PRIORITY + 1)

#define LOG_QUEUE_LENGTH            32U

enum NavMode : uint8_t {
    NAV_WAIT_FOR_IMU = 0,
    NAV_DRIVE_LEG,
    NAV_TURN_RIGHT,
    NAV_BACKTRACK,
    NAV_RETRY_DELAY,
    NAV_FINISHED
};

struct SharedState {
    bool video_ok;
    bool bno_ok;
    bool sonar_ok;
    bool ina_ok;

    int16_t lax;
    int16_t lay;
    int16_t laz;
    int16_t gx;
    int16_t gy;
    int16_t gz;

    int sonar_inches;

    int32_t ina_bus_mv;
    int32_t ina_current_ma;
    int32_t ina_power_mw;

    uint8_t nav_mode;
    uint8_t current_leg;
    int32_t leg_progress_mm;
    int32_t heading_cdeg;
};

enum LogType : uint8_t {
    LOG_SENSOR_INIT_START = 0,
    LOG_SENSOR_BNO_OK,
    LOG_SENSOR_INA_OK,
    LOG_SONAR_INIT_START,
    LOG_SONAR_UART_OK,
    LOG_MOTOR_INIT_START,
    LOG_MOTOR_ACTIVE,
    LOG_OBSTACLE_BACKTRACK,
    LOG_FINISHED,
    LOG_TELEMETRY
};

struct LogEvent {
    LogType type;
    SharedState snap;
};

struct MotorChannelCmd {
    uint16_t duty;
    bool enable;
    bool dir;
    bool brake;
};

static SharedState g_state{};
static XIicPs g_iic{};
static XUartLite g_uart{};
static QueueHandle_t g_log_queue = nullptr;

/* Forward declarations */
static void vLoggerTask(void *);
static void vSensorTask(void *);
static void vSonarTask(void *);
static void vMotorTask(void *);
static void vTelemetryTask(void *);

static inline void motor_wr(unsigned offset, unsigned value)
{
    Xil_Out32(MOTOR_BASE + offset, value);
}

static inline float f_abs(float x)
{
    return (x < 0.0f) ? -x : x;
}

static inline float f_clamp(float x, float lo, float hi)
{
    if (x < lo)
        return lo;
    if (x > hi)
        return hi;
    return x;
}

static inline int32_t i_abs32(int32_t x)
{
    return (x < 0) ? -x : x;
}

static inline int16_t le16_to_s16(uint8_t lsb, uint8_t msb)
{
    return static_cast<int16_t>((static_cast<uint16_t>(msb) << 8) | static_cast<uint16_t>(lsb));
}

static inline uint16_t be16_to_u16(uint8_t msb, uint8_t lsb)
{
    return static_cast<uint16_t>((static_cast<uint16_t>(msb) << 8) | static_cast<uint16_t>(lsb));
}

static inline int16_t pick_axis_s16(int16_t x, int16_t y, int16_t z, int axis)
{
    switch (axis) {
    case IMU_AXIS_X:
        return x;
    case IMU_AXIS_Y:
        return y;
    default:
        return z;
    }
}

static inline float bno_lia_raw_to_mps2(int16_t raw)
{
    return static_cast<float>(raw) / 100.0f;
}

static inline float bno_gyro_raw_to_dps(int16_t raw)
{
    return static_cast<float>(raw) / 16.0f;
}

static inline int32_t mps_to_mmps(float mps)
{
    return static_cast<int32_t>(mps * 1000.0f);
}

static inline int32_t dps_to_cdegps(float dps)
{
    return static_cast<int32_t>(dps * 100.0f);
}

static void update_bno(int16_t lax, int16_t lay, int16_t laz, int16_t gx, int16_t gy, int16_t gz, bool ok)
{
    taskENTER_CRITICAL();
    g_state.bno_ok = ok;
    g_state.lax = lax;
    g_state.lay = lay;
    g_state.laz = laz;
    g_state.gx = gx;
    g_state.gy = gy;
    g_state.gz = gz;
    taskEXIT_CRITICAL();
}

static void update_ina(int32_t bus_mv, int32_t current_ma, int32_t power_mw, bool ok)
{
    taskENTER_CRITICAL();
    g_state.ina_ok = ok;
    g_state.ina_bus_mv = bus_mv;
    g_state.ina_current_ma = current_ma;
    g_state.ina_power_mw = power_mw;
    taskEXIT_CRITICAL();
}

static void update_sonar(int sonar_inches, bool ok)
{
    taskENTER_CRITICAL();
    g_state.sonar_ok = ok;
    g_state.sonar_inches = sonar_inches;
    taskEXIT_CRITICAL();
}

static void update_video(bool ok)
{
    taskENTER_CRITICAL();
    g_state.video_ok = ok;
    taskEXIT_CRITICAL();
}

static void update_nav(NavMode mode, uint8_t current_leg, int32_t leg_progress_mm, int32_t heading_cdeg)
{
    taskENTER_CRITICAL();
    g_state.nav_mode = static_cast<uint8_t>(mode);
    g_state.current_leg = current_leg;
    g_state.leg_progress_mm = leg_progress_mm;
    g_state.heading_cdeg = heading_cdeg;
    taskEXIT_CRITICAL();
}

static SharedState snapshot_state(void)
{
    SharedState snap;

    taskENTER_CRITICAL();
    snap = g_state;
    taskEXIT_CRITICAL();

    return snap;
}

static void log_send_type(LogType type)
{
    if (g_log_queue == nullptr)
        return;

    LogEvent ev{};
    ev.type = type;
    (void)xQueueSend(g_log_queue, &ev, 0);
}

static void log_send_telem(const SharedState &snap)
{
    if (g_log_queue == nullptr)
        return;

    LogEvent ev{};
    ev.type = LOG_TELEMETRY;
    ev.snap = snap;
    (void)xQueueSend(g_log_queue, &ev, 0);
}

static int wait_bus_idle(XIicPs *iic)
{
    int timeout = 1000000;

    while (XIicPs_BusIsBusy(iic) && timeout > 0)
        timeout--;

    return (timeout > 0) ? XST_SUCCESS : XST_FAILURE;
}

static int sensor_i2c_init(void)
{
    XIicPs_Config *cfg;
    int status;

#ifdef SDT
    cfg = XIicPs_LookupConfig(XPAR_XIICPS_1_BASEADDR);
#else
    cfg = XIicPs_LookupConfig(XPAR_XIICPS_1_DEVICE_ID);
#endif
    if (cfg == nullptr)
        return XST_FAILURE;

    status = XIicPs_CfgInitialize(&g_iic, cfg, cfg->BaseAddress);
    if (status != XST_SUCCESS)
        return status;

    status = XIicPs_SelfTest(&g_iic);
    if (status != XST_SUCCESS)
        return status;

    status = XIicPs_SetSClk(&g_iic, SENSOR_I2C_SCLK_HZ);
    if (status != XST_SUCCESS)
        return status;

    status = XIicPs_SetOptions(&g_iic, XIICPS_7_BIT_ADDR_OPTION);
    if (status != XST_SUCCESS)
        return status;

    return XST_SUCCESS;
}

static int i2c_write_reg8(uint16_t addr, uint8_t reg, uint8_t value)
{
    uint8_t tx[2];
    int status;

    tx[0] = reg;
    tx[1] = value;

    status = XIicPs_MasterSendPolled(&g_iic, tx, 2, addr);
    if (status != XST_SUCCESS)
        return status;

    return wait_bus_idle(&g_iic);
}

static int i2c_write_reg16_be(uint16_t addr, uint8_t reg, uint16_t value)
{
    uint8_t tx[3];
    int status;

    tx[0] = reg;
    tx[1] = static_cast<uint8_t>((value >> 8) & 0xFFU);
    tx[2] = static_cast<uint8_t>(value & 0xFFU);

    status = XIicPs_MasterSendPolled(&g_iic, tx, 3, addr);
    if (status != XST_SUCCESS)
        return status;

    return wait_bus_idle(&g_iic);
}

static int i2c_read_bytes(uint16_t addr, uint8_t reg, uint8_t *rx, int len)
{
    int status;

    status = XIicPs_SetOptions(&g_iic, XIICPS_7_BIT_ADDR_OPTION | XIICPS_REP_START_OPTION);
    if (status != XST_SUCCESS)
        return status;

    status = XIicPs_MasterSendPolled(&g_iic, &reg, 1, addr);
    if (status != XST_SUCCESS) {
        XIicPs_ClearOptions(&g_iic, XIICPS_REP_START_OPTION);
        return status;
    }

    status = XIicPs_MasterRecvPolled(&g_iic, rx, len, addr);
    XIicPs_ClearOptions(&g_iic, XIICPS_REP_START_OPTION);

    if (status != XST_SUCCESS)
        return status;

    return wait_bus_idle(&g_iic);
}

static int i2c_read_reg16_be(uint16_t addr, uint8_t reg, uint16_t *value)
{
    uint8_t rx[2];
    int status = i2c_read_bytes(addr, reg, rx, 2);

    if (status != XST_SUCCESS)
        return status;

    *value = be16_to_u16(rx[0], rx[1]);
    return XST_SUCCESS;
}

static int bno055_init(void)
{
    uint8_t chip_id = 0;
    int status;

    vTaskDelay(pdMS_TO_TICKS(800));

    for (int tries = 0; tries < 10; tries++) {
        status = i2c_read_bytes(BNO055_I2C_ADDR, BNO055_CHIP_ID_REG, &chip_id, 1);
        if ((status == XST_SUCCESS) && (chip_id == BNO055_CHIP_ID_VAL))
            break;
        vTaskDelay(pdMS_TO_TICKS(50));
    }

    if (chip_id != BNO055_CHIP_ID_VAL)
        return XST_FAILURE;

    status = i2c_write_reg8(BNO055_I2C_ADDR, BNO055_PAGE_ID_REG, 0x00U);
    if (status != XST_SUCCESS)
        return status;

    status = i2c_write_reg8(BNO055_I2C_ADDR, BNO055_OPR_MODE_REG, BNO055_OPR_CONFIG);
    if (status != XST_SUCCESS)
        return status;

    vTaskDelay(pdMS_TO_TICKS(25));

    status = i2c_write_reg8(BNO055_I2C_ADDR, BNO055_PWR_MODE_REG, BNO055_PWR_NORMAL);
    if (status != XST_SUCCESS)
        return status;

    vTaskDelay(pdMS_TO_TICKS(10));

    status = i2c_write_reg8(BNO055_I2C_ADDR, BNO055_UNIT_SEL_REG, 0x00U);
    if (status != XST_SUCCESS)
        return status;

    vTaskDelay(pdMS_TO_TICKS(10));

    status = i2c_write_reg8(BNO055_I2C_ADDR, BNO055_OPR_MODE_REG, BNO055_OPR_IMUPLUS);
    if (status != XST_SUCCESS)
        return status;

    vTaskDelay(pdMS_TO_TICKS(30));
    return XST_SUCCESS;
}

static int ina260_init(void)
{
    uint16_t manuf = 0;
    uint16_t die_id = 0;
    int status;

    status = i2c_read_reg16_be(INA260_I2C_ADDR, INA260_REG_MANUF_ID, &manuf);
    if (status != XST_SUCCESS)
        return status;

    status = i2c_read_reg16_be(INA260_I2C_ADDR, INA260_REG_DIE_ID, &die_id);
    if (status != XST_SUCCESS)
        return status;

    if ((manuf != INA260_MANUF_ID_VAL) || (die_id != INA260_DIE_ID_VAL))
        return XST_FAILURE;

    /* Optional explicit config write: continuous current + bus voltage mode. */
    (void)i2c_write_reg16_be(INA260_I2C_ADDR, INA260_REG_CONFIG, 0x6127U);
    vTaskDelay(pdMS_TO_TICKS(2));

    return XST_SUCCESS;
}

static int sonar_uart_init(void)
{
    int status;

#ifdef SDT
    status = XUartLite_Initialize(&g_uart, SONAR_UART_BASEADDR);
#else
    status = XUartLite_Initialize(&g_uart, XPAR_UARTLITE_0_DEVICE_ID);
#endif
    if (status != XST_SUCCESS)
        return status;

    status = XUartLite_SelfTest(&g_uart);
    if (status != XST_SUCCESS)
        return status;

    return XST_SUCCESS;
}

static int uartlite_get_byte(uint8_t *byte_out)
{
    const unsigned rx_count = XUartLite_Recv(&g_uart, byte_out, 1);
    return (rx_count == 1U) ? 1 : 0;
}

static int is_digit_u8(uint8_t c)
{
    return (c >= '0' && c <= '9');
}

static void pipeline_mode_change(
    AXI_VDMA<ScuGicInterruptController> &vdma_driver,
    OV5640 &cam,
    VideoOutput &vid,
    Resolution res,
    OV5640_cfg::mode_t mode)
{
    {
        vdma_driver.resetWrite();
        MIPI_CSI_2_RX_mWriteReg(
            XPAR_MIPI_CSI_2_RX_0_S_AXI_LITE_BASEADDR,
            CR_OFFSET,
            (CR_RESET_MASK & ~CR_ENABLE_MASK));
        MIPI_D_PHY_RX_mWriteReg(
            XPAR_MIPI_D_PHY_RX_0_S_AXI_LITE_BASEADDR,
            CR_OFFSET,
            (CR_RESET_MASK & ~CR_ENABLE_MASK));
        cam.reset();
    }

    {
        vdma_driver.configureWrite(
            timing[static_cast<int>(res)].h_active,
            timing[static_cast<int>(res)].v_active);
        Xil_Out32(GAMMA_BASE_ADDR, 3U);
        cam.init();
    }

    {
        vdma_driver.enableWrite();
        MIPI_CSI_2_RX_mWriteReg(
            XPAR_MIPI_CSI_2_RX_0_S_AXI_LITE_BASEADDR,
            CR_OFFSET,
            CR_ENABLE_MASK);
        MIPI_D_PHY_RX_mWriteReg(
            XPAR_MIPI_D_PHY_RX_0_S_AXI_LITE_BASEADDR,
            CR_OFFSET,
            CR_ENABLE_MASK);
        cam.set_mode(mode);
        cam.set_awb(OV5640_cfg::awb_t::AWB_ADVANCED);
    }

    {
        vid.reset();
        vdma_driver.resetRead();
    }

    {
        vid.configure(res);
        vdma_driver.configureRead(
            timing[static_cast<int>(res)].h_active,
            timing[static_cast<int>(res)].v_active);
    }

    {
        vid.enable();
        vdma_driver.enableRead();
    }
}

static int video_init_once(void)
{
    try {
        static ScuGicInterruptController irpt_ctl(IRPT_CTL_DEVID);
        static PS_GPIO<ScuGicInterruptController> gpio_driver(GPIO_DEVID, irpt_ctl, GPIO_IRPT_ID);
        static PS_IIC<ScuGicInterruptController> iic_driver(CAM_I2C_DEVID, irpt_ctl, CAM_I2C_IRPT_ID, 100000);
        static OV5640 cam(iic_driver, gpio_driver);
        static AXI_VDMA<ScuGicInterruptController> vdma_driver(
            VDMA_DEVID,
            MEM_BASE_ADDR,
            irpt_ctl,
            VDMA_MM2S_IRPT_ID,
            VDMA_S2MM_IRPT_ID);
        static VideoOutput vid(XPAR_VTC_0_DEVICE_ID, XPAR_VIDEO_DYNCLK_DEVICE_ID);

        pipeline_mode_change(
            vdma_driver,
            cam,
            vid,
            Resolution::R1920_1080_60_PP,
            OV5640_cfg::mode_t::MODE_1080P_1920_1080_30fps);

        return XST_SUCCESS;
    }
    catch (...) {
        return XST_FAILURE;
    }
}

static const char *nav_mode_name(uint8_t mode)
{
    switch (static_cast<NavMode>(mode)) {
    case NAV_WAIT_FOR_IMU:
        return "WAIT";
    case NAV_DRIVE_LEG:
        return "DRIVE";
    case NAV_TURN_RIGHT:
        return "TURN";
    case NAV_BACKTRACK:
        return "BACKTRACK";
    case NAV_RETRY_DELAY:
        return "RETRY";
    case NAV_FINISHED:
        return "FINISHED";
    default:
        return "UNKNOWN";
    }
}

static void print_fixed_2(const char *name, int32_t whole, int32_t frac, int negative)
{
    xil_printf("%s=%s%d.%02d", name, negative ? "-" : "", static_cast<int>(whole), static_cast<int>(frac));
}

static void print_bno_accel(const char *name, int16_t raw)
{
    int32_t mag = raw;
    int negative = 0;

    if (mag < 0) {
        negative = 1;
        mag = -mag;
    }

    print_fixed_2(name, mag / 100, mag % 100, negative);
}

static void print_gyro_dps(const char *name, int16_t raw)
{
    int32_t mag = raw;
    int negative = 0;
    int32_t whole;
    int32_t frac;

    if (mag < 0) {
        negative = 1;
        mag = -mag;
    }

    whole = mag / 16;
    frac = ((mag % 16) * 100) / 16;
    print_fixed_2(name, whole, frac, negative);
}

static void print_millivolts(const char *name, int32_t mv)
{
    int32_t mag = mv;
    int negative = 0;

    if (mag < 0) {
        negative = 1;
        mag = -mag;
    }

    print_fixed_2(name, mag / 1000, (mag % 1000) / 10, negative);
}

static void print_milliunits(const char *name, int32_t value_ma_or_mw)
{
    int32_t mag = value_ma_or_mw;
    int negative = 0;

    if (mag < 0) {
        negative = 1;
        mag = -mag;
    }

    print_fixed_2(name, mag / 1000, (mag % 1000) / 10, negative);
}

static void print_heading_deg(int32_t cdeg)
{
    int32_t mag = cdeg;
    int negative = 0;

    if (mag < 0) {
        negative = 1;
        mag = -mag;
    }

    print_fixed_2("Heading", mag / 100, mag % 100, negative);
}

static void print_telem(const SharedState &snap)
{
    xil_printf("\r\n[telemetry] video=%d bno=%d ina=%d sonar=%d ",
               snap.video_ok ? 1 : 0,
               snap.bno_ok ? 1 : 0,
               snap.ina_ok ? 1 : 0,
               snap.sonar_ok ? 1 : 0);

    if (snap.bno_ok) {
        print_bno_accel("LAX", snap.lax);
        xil_printf(" ");
        print_bno_accel("LAY", snap.lay);
        xil_printf(" ");
        print_bno_accel("LAZ", snap.laz);
        xil_printf(" m/s^2 | ");
        print_gyro_dps("GX", snap.gx);
        xil_printf(" ");
        print_gyro_dps("GY", snap.gy);
        xil_printf(" ");
        print_gyro_dps("GZ", snap.gz);
        xil_printf(" dps | ");
    } else {
        xil_printf("BNO055 unavailable | ");
    }

    // if (snap.ina_ok) {
    //     print_millivolts("VBUS", snap.ina_bus_mv);
    //     xil_printf(" V ");
    //     print_milliunits("I", snap.ina_current_ma);
    //     xil_printf(" A ");
    //     print_milliunits("P", snap.ina_power_mw);
    //     xil_printf(" W | ");
    // } else {
    //     xil_printf("INA260 unavailable | ");
    // }

    if (snap.sonar_ok)
        xil_printf("Range=%d in | ", snap.sonar_inches);
    else
        xil_printf("Sonar unavailable | ");

    xil_printf("Mode=%s Leg=%d/%d Seg=%d mm ",
               nav_mode_name(snap.nav_mode),
               static_cast<int>(snap.current_leg + 1),
               static_cast<int>(NUM_SQUARE_LEGS),
               static_cast<int>(snap.leg_progress_mm));
    print_heading_deg(snap.heading_cdeg);
    xil_printf(" deg\r\n");
}

static void vLoggerTask(void *)
{
    LogEvent ev{};

    xil_printf("\r\n========================================\r\n");
    xil_printf("FreeRTOS square-navigation test\r\n");
    xil_printf("Video passthrough runs before scheduler. Runtime prints come only from logger.\r\n");
    xil_printf("Motor base     = 0x%08x\r\n", static_cast<unsigned>(MOTOR_BASE));
    xil_printf("I2C1 base      = 0x%08x\r\n", static_cast<unsigned>(XPAR_XIICPS_1_BASEADDR));
    xil_printf("UARTLite base  = 0x%08x\r\n", static_cast<unsigned>(SONAR_UART_BASEADDR));
    xil_printf("DDR video base = 0x%08x\r\n", static_cast<unsigned>(MEM_BASE_ADDR));
    if (g_state.video_ok)
        xil_printf("[video] passthrough init ok\r\n");
    else
        xil_printf("[video] passthrough init failed\r\n");
    xil_printf("[rtos] starting scheduler\r\n");

    for (;;) {
        if (xQueueReceive(g_log_queue, &ev, portMAX_DELAY) != pdPASS)
            continue;

        switch (ev.type) {
        case LOG_SENSOR_INIT_START:
            xil_printf("[sensor] I2C1 + BNO055 init start (INA260 removed)\r\n");
            break;

        case LOG_SENSOR_BNO_OK:
            xil_printf("[sensor] BNO055 init ok\r\n");
            break;

        case LOG_SENSOR_INA_OK:
            xil_printf("[sensor] INA260 init ok\r\n");
            break;

        case LOG_SONAR_INIT_START:
            xil_printf("[sonar] init start\r\n");
            break;

        case LOG_SONAR_UART_OK:
            xil_printf("[sonar] UARTLite init ok\r\n");
            break;

        case LOG_MOTOR_INIT_START:
            xil_printf("[motor] init start\r\n");
            break;

        case LOG_MOTOR_ACTIVE:
            xil_printf("[motor] square path control active\r\n");
            break;

        case LOG_OBSTACLE_BACKTRACK:
            xil_printf("Obstacle detected going to last vertex\r\n");
            break;

        case LOG_FINISHED:
            xil_printf("FINISHED!!!\r\n");
            break;

        case LOG_TELEMETRY:
            print_telem(ev.snap);
            break;

        default:
            break;
        }
    }
}

static void motor_apply(const MotorChannelCmd (&cmd)[4])
{
    uint32_t ctrl = 0U;

    motor_wr(REG_DUTY0, cmd[0].duty);
    motor_wr(REG_DUTY1, cmd[1].duty);
    motor_wr(REG_DUTY2, cmd[2].duty);
    motor_wr(REG_DUTY3, cmd[3].duty);

    for (unsigned ch = 0; ch < 4; ch++) {
        if (cmd[ch].enable)
            ctrl |= MOTOR_EN_BIT(ch);
        if (cmd[ch].dir)
            ctrl |= MOTOR_DIR_BIT(ch);
        if (cmd[ch].brake)
            ctrl |= MOTOR_BRAKE_BIT(ch);
    }

    motor_wr(REG_CTRL, ctrl);
}

static void motor_all_stop(void)
{
    const MotorChannelCmd cmd[4] = {
        {0U, false, false, false},
        {0U, false, false, false},
        {0U, false, false, false},
        {0U, false, false, false},
    };

    motor_apply(cmd);
}

static void motor_all_brake(void)
{
    const MotorChannelCmd cmd[4] = {
        {0U, true, false, true},
        {0U, true, false, true},
        {0U, true, false, true},
        {0U, true, false, true},
    };

    motor_apply(cmd);
}

static void motor_drive_body(bool forward, uint16_t duty)
{
    const bool left_dir = forward ? (LEFT_FORWARD_DIR_TRUE != 0) : (LEFT_FORWARD_DIR_TRUE == 0);
    const bool right_dir = forward ? (RIGHT_FORWARD_DIR_TRUE != 0) : (RIGHT_FORWARD_DIR_TRUE == 0);
    const MotorChannelCmd cmd[4] = {
        {duty, true, left_dir, false},
        {duty, true, left_dir, false},
        {duty, true, right_dir, false},
        {duty, true, right_dir, false},
    };

    motor_apply(cmd);
}

static void motor_turn_right(uint16_t duty)
{
    const bool left_dir = (LEFT_FORWARD_DIR_TRUE != 0);
    const bool right_dir = (RIGHT_FORWARD_DIR_TRUE == 0);
    const MotorChannelCmd cmd[4] = {
        {duty, true, left_dir, false},
        {duty, true, left_dir, false},
        {duty, true, right_dir, false},
        {duty, true, right_dir, false},
    };

    motor_apply(cmd);
}

static void vSensorTask(void *)
{
    TickType_t last_wake = xTaskGetTickCount();
    const TickType_t period = pdMS_TO_TICKS(SENSOR_TASK_PERIOD_MS);
    int status;

    log_send_type(LOG_SENSOR_INIT_START);

    for (;;) {
        status = sensor_i2c_init();
        if (status == XST_SUCCESS)
            break;
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    /* BNO055 init: bounded retry. If it never comes up, sensor task proceeds
     * with bno_ok=false and the motor state machine will keep waiting in
     * NAV_WAIT_FOR_IMU (by design -- square-path nav needs the IMU). */
    bool bno_ready = false;
    for (int tries = 0; tries < 5; tries++) {
        if (bno055_init() == XST_SUCCESS) {
            bno_ready = true;
            break;
        }
        update_bno(0, 0, 0, 0, 0, 0, false);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    if (bno_ready)
        log_send_type(LOG_SENSOR_BNO_OK);

    /* INA260 physically removed from the PCB -- skip init and leave ina_ok=false.
     * Telemetry will show "INA260 unavailable" permanently. If the chip is ever
     * reinstalled, uncomment the block below and the read block further down. */
    update_ina(0, 0, 0, false);
    /*
    bool ina_ready = false;
    for (int tries = 0; tries < 3; tries++) {
        if (ina260_init() == XST_SUCCESS) {
            ina_ready = true;
            break;
        }
        update_ina(0, 0, 0, false);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    if (ina_ready)
        log_send_type(LOG_SENSOR_INA_OK);
    */

    for (;;) {
        uint8_t lia_buf[6];
        uint8_t gyr_buf[6];
        /* INA260 removed -- read buffers no longer needed
        uint16_t bus_raw = 0;
        uint16_t current_raw = 0;
        uint16_t power_raw = 0;
        */

        if (bno_ready) {
            status = i2c_read_bytes(BNO055_I2C_ADDR, BNO055_LIA_DATA_X_LSB, lia_buf, 6);
            if (status == XST_SUCCESS) {
                status = i2c_read_bytes(BNO055_I2C_ADDR, BNO055_GYR_DATA_X_LSB, gyr_buf, 6);
                if (status == XST_SUCCESS) {
                    update_bno(
                        le16_to_s16(lia_buf[0], lia_buf[1]),
                        le16_to_s16(lia_buf[2], lia_buf[3]),
                        le16_to_s16(lia_buf[4], lia_buf[5]),
                        le16_to_s16(gyr_buf[0], gyr_buf[1]),
                        le16_to_s16(gyr_buf[2], gyr_buf[3]),
                        le16_to_s16(gyr_buf[4], gyr_buf[5]),
                        true);
                } else {
                    update_bno(0, 0, 0, 0, 0, 0, false);
                }
            } else {
                update_bno(0, 0, 0, 0, 0, 0, false);
            }
        }

        /* INA260 physically removed -- read block disabled. Uncomment if reinstalled.
        status = i2c_read_reg16_be(INA260_I2C_ADDR, INA260_REG_BUS_VOLT, &bus_raw);
        if (status == XST_SUCCESS)
            status = i2c_read_reg16_be(INA260_I2C_ADDR, INA260_REG_CURRENT, &current_raw);
        if (status == XST_SUCCESS)
            status = i2c_read_reg16_be(INA260_I2C_ADDR, INA260_REG_POWER, &power_raw);

        if (status == XST_SUCCESS) {
            const int32_t bus_mv = (static_cast<int32_t>(bus_raw) * 125) / 100;
            const int32_t current_ma = (static_cast<int32_t>(static_cast<int16_t>(current_raw)) * 125) / 100;
            const int32_t power_mw = static_cast<int32_t>(power_raw) * 10;
            update_ina(bus_mv, current_ma, power_mw, true);
        } else {
            update_ina(0, 0, 0, false);
        }
        */

        vTaskDelayUntil(&last_wake, period);
    }
}

static void vSonarTask(void *)
{
    int status;
    uint8_t c = 0;
    uint8_t d1 = 0;
    uint8_t d2 = 0;
    uint8_t d3 = 0;
    int state = 0;

    log_send_type(LOG_SONAR_INIT_START);

    for (;;) {
        status = sonar_uart_init();
        if (status == XST_SUCCESS)
            break;
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    log_send_type(LOG_SONAR_UART_OK);

    for (;;) {
        if (!uartlite_get_byte(&c)) {
            vTaskDelay(pdMS_TO_TICKS(1));
            continue;
        }

        switch (state) {
        case 0:
            if (c == 'R')
                state = 1;
            break;

        case 1:
            if (is_digit_u8(c)) {
                d1 = c;
                state = 2;
            } else if (c == 'R') {
                state = 1;
            } else {
                state = 0;
            }
            break;

        case 2:
            if (is_digit_u8(c)) {
                d2 = c;
                state = 3;
            } else if (c == 'R') {
                state = 1;
            } else {
                state = 0;
            }
            break;

        case 3:
            if (is_digit_u8(c)) {
                d3 = c;
                state = 4;
            } else if (c == 'R') {
                state = 1;
            } else {
                state = 0;
            }
            break;

        case 4:
            if (c == '\r') {
                const int inches = (d1 - '0') * 100 + (d2 - '0') * 10 + (d3 - '0');
                update_sonar(inches, true);
            }
            state = 0;
            break;

        default:
            state = 0;
            break;
        }

        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

static void vMotorTask(void *)
{
    vPortTaskUsesFPU();
    TickType_t last_wake = xTaskGetTickCount();
    const TickType_t period = pdMS_TO_TICKS(MOTOR_TASK_PERIOD_MS);
    const float dt_s = static_cast<float>(MOTOR_TASK_PERIOD_MS) / 1000.0f;

    NavMode mode = NAV_WAIT_FOR_IMU;
    NavMode retry_resume_mode = NAV_DRIVE_LEG;
    TickType_t retry_deadline = 0;
    uint8_t current_leg = 0U;
    bool finished_logged = false;

    float heading_deg = 0.0f;
    float leg_progress_m = 0.0f;
    float turn_progress_deg = 0.0f;
    float backtrack_progress_m = 0.0f;
    float backtrack_target_m = 0.0f;
    float velocity_mps = 0.0f;

    log_send_type(LOG_MOTOR_INIT_START);

    motor_wr(REG_PERIOD, MOTOR_PWM_PERIOD);
    motor_all_stop();
    log_send_type(LOG_MOTOR_ACTIVE);

    for (;;) {
        const SharedState snap = snapshot_state();
        const int16_t forward_acc_raw = IMU_FORWARD_SIGN * pick_axis_s16(snap.lax, snap.lay, snap.laz, IMU_FORWARD_AXIS);
        const int16_t yaw_rate_raw = IMU_YAW_SIGN * pick_axis_s16(snap.gx, snap.gy, snap.gz, IMU_YAW_AXIS);

        float forward_acc_mps2 = snap.bno_ok ? bno_lia_raw_to_mps2(forward_acc_raw) : 0.0f;
        float yaw_rate_dps = snap.bno_ok ? bno_gyro_raw_to_dps(yaw_rate_raw) : 0.0f;

        if (i_abs32(mps_to_mmps(forward_acc_mps2)) < ACCEL_DEADBAND_CMS2 * 10)
            forward_acc_mps2 = 0.0f;
        if (i_abs32(dps_to_cdegps(yaw_rate_dps)) < GYRO_DEADBAND_CDPS)
            yaw_rate_dps = 0.0f;

        switch (mode) {
        case NAV_WAIT_FOR_IMU:
            motor_all_brake();
            velocity_mps = 0.0f;
            leg_progress_m = 0.0f;
            turn_progress_deg = 0.0f;
            backtrack_progress_m = 0.0f;
            backtrack_target_m = 0.0f;
            if (snap.bno_ok && snap.sonar_ok) {
                current_leg = 0U;
                heading_deg = 0.0f;
                mode = NAV_DRIVE_LEG;
            }
            break;

        case NAV_DRIVE_LEG:
            if (!snap.bno_ok) {
                motor_all_brake();
                velocity_mps = 0.0f;
                break;
            }

            if (snap.sonar_ok && (snap.sonar_inches > 0) && (snap.sonar_inches <= SONAR_STOP_INCHES)) {
                motor_all_brake();
                velocity_mps = 0.0f;
                backtrack_target_m = leg_progress_m;
                backtrack_progress_m = 0.0f;
                retry_resume_mode = NAV_DRIVE_LEG;
                mode = NAV_BACKTRACK;
                log_send_type(LOG_OBSTACLE_BACKTRACK);
                break;
            }

            motor_drive_body(true, MOTOR_DUTY_DRIVE);
            velocity_mps += forward_acc_mps2 * dt_s;
            velocity_mps = velocity_mps * (1.0f - (static_cast<float>(VELOCITY_BLEND_PCT) / 100.0f))
                         + (static_cast<float>(FORWARD_CRUISE_MMPS) / 1000.0f) * (static_cast<float>(VELOCITY_BLEND_PCT) / 100.0f);
            velocity_mps = f_clamp(velocity_mps, 0.0f, static_cast<float>(MAX_TRACKED_SPEED_MMPS) / 1000.0f);
            leg_progress_m += velocity_mps * dt_s;
            heading_deg += yaw_rate_dps * dt_s;

            if (leg_progress_m * 1000.0f >= static_cast<float>(SQUARE_LEG_TARGET_MM)) {
                motor_all_brake();
                velocity_mps = 0.0f;

                if ((current_leg + 1U) >= NUM_SQUARE_LEGS) {
                    if (TURN_AFTER_FINAL_LEG) {
                        turn_progress_deg = 0.0f;
                        mode = NAV_TURN_RIGHT;
                    } else {
                        mode = NAV_FINISHED;
                    }
                } else {
                    turn_progress_deg = 0.0f;
                    mode = NAV_TURN_RIGHT;
                }
            }
            break;

        case NAV_TURN_RIGHT:
            if (!snap.bno_ok) {
                motor_all_brake();
                break;
            }

            if (snap.sonar_ok && (snap.sonar_inches > 0) && (snap.sonar_inches <= SONAR_STOP_INCHES)) {
                motor_all_brake();
                velocity_mps = 0.0f;
                backtrack_target_m = 0.0f;
                backtrack_progress_m = 0.0f;
                retry_resume_mode = NAV_TURN_RIGHT;
                retry_deadline = xTaskGetTickCount() + pdMS_TO_TICKS(500);
                mode = NAV_RETRY_DELAY;
                log_send_type(LOG_OBSTACLE_BACKTRACK);
                break;
            }

            motor_turn_right(MOTOR_DUTY_TURN);
            turn_progress_deg += f_abs(yaw_rate_dps) * dt_s;
            heading_deg -= f_abs(yaw_rate_dps) * dt_s;

            if (turn_progress_deg >= static_cast<float>(RIGHT_TURN_TARGET_DEG)) {
                motor_all_brake();
                velocity_mps = 0.0f;
                turn_progress_deg = 0.0f;

                if (current_leg + 1U < NUM_SQUARE_LEGS) {
                    current_leg++;
                    leg_progress_m = 0.0f;
                    mode = NAV_DRIVE_LEG;
                } else {
                    mode = NAV_FINISHED;
                }
            }
            break;

        case NAV_BACKTRACK:
            if (backtrack_target_m <= 0.0f) {
                motor_all_brake();
                retry_deadline = xTaskGetTickCount() + pdMS_TO_TICKS(500);
                leg_progress_m = 0.0f;
                velocity_mps = 0.0f;
                mode = NAV_RETRY_DELAY;
                break;
            }

            motor_drive_body(false, MOTOR_DUTY_REVERSE);
            velocity_mps += (-forward_acc_mps2) * dt_s;
            velocity_mps = velocity_mps * (1.0f - (static_cast<float>(VELOCITY_BLEND_PCT) / 100.0f))
                         + (static_cast<float>(REVERSE_CRUISE_MMPS) / 1000.0f) * (static_cast<float>(VELOCITY_BLEND_PCT) / 100.0f);
            velocity_mps = f_clamp(velocity_mps, 0.0f, static_cast<float>(MAX_TRACKED_SPEED_MMPS) / 1000.0f);
            backtrack_progress_m += velocity_mps * dt_s;

            if (backtrack_progress_m >= backtrack_target_m) {
                motor_all_brake();
                retry_deadline = xTaskGetTickCount() + pdMS_TO_TICKS(500);
                leg_progress_m = 0.0f;
                backtrack_progress_m = 0.0f;
                backtrack_target_m = 0.0f;
                velocity_mps = 0.0f;
                mode = NAV_RETRY_DELAY;
            }
            break;

        case NAV_RETRY_DELAY:
            motor_all_brake();
            velocity_mps = 0.0f;
            if (xTaskGetTickCount() >= retry_deadline) {
                leg_progress_m = 0.0f;
                turn_progress_deg = 0.0f;
                mode = retry_resume_mode;
            }
            break;

        case NAV_FINISHED:
            motor_all_brake();
            velocity_mps = 0.0f;
            if (!finished_logged) {
                finished_logged = true;
                log_send_type(LOG_FINISHED);
                vTaskDelay(pdMS_TO_TICKS(FINISH_SETTLE_MS));
            }
            break;

        default:
            motor_all_brake();
            velocity_mps = 0.0f;
            mode = NAV_WAIT_FOR_IMU;
            break;
        }

        update_nav(
            mode,
            current_leg,
            static_cast<int32_t>(leg_progress_m * 1000.0f),
            static_cast<int32_t>(heading_deg * 100.0f));

        vTaskDelayUntil(&last_wake, period);
    }
}

static void vTelemetryTask(void *)
{
    TickType_t last_wake = xTaskGetTickCount();
    const TickType_t period = pdMS_TO_TICKS(TELEMETRY_PERIOD_MS);

    for (;;) {
        log_send_telem(snapshot_state());
        vTaskDelayUntil(&last_wake, period);
    }
}

} // namespace

extern "C" void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    (void)xTask;
    taskDISABLE_INTERRUPTS();
    xil_printf("\r\n[FATAL] stack overflow in task: %s\r\n",
               (pcTaskName != nullptr) ? pcTaskName : "unknown");
    for (;;) {
    }
}

extern "C" void vApplicationMallocFailedHook(void)
{
    taskDISABLE_INTERRUPTS();
    xil_printf("\r\n[FATAL] pvPortMalloc failed\r\n");
    for (;;) {
    }
}

int main()
{
    BaseType_t ok;
    int status;

    init_platform();

    status = video_init_once();
    update_video(status == XST_SUCCESS);

    if (status == XST_SUCCESS) {
        /* Source-side: disable Digilent video-related interrupts before FreeRTOS. */
        const uintptr_t vdma = XPAR_XAXIVDMA_0_BASEADDR;
        const uintptr_t vtc = 0x43C10000u;
        uint32_t cr;

        cr = Xil_In32(vdma + 0x00);
        Xil_Out32(vdma + 0x00, cr & ~((1u << 12) | (1u << 13) | (1u << 14)));
        cr = Xil_In32(vdma + 0x30);
        Xil_Out32(vdma + 0x30, cr & ~((1u << 12) | (1u << 13) | (1u << 14)));
        Xil_Out32(vdma + 0x04, 0x00007000);
        Xil_Out32(vdma + 0x34, 0x00007000);

        Xil_Out32(vtc + 0x0C, 0u);
        Xil_Out32(vtc + 0x04, 0xFFFFFFFFu);

#define GIC_ICDICER(id) (0xF8F01180u + 4u * ((id) / 32u))
#define GIC_DISABLE_IRQ(id) \
        Xil_Out32(GIC_ICDICER(id), 1u << ((id) % 32u))

        GIC_DISABLE_IRQ(61);
        GIC_DISABLE_IRQ(XPAR_FABRIC_AXI_VDMA_0_MM2S_INTROUT_INTR);
        GIC_DISABLE_IRQ(XPAR_FABRIC_AXI_VDMA_0_S2MM_INTROUT_INTR);
        GIC_DISABLE_IRQ(XPAR_PS7_GPIO_0_INTR);
        GIC_DISABLE_IRQ(XPAR_PS7_I2C_0_INTR);

        xil_printf("[video] VTG + VDMA + PS I2C0 + PS GPIO IRQs disabled\r\n");
    }

    g_log_queue = xQueueCreate(LOG_QUEUE_LENGTH, sizeof(LogEvent));
    if (g_log_queue == nullptr) {
        xil_printf("\r\n[FATAL] log queue create failed\r\n");
        for (;;) {
        }
    }

    ok = xTaskCreate(vLoggerTask, "logger", STACK_LOGGER_TASK, nullptr, PRIO_LOGGER_TASK, nullptr);
    if (ok != pdPASS) {
        xil_printf("\r\n[FATAL] logger task create failed\r\n");
        for (;;) {
        }
    }

    ok = xTaskCreate(vSensorTask, "sensor", STACK_SENSOR_TASK, nullptr, PRIO_SENSOR_TASK, nullptr);
    if (ok != pdPASS) {
        xil_printf("\r\n[FATAL] sensor task create failed\r\n");
        for (;;) {
        }
    }

    ok = xTaskCreate(vSonarTask, "sonar", STACK_SONAR_TASK, nullptr, PRIO_SONAR_TASK, nullptr);
    if (ok != pdPASS) {
        xil_printf("\r\n[FATAL] sonar task create failed\r\n");
        for (;;) {
        }
    }

    ok = xTaskCreate(vMotorTask, "motor", STACK_MOTOR_TASK, nullptr, PRIO_MOTOR_TASK, nullptr);
    if (ok != pdPASS) {
        xil_printf("\r\n[FATAL] motor task create failed\r\n");
        for (;;) {
        }
    }

    ok = xTaskCreate(vTelemetryTask, "telem", STACK_TELEM_TASK, nullptr, PRIO_TELEM_TASK, nullptr);
    if (ok != pdPASS) {
        xil_printf("\r\n[FATAL] telemetry task create failed\r\n");
        for (;;) {
        }
    }

    vTaskStartScheduler();

    xil_printf("\r\n[FATAL] scheduler returned\r\n");
    for (;;) {
    }
}

// #include <stdint.h>
// #include <stdbool.h>

// #ifdef __cplusplus
// #ifndef _Bool
// #define _Bool bool
// #endif
// extern "C" {
// #endif

// #include "xparameters.h"
// #include "xstatus.h"
// #include "sleep.h"
// #include "xil_printf.h"
// #include "xil_types.h"
// #include "xil_cache.h"
// #include "xil_io.h"

// #include "xiicps.h"
// #include "xuartlite.h"

// #include "xaudioformatter.h"
// #include "xi2stx.h"
// #include "xiic.h"
// #include "xgpio.h"

// #ifdef __cplusplus
// }
// #endif

// /*
//  * ============================================================
//  * Top-level test selection
//  * Set exactly ONE of these to 1.
//  * ============================================================
//  */
// #define TEST_MOTOR                0
// #define TEST_I2C_BNO055           0
// #define TEST_SONAR_UART           0
// #define TEST_AUDIO_A4_TONE        0
// #define TEST_AUDIO_SINGLE_NOTE    0
// #define TEST_AUDIO_C_SCALE        0
// #define TEST_AUDIO_C_SCALE_UPDOWN 1

// #if (TEST_MOTOR + TEST_I2C_BNO055 + TEST_SONAR_UART + \
//      TEST_AUDIO_A4_TONE + TEST_AUDIO_SINGLE_NOTE + \
//      TEST_AUDIO_C_SCALE + TEST_AUDIO_C_SCALE_UPDOWN) != 1
// #error "Set exactly one TEST_* macro to 1"
// #endif

// /* ============================================================
//  * MOTOR TEST
//  * ============================================================
//  */
// #if TEST_MOTOR

// #ifndef XPAR_MOTOR_CTRL_4CH_0_BASEADDR
// #error "XPAR_MOTOR_CTRL_4CH_0_BASEADDR is not defined in xparameters.h"
// #endif

// #define MOTOR_BASE XPAR_MOTOR_CTRL_4CH_0_BASEADDR

// #define REG_CTRL   0x00U
// #define REG_PERIOD 0x04U
// #define REG_DUTY0  0x08U

// #define M0_EN      (1u << 0)
// #define M0_DIR     (1u << 4)
// #define M0_BRAKE   (1u << 8)

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

// static void motor0_reverse(unsigned duty)
// {
//     motor_wr(REG_DUTY0, duty);
//     motor_wr(REG_CTRL, M0_EN | M0_DIR);
// }

// int main(void)
// {
//     const unsigned period = 5000U;
//     const unsigned duty10 = 500U;
//     const unsigned duty20 = 1000U;

//     xil_printf("motor functional test start\r\n");
//     xil_printf("motor base = 0x%08X\r\n", (unsigned)MOTOR_BASE);

//     motor_wr(REG_PERIOD, period);
//     motor0_stop();
//     usleep(300000);

//     while (1) {
//         xil_printf("forward 10%%\r\n");
//         motor0_forward(duty10);
//         usleep(300000);

//         xil_printf("brake\r\n");
//         motor0_brake();
//         usleep(200000);

//         xil_printf("reverse 10%%\r\n");
//         motor0_reverse(duty10);
//         usleep(300000);

//         xil_printf("brake\r\n");
//         motor0_brake();
//         usleep(200000);

//         xil_printf("forward 20%%\r\n");
//         motor0_forward(duty20);
//         usleep(300000);

//         xil_printf("stop\r\n");
//         motor0_stop();
//         usleep(500000);
//     }

//     return 0;
// }

// /* ============================================================
//  * I2C BNO055 TEST
//  * ============================================================
//  */
// #elif TEST_I2C_BNO055

// #if !defined(XPAR_XIICPS_1_BASEADDR)
// #error "XPAR_XIICPS_1_BASEADDR is not defined in xparameters.h"
// #endif

// #define IIC_SCLK_RATE           50000U
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

// static XIicPs Iic;

// static int wait_bus_idle(XIicPs *IicPtr)
// {
//     int timeout = 1000000;

//     while (XIicPs_BusIsBusy(IicPtr) && timeout > 0)
//         timeout--;

//     return (timeout > 0) ? XST_SUCCESS : XST_FAILURE;
// }

// static int i2c_write_reg(u8 reg, u8 value)
// {
//     u8 tx[2];
//     int status;

//     tx[0] = reg;
//     tx[1] = value;

//     status = XIicPs_MasterSendPolled(&Iic, tx, 2, BNO055_I2C_ADDR);
//     if (status != XST_SUCCESS)
//         return status;

//     return wait_bus_idle(&Iic);
// }

// static int i2c_read_bytes(u8 reg, u8 *rx, int len)
// {
//     int status;

//     status = XIicPs_SetOptions(&Iic, XIICPS_7_BIT_ADDR_OPTION | XIICPS_REP_START_OPTION);
//     if (status != XST_SUCCESS)
//         return status;

//     status = XIicPs_MasterSendPolled(&Iic, &reg, 1, BNO055_I2C_ADDR);
//     if (status != XST_SUCCESS) {
//         XIicPs_ClearOptions(&Iic, XIICPS_REP_START_OPTION);
//         return status;
//     }

//     status = XIicPs_MasterRecvPolled(&Iic, rx, len, BNO055_I2C_ADDR);

//     XIicPs_ClearOptions(&Iic, XIICPS_REP_START_OPTION);

//     if (status != XST_SUCCESS)
//         return status;

//     return wait_bus_idle(&Iic);
// }

// static int i2c_read_reg(u8 reg, u8 *value)
// {
//     return i2c_read_bytes(reg, value, 1);
// }

// static int16_t le16_to_s16(u8 lsb, u8 msb)
// {
//     return (int16_t)(((uint16_t)msb << 8) | (uint16_t)lsb);
// }

// static void print_fixed_2(const char *name, int32_t whole, int32_t frac, int negative)
// {
//     xil_printf("%s=%s%d.%02d", name, negative ? "-" : "", (int)whole, (int)frac);
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
//     frac  = ((mag % 16) * 100) / 16;

//     print_fixed_2(name, whole, frac, negative);
// }

// static int bno055_init(void)
// {
//     u8 chip_id = 0;
//     int status;
//     int tries;

//     usleep(800000);

//     for (tries = 0; tries < 10; tries++) {
//         status = i2c_read_reg(BNO055_CHIP_ID_REG, &chip_id);
//         if ((status == XST_SUCCESS) && (chip_id == BNO055_CHIP_ID_VAL))
//             break;
//         usleep(50000);
//     }

//     if (chip_id != BNO055_CHIP_ID_VAL) {
//         xil_printf("BNO055 chip ID read failed. Got 0x%02X\r\n", chip_id);
//         return XST_FAILURE;
//     }

//     xil_printf("BNO055 chip ID OK: 0x%02X\r\n", chip_id);

//     status = i2c_write_reg(BNO055_PAGE_ID_REG, 0x00U);
//     if (status != XST_SUCCESS)
//         return status;

//     status = i2c_write_reg(BNO055_OPR_MODE_REG, BNO055_OPR_CONFIG);
//     if (status != XST_SUCCESS)
//         return status;

//     usleep(25000);

//     status = i2c_write_reg(BNO055_PWR_MODE_REG, BNO055_PWR_NORMAL);
//     if (status != XST_SUCCESS)
//         return status;

//     usleep(10000);

//     status = i2c_write_reg(BNO055_UNIT_SEL_REG, 0x00U);
//     if (status != XST_SUCCESS)
//         return status;

//     usleep(10000);

//     status = i2c_write_reg(BNO055_OPR_MODE_REG, BNO055_OPR_AMG);
//     if (status != XST_SUCCESS)
//         return status;

//     usleep(20000);

//     xil_printf("BNO055 set to AMG mode\r\n");
//     return XST_SUCCESS;
// }

// static int i2c_controller_init(void)
// {
//     XIicPs_Config *cfg;
//     int status;

// #ifdef SDT
//     cfg = XIicPs_LookupConfig(XPAR_XIICPS_1_BASEADDR);
// #else
//     cfg = XIicPs_LookupConfig(XPAR_XIICPS_1_DEVICE_ID);
// #endif

//     if (cfg == NULL)
//         return XST_FAILURE;

//     status = XIicPs_CfgInitialize(&Iic, cfg, cfg->BaseAddress);
//     if (status != XST_SUCCESS)
//         return status;

//     status = XIicPs_SelfTest(&Iic);
//     if (status != XST_SUCCESS)
//         return status;

//     status = XIicPs_SetSClk(&Iic, IIC_SCLK_RATE);
//     if (status != XST_SUCCESS)
//         return status;

//     status = XIicPs_SetOptions(&Iic, XIICPS_7_BIT_ADDR_OPTION);
//     if (status != XST_SUCCESS)
//         return status;

//     return XST_SUCCESS;
// }

// int main(void)
// {
//     int status;
//     u8 acc_buf[6];
//     u8 gyr_buf[6];
//     int16_t ax, ay, az;
//     int16_t gx, gy, gz;

//     xil_printf("BNO055 I2C test start\r\n");
//     xil_printf("I2C1 base = 0x%08X\r\n", (unsigned)XPAR_XIICPS_1_BASEADDR);

//     status = i2c_controller_init();
//     if (status != XST_SUCCESS) {
//         xil_printf("I2C controller init failed\r\n");
//         while (1) {
//         }
//     }

//     status = bno055_init();
//     if (status != XST_SUCCESS) {
//         xil_printf("BNO055 init failed\r\n");
//         while (1) {
//         }
//     }

//     while (1) {
//         status = i2c_read_bytes(BNO055_ACC_DATA_X_LSB, acc_buf, 6);
//         if (status != XST_SUCCESS) {
//             xil_printf("Accel read failed\r\n");
//             usleep(200000);
//             continue;
//         }

//         status = i2c_read_bytes(BNO055_GYR_DATA_X_LSB, gyr_buf, 6);
//         if (status != XST_SUCCESS) {
//             xil_printf("Gyro read failed\r\n");
//             usleep(200000);
//             continue;
//         }

//         ax = le16_to_s16(acc_buf[0], acc_buf[1]);
//         ay = le16_to_s16(acc_buf[2], acc_buf[3]);
//         az = le16_to_s16(acc_buf[4], acc_buf[5]);

//         gx = le16_to_s16(gyr_buf[0], gyr_buf[1]);
//         gy = le16_to_s16(gyr_buf[2], gyr_buf[3]);
//         gz = le16_to_s16(gyr_buf[4], gyr_buf[5]);

//         print_accel_mps2("AX", ax);
//         xil_printf(" ");
//         print_accel_mps2("AY", ay);
//         xil_printf(" ");
//         print_accel_mps2("AZ", az);
//         xil_printf(" m/s^2 | ");

//         print_gyro_dps("GX", gx);
//         xil_printf(" ");
//         print_gyro_dps("GY", gy);
//         xil_printf(" ");
//         print_gyro_dps("GZ", gz);
//         xil_printf(" dps\r\n");

//         usleep(100000);
//     }

//     return 0;
// }

// /* ============================================================
//  * SONAR UART TEST
//  * ============================================================
//  */
// #elif TEST_SONAR_UART

// #ifndef XPAR_XUARTLITE_0_BASEADDR
// #error "XPAR_XUARTLITE_0_BASEADDR is not defined in xparameters.h"
// #endif

// #define SONAR_UART_BASEADDR XPAR_XUARTLITE_0_BASEADDR

// static XUartLite UartLite;

// static int uartlite_init(void)
// {
//     int status;

// #ifdef SDT
//     status = XUartLite_Initialize(&UartLite, SONAR_UART_BASEADDR);
// #else
//     status = XUartLite_Initialize(&UartLite, XPAR_UARTLITE_0_DEVICE_ID);
// #endif
//     if (status != XST_SUCCESS)
//         return status;

//     status = XUartLite_SelfTest(&UartLite);
//     if (status != XST_SUCCESS)
//         return status;

//     return XST_SUCCESS;
// }

// static int uartlite_get_byte(u8 *byte_out)
// {
//     unsigned int rx_count;

//     rx_count = XUartLite_Recv(&UartLite, byte_out, 1);
//     return (rx_count == 1U) ? 1 : 0;
// }

// static int is_digit_u8(u8 c)
// {
//     return (c >= '0' && c <= '9');
// }

// int main(void)
// {
//     int status;
//     u8 c = 0;
//     u8 d1 = 0, d2 = 0, d3 = 0;
//     int state = 0;
//     int inches;
//     int cm_x100;

//     xil_printf("MAXSONAR UART test start\r\n");
//     xil_printf("UARTLite base = 0x%08X\r\n", (unsigned int)SONAR_UART_BASEADDR);

//     status = uartlite_init();
//     if (status != XST_SUCCESS) {
//         xil_printf("UARTLite init failed\r\n");
//         while (1) {
//         }
//     }

//     xil_printf("UARTLite init OK\r\n");
//     xil_printf("Waiting for sonar frames...\r\n");

//     usleep(500000);

//     while (1) {
//         if (!uartlite_get_byte(&c)) {
//             usleep(1000);
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
//                 inches = (d1 - '0') * 100 + (d2 - '0') * 10 + (d3 - '0');
//                 cm_x100 = inches * 254;

//                 xil_printf("Range: %d in, %d.%02d cm\r\n",
//                            inches,
//                            cm_x100 / 100,
//                            cm_x100 % 100);
//             }
//             state = 0;
//             break;

//         default:
//             state = 0;
//             break;
//         }
//     }

//     return 0;
// }

// /* ============================================================
//  * AUDIO TESTS
//  * ============================================================
//  */
// #else

// #ifndef XPAR_XAUDIO_FORMATTER_0_BASEADDR
// #error "XPAR_XAUDIO_FORMATTER_0_BASEADDR is not defined in xparameters.h"
// #endif

// #ifndef XPAR_XI2STX_0_BASEADDR
// #error "XPAR_XI2STX_0_BASEADDR is not defined in xparameters.h"
// #endif

// #ifndef XPAR_XIIC_0_BASEADDR
// #error "XPAR_XIIC_0_BASEADDR is not defined in xparameters.h"
// #endif

// #ifndef XPAR_XGPIO_0_BASEADDR
// #error "XPAR_XGPIO_0_BASEADDR is not defined in xparameters.h"
// #endif

// #define CODEC_I2C_ADDR           0x1AU

// #define SAMPLE_RATE_HZ           48000U
// #define AUDIO_MCLK_HZ            12288000U

// #define AMP                      10000
// #define NOTE_DURATION_MS         500U
// #define GAP_DURATION_US          250000U

// #define NUM_CHANNELS             2U
// #define BYTES_PER_SAMPLE         2U
// #define FRAME_BYTES              (NUM_CHANNELS * BYTES_PER_SAMPLE)

// #define NOTE_FRAMES              ((SAMPLE_RATE_HZ * NOTE_DURATION_MS) / 1000U)
// #define NOTE_BYTES               (NOTE_FRAMES * FRAME_BYTES)

// #define AF_PERIODS               8U
// #define AF_BYTES_PER_PERIOD      (NOTE_BYTES / AF_PERIODS)

// #if (NOTE_BYTES % AF_PERIODS) != 0
// #error "NOTE_BYTES must divide evenly into AF_PERIODS"
// #endif

// #if (AF_BYTES_PER_PERIOD < 64) || (AF_BYTES_PER_PERIOD > (50 * 1024))
// #error "AF_BYTES_PER_PERIOD must be between 64 and 51200 bytes"
// #endif

// #define A4_FREQ_CENTI_HZ           44000U
// #define A4_NAME                    "A4"
// #define SINGLE_NOTE_FREQ_CENTI_HZ  26163U
// #define SINGLE_NOTE_NAME           "C4"

// static const uint32_t ScaleFreqCentiHz[] = {
//     26163U, 29366U, 32963U, 34923U, 39200U, 44000U, 49388U, 52325U
// };

// static const char *ScaleNames[] = {
//     "C4", "D4", "E4", "F4", "G4", "A4", "B4", "C5"
// };

// #define SCALE_NOTES ((uint32_t)(sizeof(ScaleFreqCentiHz) / sizeof(ScaleFreqCentiHz[0])))

// static int16_t AudioBuf[NOTE_FRAMES * NUM_CHANNELS] __attribute__((aligned(64)));

// static XAudioFormatter AudioFormatter;
// static XI2s_Tx I2sTx;
// static XIic AudioIic;
// static XGpio MuteGpio;

// static int mute_gpio_init(void)
// {
//     int status;

// #ifdef SDT
//     status = XGpio_Initialize(&MuteGpio, XPAR_XGPIO_0_BASEADDR);
// #else
//     status = XGpio_Initialize(&MuteGpio, XPAR_XGPIO_0_DEVICE_ID);
// #endif
//     if (status != XST_SUCCESS)
//         return status;

//     XGpio_SetDataDirection(&MuteGpio, 1, 0x0U);
//     XGpio_DiscreteWrite(&MuteGpio, 1, 0x0U);

//     return XST_SUCCESS;
// }

// static void codec_unmute(void)
// {
//     XGpio_DiscreteWrite(&MuteGpio, 1, 0x1U);
// }

// static void codec_mute(void)
// {
//     XGpio_DiscreteWrite(&MuteGpio, 1, 0x0U);
// }

// static int audio_iic_init(void)
// {
//     int status;

// #ifdef SDT
//     status = XIic_Initialize(&AudioIic, XPAR_XIIC_0_BASEADDR);
// #else
//     status = XIic_Initialize(&AudioIic, XPAR_XIIC_0_DEVICE_ID);
// #endif
//     if (status != XST_SUCCESS)
//         return status;

//     status = XIic_Start(&AudioIic);
//     if (status != XST_SUCCESS)
//         return status;

//     return XST_SUCCESS;
// }

// static int codec_write_reg(uint8_t reg, uint16_t value9)
// {
//     uint8_t buf[2];
//     int sent;

//     buf[0] = (uint8_t)((reg << 1) | ((value9 >> 8) & 0x1U));
//     buf[1] = (uint8_t)(value9 & 0xFFU);

//     sent = XIic_Send(AudioIic.BaseAddress, CODEC_I2C_ADDR, buf, 2, XIIC_STOP);
//     return (sent == 2) ? XST_SUCCESS : XST_FAILURE;
// }

// static int codec_init_ssm2603(void)
// {
//     int status;

//     status = codec_write_reg(15U, 0x000U);
//     if (status != XST_SUCCESS)
//         return status;

//     usleep(10000);

//     status = codec_write_reg(6U, 0x030U);
//     if (status != XST_SUCCESS)
//         return status;

//     status = codec_write_reg(0U, 0x017U);
//     if (status != XST_SUCCESS)
//         return status;

//     status = codec_write_reg(1U, 0x017U);
//     if (status != XST_SUCCESS)
//         return status;

//     status = codec_write_reg(2U, 0x079U);
//     if (status != XST_SUCCESS)
//         return status;

//     status = codec_write_reg(3U, 0x079U);
//     if (status != XST_SUCCESS)
//         return status;

//     status = codec_write_reg(4U, 0x010U);
//     if (status != XST_SUCCESS)
//         return status;

//     status = codec_write_reg(5U, 0x000U);
//     if (status != XST_SUCCESS)
//         return status;

//     status = codec_write_reg(7U, 0x002U);
//     if (status != XST_SUCCESS)
//         return status;

//     status = codec_write_reg(8U, 0x000U);
//     if (status != XST_SUCCESS)
//         return status;

//     usleep(100000);

//     status = codec_write_reg(9U, 0x001U);
//     if (status != XST_SUCCESS)
//         return status;

//     status = codec_write_reg(6U, 0x020U);
//     if (status != XST_SUCCESS)
//         return status;

//     usleep(10000);
//     return XST_SUCCESS;
// }

// static int i2s_tx_init(void)
// {
//     int status;
//     XI2stx_Config *cfg;

// #ifdef SDT
//     cfg = XI2s_Tx_LookupConfig(XPAR_XI2STX_0_BASEADDR);
// #else
//     cfg = XI2s_Tx_LookupConfig(XPAR_XI2STX_0_DEVICE_ID);
// #endif
//     if (cfg == NULL)
//         return XST_FAILURE;

//     status = XI2s_Tx_CfgInitialize(&I2sTx, cfg, cfg->BaseAddress);
//     if (status != XST_SUCCESS)
//         return status;

//     status = XI2s_Tx_SelfTest(&I2sTx);
//     if (status != XST_SUCCESS)
//         return status;

//     status = XI2s_Tx_SetSclkOutDiv(&I2sTx, AUDIO_MCLK_HZ, SAMPLE_RATE_HZ);
//     if (status != XST_SUCCESS)
//         return status;

//     status = XI2s_Tx_SetChMux(&I2sTx, XI2S_TX_CHID0, XI2S_TX_CHMUX_AXIS_01);
//     if (status != XST_SUCCESS)
//         return status;

//     XI2s_Tx_Enable(&I2sTx, TRUE);
//     return XST_SUCCESS;
// }

// static int audio_formatter_init(void)
// {
//     int status;

// #ifdef SDT
//     status = XAudioFormatter_Initialize(&AudioFormatter, XPAR_XAUDIO_FORMATTER_0_BASEADDR);
// #else
//     status = XAudioFormatter_Initialize(&AudioFormatter, XPAR_XAUDIO_FORMATTER_0_DEVICE_ID);
// #endif
//     if (status != XST_SUCCESS)
//         return status;

//     AudioFormatter.ChannelId = XAudioFormatter_MM2S;
//     XAudioFormatterDMAReset(&AudioFormatter);
//     XAudioFormatterSetFsMultiplier(&AudioFormatter, AUDIO_MCLK_HZ, SAMPLE_RATE_HZ);
//     XAudioFormatter_InterruptClear(&AudioFormatter,
//                                    XAUD_STS_IOC_IRQ_MASK |
//                                    XAUD_STS_ERROR_IRQ_MASK |
//                                    XAUD_STS_TIMEOUT_IRQ_MASK);
//     return XST_SUCCESS;
// }

// static uint32_t phase_inc_from_centi_hz(uint32_t freq_centi_hz)
// {
//     uint64_t num = ((uint64_t)freq_centi_hz << 32);
//     uint64_t den = ((uint64_t)SAMPLE_RATE_HZ * 100ULL);
//     return (uint32_t)(num / den);
// }

// static void build_note_buffer(uint32_t freq_centi_hz)
// {
//     uint32_t i;
//     uint32_t phase = 0U;
//     uint32_t inc = phase_inc_from_centi_hz(freq_centi_hz);

//     for (i = 0U; i < NOTE_FRAMES; i++) {
//         int16_t s;
//         uint32_t idx;

//         phase += inc;
//         s = (phase & 0x80000000U) ? (int16_t)AMP : (int16_t)(-AMP);

//         idx = i * NUM_CHANNELS;
//         AudioBuf[idx + 0U] = s;
//         AudioBuf[idx + 1U] = s;
//     }
// }

// static void print_audio_debug(const char *tag)
// {
//     uint32_t af_sts;
//     uint32_t af_xfer;
//     uint32_t i2s_isr;

//     af_sts = XAudioFormatter_ReadReg(AudioFormatter.BaseAddress,
//                                      XAUD_FORMATTER_STS + XAUD_FORMATTER_MM2S_OFFSET);
//     af_xfer = XAudioFormatterGetDMATransferCount(&AudioFormatter);
//     i2s_isr = XI2s_Tx_ReadReg(I2sTx.Config.BaseAddress, 0x14U);

//     xil_printf("%s\r\n", tag);
//     xil_printf("AF_MM2S_STS   = 0x%08x\r\n", af_sts);
//     xil_printf("AF_MM2S_XFER  = %u / %u bytes\r\n", af_xfer, (uint32_t)NOTE_BYTES);
//     xil_printf("I2S_ISR       = 0x%08x\r\n", i2s_isr);
// }

// static void setup_note_transfer(uint32_t freq_centi_hz)
// {
//     XAudioFormatterHwParams hw_params;

//     build_note_buffer(freq_centi_hz);
//     Xil_DCacheFlushRange((UINTPTR)AudioBuf, NOTE_BYTES);

//     AudioFormatter.ChannelId = XAudioFormatter_MM2S;
//     XAudioFormatterDMAReset(&AudioFormatter);

//     hw_params.buf_addr = (UINTPTR)AudioBuf;
//     hw_params.active_ch = NUM_CHANNELS;
//     hw_params.bits_per_sample = BIT_DEPTH_16;
//     hw_params.periods = AF_PERIODS;
//     hw_params.bytes_per_period = AF_BYTES_PER_PERIOD;

//     XAudioFormatterSetHwParams(&AudioFormatter, &hw_params);
//     XAudioFormatter_InterruptClear(&AudioFormatter,
//                                    XAUD_STS_IOC_IRQ_MASK |
//                                    XAUD_STS_ERROR_IRQ_MASK |
//                                    XAUD_STS_TIMEOUT_IRQ_MASK);
// }

// static int play_note_timed(uint32_t freq_centi_hz, const char *name, uint32_t note_ms, uint32_t gap_us)
// {
//     xil_printf("Playing %s = %u centi-Hz\r\n", name, (unsigned)freq_centi_hz);

//     setup_note_transfer(freq_centi_hz);
//     print_audio_debug("Before start");

//     codec_unmute();
//     XAudioFormatterDMAStart(&AudioFormatter);

//     usleep(note_ms * 1000U);

//     XAudioFormatterDMAStop(&AudioFormatter);
//     print_audio_debug("After stop");

//     codec_mute();
//     usleep(gap_us);

//     return XST_SUCCESS;
// }

// static int run_audio_test(void)
// {
// #if TEST_AUDIO_A4_TONE
//     xil_printf("Selected test: A4 tone\r\n");
//     return play_note_timed(A4_FREQ_CENTI_HZ, A4_NAME, NOTE_DURATION_MS, GAP_DURATION_US);
// #elif TEST_AUDIO_SINGLE_NOTE
//     xil_printf("Selected test: single note\r\n");
//     return play_note_timed(SINGLE_NOTE_FREQ_CENTI_HZ, SINGLE_NOTE_NAME, NOTE_DURATION_MS, GAP_DURATION_US);
// #elif TEST_AUDIO_C_SCALE
//     uint32_t note;
//     xil_printf("Selected test: C major scale up\r\n");
//     for (note = 0U; note < SCALE_NOTES; note++) {
//         int status = play_note_timed(ScaleFreqCentiHz[note], ScaleNames[note], NOTE_DURATION_MS, GAP_DURATION_US);
//         if (status != XST_SUCCESS)
//             return status;
//     }
//     return XST_SUCCESS;
// #elif TEST_AUDIO_C_SCALE_UPDOWN
//     uint32_t note;
//     xil_printf("Selected test: C major scale up and down\r\n");
//     for (note = 0U; note < SCALE_NOTES; note++) {
//         int status = play_note_timed(ScaleFreqCentiHz[note], ScaleNames[note], NOTE_DURATION_MS, GAP_DURATION_US);
//         if (status != XST_SUCCESS)
//             return status;
//     }
//     for (note = SCALE_NOTES - 1U; note > 0U; note--) {
//         int status = play_note_timed(ScaleFreqCentiHz[note - 1U], ScaleNames[note - 1U], NOTE_DURATION_MS, GAP_DURATION_US);
//         if (status != XST_SUCCESS)
//             return status;
//     }
//     return XST_SUCCESS;
// #else
// #error "No audio test selected"
// #endif
// }

// int main(void)
// {
//     int status;

//     xil_printf("\r\n----------------------------------------\r\n");
//     xil_printf("Audio Formatter + I2S TX selectable audio test\r\n");
//     xil_printf("AF base   = 0x%08x\r\n", (uint32_t)XPAR_XAUDIO_FORMATTER_0_BASEADDR);
//     xil_printf("I2S base  = 0x%08x\r\n", (uint32_t)XPAR_XI2STX_0_BASEADDR);
//     xil_printf("IIC base  = 0x%08x\r\n", (uint32_t)XPAR_XIIC_0_BASEADDR);
//     xil_printf("GPIO base = 0x%08x\r\n", (uint32_t)XPAR_XGPIO_0_BASEADDR);
//     xil_printf("Buffer @  = 0x%08x\r\n", (uint32_t)(UINTPTR)AudioBuf);
//     xil_printf("Note bytes       = %u\r\n", (uint32_t)NOTE_BYTES);
//     xil_printf("Bytes per period = %u\r\n", (uint32_t)AF_BYTES_PER_PERIOD);
//     xil_printf("Periods          = %u\r\n", (uint32_t)AF_PERIODS);
//     xil_printf("Note time        = %u ms\r\n", (unsigned)NOTE_DURATION_MS);
//     xil_printf("Mute gap         = %u us\r\n", (unsigned)GAP_DURATION_US);

//     Xil_ICacheEnable();
//     Xil_DCacheEnable();

//     status = mute_gpio_init();
//     if (status != XST_SUCCESS) {
//         xil_printf("Mute GPIO init failed\r\n");
//         while (1) {
//         }
//     }
//     codec_mute();

//     status = audio_iic_init();
//     if (status != XST_SUCCESS) {
//         xil_printf("Audio IIC init failed\r\n");
//         while (1) {
//         }
//     }

//     status = codec_init_ssm2603();
//     if (status != XST_SUCCESS) {
//         xil_printf("Codec init failed\r\n");
//         while (1) {
//         }
//     }

//     status = i2s_tx_init();
//     if (status != XST_SUCCESS) {
//         xil_printf("I2S TX init failed\r\n");
//         while (1) {
//         }
//     }

//     status = audio_formatter_init();
//     if (status != XST_SUCCESS) {
//         xil_printf("Audio Formatter init failed\r\n");
//         while (1) {
//         }
//     }

//     status = run_audio_test();
//     if (status != XST_SUCCESS) {
//         codec_mute();
//         xil_printf("Audio test failed\r\n");
//         while (1) {
//         }
//     }

//     codec_mute();
//     xil_printf("Audio test done\r\n");
//     xil_printf("----------------------------------------\r\n");

//     while (1) {
//     }

//     return 0;
// }

// #endif

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
#ifndef _Bool
#define _Bool bool
#endif
extern "C" {
#endif

#include "xparameters.h"
#include "xstatus.h"
#include "sleep.h"
#include "xil_printf.h"
#include "xil_types.h"
#include "xil_cache.h"
#include "xil_io.h"

#include "xiicps.h"
#include "xuartlite.h"

#include "xaudioformatter.h"
#include "xi2stx.h"
#include "xiic.h"
#include "xgpio.h"

#ifdef __cplusplus
}
#endif

/*
 * ============================================================
 * Top-level test selection
 * Set exactly ONE of these to 1.
 * ============================================================
 */
#define TEST_MOTOR                0
#define TEST_I2C_BNO055           0
#define TEST_SONAR_UART           0
#define TEST_AUDIO_A4_TONE        0
#define TEST_AUDIO_SINGLE_NOTE    0
#define TEST_AUDIO_C_SCALE        0
#define TEST_AUDIO_C_SCALE_UPDOWN 0
#define TEST_AUDIO_TETRIS_THEME   1

#if (TEST_MOTOR + TEST_I2C_BNO055 + TEST_SONAR_UART + \
     TEST_AUDIO_A4_TONE + TEST_AUDIO_SINGLE_NOTE + \
     TEST_AUDIO_C_SCALE + TEST_AUDIO_C_SCALE_UPDOWN + \
     TEST_AUDIO_TETRIS_THEME) != 1
#error "Set exactly one TEST_* macro to 1"
#endif

/* ============================================================
 * MOTOR TEST
 * ============================================================
 */
#if TEST_MOTOR

#ifndef XPAR_MOTOR_CTRL_4CH_0_BASEADDR
#error "XPAR_MOTOR_CTRL_4CH_0_BASEADDR is not defined in xparameters.h"
#endif

#define MOTOR_BASE XPAR_MOTOR_CTRL_4CH_0_BASEADDR

#define REG_CTRL   0x00U
#define REG_PERIOD 0x04U
#define REG_DUTY0  0x08U

#define M0_EN      (1u << 0)
#define M0_DIR     (1u << 4)
#define M0_BRAKE   (1u << 8)

static inline void motor_wr(unsigned offset, unsigned value)
{
    Xil_Out32(MOTOR_BASE + offset, value);
}

static void motor0_stop(void)
{
    motor_wr(REG_DUTY0, 0U);
    motor_wr(REG_CTRL, 0U);
}

static void motor0_brake(void)
{
    motor_wr(REG_DUTY0, 0U);
    motor_wr(REG_CTRL, M0_EN | M0_BRAKE);
}

static void motor0_forward(unsigned duty)
{
    motor_wr(REG_DUTY0, duty);
    motor_wr(REG_CTRL, M0_EN);
}

static void motor0_reverse(unsigned duty)
{
    motor_wr(REG_DUTY0, duty);
    motor_wr(REG_CTRL, M0_EN | M0_DIR);
}

int main(void)
{
    const unsigned period = 5000U;
    const unsigned duty10 = 500U;
    const unsigned duty20 = 1000U;

    xil_printf("motor functional test start\r\n");
    xil_printf("motor base = 0x%08X\r\n", (unsigned)MOTOR_BASE);

    motor_wr(REG_PERIOD, period);
    motor0_stop();
    usleep(300000);

    while (1) {
        xil_printf("forward 10%%\r\n");
        motor0_forward(duty10);
        usleep(300000);

        xil_printf("brake\r\n");
        motor0_brake();
        usleep(200000);

        xil_printf("reverse 10%%\r\n");
        motor0_reverse(duty10);
        usleep(300000);

        xil_printf("brake\r\n");
        motor0_brake();
        usleep(200000);

        xil_printf("forward 20%%\r\n");
        motor0_forward(duty20);
        usleep(300000);

        xil_printf("stop\r\n");
        motor0_stop();
        usleep(500000);
    }

    return 0;
}

/* ============================================================
 * I2C BNO055 TEST
 * ============================================================
 */
#elif TEST_I2C_BNO055

#if !defined(XPAR_XIICPS_1_BASEADDR)
#error "XPAR_XIICPS_1_BASEADDR is not defined in xparameters.h"
#endif

#define IIC_SCLK_RATE           50000U
#define BNO055_I2C_ADDR         0x28U

#define BNO055_CHIP_ID_REG      0x00U
#define BNO055_PAGE_ID_REG      0x07U
#define BNO055_ACC_DATA_X_LSB   0x08U
#define BNO055_GYR_DATA_X_LSB   0x14U
#define BNO055_UNIT_SEL_REG     0x3BU
#define BNO055_OPR_MODE_REG     0x3DU
#define BNO055_PWR_MODE_REG     0x3EU

#define BNO055_CHIP_ID_VAL      0xA0U
#define BNO055_PWR_NORMAL       0x00U
#define BNO055_OPR_CONFIG       0x00U
#define BNO055_OPR_AMG          0x07U

static XIicPs Iic;

static int wait_bus_idle(XIicPs *IicPtr)
{
    int timeout = 1000000;

    while (XIicPs_BusIsBusy(IicPtr) && timeout > 0)
        timeout--;

    return (timeout > 0) ? XST_SUCCESS : XST_FAILURE;
}

static int i2c_write_reg(u8 reg, u8 value)
{
    u8 tx[2];
    int status;

    tx[0] = reg;
    tx[1] = value;

    status = XIicPs_MasterSendPolled(&Iic, tx, 2, BNO055_I2C_ADDR);
    if (status != XST_SUCCESS)
        return status;

    return wait_bus_idle(&Iic);
}

static int i2c_read_bytes(u8 reg, u8 *rx, int len)
{
    int status;

    status = XIicPs_SetOptions(&Iic, XIICPS_7_BIT_ADDR_OPTION | XIICPS_REP_START_OPTION);
    if (status != XST_SUCCESS)
        return status;

    status = XIicPs_MasterSendPolled(&Iic, &reg, 1, BNO055_I2C_ADDR);
    if (status != XST_SUCCESS) {
        XIicPs_ClearOptions(&Iic, XIICPS_REP_START_OPTION);
        return status;
    }

    status = XIicPs_MasterRecvPolled(&Iic, rx, len, BNO055_I2C_ADDR);

    XIicPs_ClearOptions(&Iic, XIICPS_REP_START_OPTION);

    if (status != XST_SUCCESS)
        return status;

    return wait_bus_idle(&Iic);
}

static int i2c_read_reg(u8 reg, u8 *value)
{
    return i2c_read_bytes(reg, value, 1);
}

static int16_t le16_to_s16(u8 lsb, u8 msb)
{
    return (int16_t)(((uint16_t)msb << 8) | (uint16_t)lsb);
}

static void print_fixed_2(const char *name, int32_t whole, int32_t frac, int negative)
{
    xil_printf("%s=%s%d.%02d", name, negative ? "-" : "", (int)whole, (int)frac);
}

static void print_accel_mps2(const char *name, int16_t raw)
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
    frac  = ((mag % 16) * 100) / 16;

    print_fixed_2(name, whole, frac, negative);
}

static int bno055_init(void)
{
    u8 chip_id = 0;
    int status;
    int tries;

    usleep(800000);

    for (tries = 0; tries < 10; tries++) {
        status = i2c_read_reg(BNO055_CHIP_ID_REG, &chip_id);
        if ((status == XST_SUCCESS) && (chip_id == BNO055_CHIP_ID_VAL))
            break;
        usleep(50000);
    }

    if (chip_id != BNO055_CHIP_ID_VAL) {
        xil_printf("BNO055 chip ID read failed. Got 0x%02X\r\n", chip_id);
        return XST_FAILURE;
    }

    xil_printf("BNO055 chip ID OK: 0x%02X\r\n", chip_id);

    status = i2c_write_reg(BNO055_PAGE_ID_REG, 0x00U);
    if (status != XST_SUCCESS)
        return status;

    status = i2c_write_reg(BNO055_OPR_MODE_REG, BNO055_OPR_CONFIG);
    if (status != XST_SUCCESS)
        return status;

    usleep(25000);

    status = i2c_write_reg(BNO055_PWR_MODE_REG, BNO055_PWR_NORMAL);
    if (status != XST_SUCCESS)
        return status;

    usleep(10000);

    status = i2c_write_reg(BNO055_UNIT_SEL_REG, 0x00U);
    if (status != XST_SUCCESS)
        return status;

    usleep(10000);

    status = i2c_write_reg(BNO055_OPR_MODE_REG, BNO055_OPR_AMG);
    if (status != XST_SUCCESS)
        return status;

    usleep(20000);

    xil_printf("BNO055 set to AMG mode\r\n");
    return XST_SUCCESS;
}

static int i2c_controller_init(void)
{
    XIicPs_Config *cfg;
    int status;

#ifdef SDT
    cfg = XIicPs_LookupConfig(XPAR_XIICPS_1_BASEADDR);
#else
    cfg = XIicPs_LookupConfig(XPAR_XIICPS_1_DEVICE_ID);
#endif

    if (cfg == NULL)
        return XST_FAILURE;

    status = XIicPs_CfgInitialize(&Iic, cfg, cfg->BaseAddress);
    if (status != XST_SUCCESS)
        return status;

    status = XIicPs_SelfTest(&Iic);
    if (status != XST_SUCCESS)
        return status;

    status = XIicPs_SetSClk(&Iic, IIC_SCLK_RATE);
    if (status != XST_SUCCESS)
        return status;

    status = XIicPs_SetOptions(&Iic, XIICPS_7_BIT_ADDR_OPTION);
    if (status != XST_SUCCESS)
        return status;

    return XST_SUCCESS;
}

int main(void)
{
    int status;
    u8 acc_buf[6];
    u8 gyr_buf[6];
    int16_t ax, ay, az;
    int16_t gx, gy, gz;

    xil_printf("BNO055 I2C test start\r\n");
    xil_printf("I2C1 base = 0x%08X\r\n", (unsigned)XPAR_XIICPS_1_BASEADDR);

    status = i2c_controller_init();
    if (status != XST_SUCCESS) {
        xil_printf("I2C controller init failed\r\n");
        while (1) {
        }
    }

    status = bno055_init();
    if (status != XST_SUCCESS) {
        xil_printf("BNO055 init failed\r\n");
        while (1) {
        }
    }

    while (1) {
        status = i2c_read_bytes(BNO055_ACC_DATA_X_LSB, acc_buf, 6);
        if (status != XST_SUCCESS) {
            xil_printf("Accel read failed\r\n");
            usleep(200000);
            continue;
        }

        status = i2c_read_bytes(BNO055_GYR_DATA_X_LSB, gyr_buf, 6);
        if (status != XST_SUCCESS) {
            xil_printf("Gyro read failed\r\n");
            usleep(200000);
            continue;
        }

        ax = le16_to_s16(acc_buf[0], acc_buf[1]);
        ay = le16_to_s16(acc_buf[2], acc_buf[3]);
        az = le16_to_s16(acc_buf[4], acc_buf[5]);

        gx = le16_to_s16(gyr_buf[0], gyr_buf[1]);
        gy = le16_to_s16(gyr_buf[2], gyr_buf[3]);
        gz = le16_to_s16(gyr_buf[4], gyr_buf[5]);

        print_accel_mps2("AX", ax);
        xil_printf(" ");
        print_accel_mps2("AY", ay);
        xil_printf(" ");
        print_accel_mps2("AZ", az);
        xil_printf(" m/s^2 | ");

        print_gyro_dps("GX", gx);
        xil_printf(" ");
        print_gyro_dps("GY", gy);
        xil_printf(" ");
        print_gyro_dps("GZ", gz);
        xil_printf(" dps\r\n");

        usleep(100000);
    }

    return 0;
}

/* ============================================================
 * SONAR UART TEST
 * ============================================================
 */
#elif TEST_SONAR_UART

#ifndef XPAR_XUARTLITE_0_BASEADDR
#error "XPAR_XUARTLITE_0_BASEADDR is not defined in xparameters.h"
#endif

#define SONAR_UART_BASEADDR XPAR_XUARTLITE_0_BASEADDR

static XUartLite UartLite;

static int uartlite_init(void)
{
    int status;

#ifdef SDT
    status = XUartLite_Initialize(&UartLite, SONAR_UART_BASEADDR);
#else
    status = XUartLite_Initialize(&UartLite, XPAR_UARTLITE_0_DEVICE_ID);
#endif
    if (status != XST_SUCCESS)
        return status;

    status = XUartLite_SelfTest(&UartLite);
    if (status != XST_SUCCESS)
        return status;

    return XST_SUCCESS;
}

static int uartlite_get_byte(u8 *byte_out)
{
    unsigned int rx_count;

    rx_count = XUartLite_Recv(&UartLite, byte_out, 1);
    return (rx_count == 1U) ? 1 : 0;
}

static int is_digit_u8(u8 c)
{
    return (c >= '0' && c <= '9');
}

int main(void)
{
    int status;
    u8 c = 0;
    u8 d1 = 0, d2 = 0, d3 = 0;
    int state = 0;
    int inches;
    int cm_x100;

    xil_printf("MAXSONAR UART test start\r\n");
    xil_printf("UARTLite base = 0x%08X\r\n", (unsigned int)SONAR_UART_BASEADDR);

    status = uartlite_init();
    if (status != XST_SUCCESS) {
        xil_printf("UARTLite init failed\r\n");
        while (1) {
        }
    }

    xil_printf("UARTLite init OK\r\n");
    xil_printf("Waiting for sonar frames...\r\n");

    usleep(500000);

    while (1) {
        if (!uartlite_get_byte(&c)) {
            usleep(1000);
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
                inches = (d1 - '0') * 100 + (d2 - '0') * 10 + (d3 - '0');
                cm_x100 = inches * 254;

                xil_printf("Range: %d in, %d.%02d cm\r\n",
                           inches,
                           cm_x100 / 100,
                           cm_x100 % 100);
            }
            state = 0;
            break;

        default:
            state = 0;
            break;
        }
    }

    return 0;
}

/* ============================================================
 * AUDIO TESTS
 * ============================================================
 */
#else

#ifndef XPAR_XAUDIO_FORMATTER_0_BASEADDR
#error "XPAR_XAUDIO_FORMATTER_0_BASEADDR is not defined in xparameters.h"
#endif

#ifndef XPAR_XI2STX_0_BASEADDR
#error "XPAR_XI2STX_0_BASEADDR is not defined in xparameters.h"
#endif

#ifndef XPAR_XIIC_0_BASEADDR
#error "XPAR_XIIC_0_BASEADDR is not defined in xparameters.h"
#endif

#ifndef XPAR_XGPIO_0_BASEADDR
#error "XPAR_XGPIO_0_BASEADDR is not defined in xparameters.h"
#endif

#define CODEC_I2C_ADDR           0x1AU

#define SAMPLE_RATE_HZ           48000U
#define AUDIO_MCLK_HZ            12288000U

#define AMP                      10000
#define NOTE_DURATION_MS         500U
#define GAP_DURATION_US          250000U
#define MAX_NOTE_DURATION_MS     500U
#define TETRIS_GAP_DURATION_US   5000U

#define NUM_CHANNELS             2U
#define BYTES_PER_SAMPLE         2U
#define FRAME_BYTES              (NUM_CHANNELS * BYTES_PER_SAMPLE)

#define AUDIO_BUF_FRAMES         ((SAMPLE_RATE_HZ * MAX_NOTE_DURATION_MS) / 1000U)
#define AUDIO_BUF_BYTES          (AUDIO_BUF_FRAMES * FRAME_BYTES)

#define AF_PERIODS               8U

#if (AUDIO_BUF_BYTES % AF_PERIODS) != 0
#error "AUDIO_BUF_BYTES must divide evenly into AF_PERIODS"
#endif

#if ((AUDIO_BUF_BYTES / AF_PERIODS) < 64) || ((AUDIO_BUF_BYTES / AF_PERIODS) > (50 * 1024))
#error "AUDIO_BUF_BYTES / AF_PERIODS must be between 64 and 51200 bytes"
#endif

#define A4_FREQ_CENTI_HZ           44000U
#define A4_NAME                    "A4"
#define SINGLE_NOTE_FREQ_CENTI_HZ  26163U
#define SINGLE_NOTE_NAME           "C4"

static const uint32_t ScaleFreqCentiHz[] = {
    26163U, 29366U, 32963U, 34923U, 39200U, 44000U, 49388U, 52325U
};

static const char *ScaleNames[] = {
    "C4", "D4", "E4", "F4", "G4", "A4", "B4", "C5"
};

#define SCALE_NOTES ((uint32_t)(sizeof(ScaleFreqCentiHz) / sizeof(ScaleFreqCentiHz[0])))

typedef struct {
    uint32_t freq_centi_hz;
    uint32_t duration_ms;
} Note;

static const Note TetrisTheme[] = {
    {65925U, 250U}, {49388U, 125U}, {52325U, 125U}, {58733U, 250U},
    {52325U, 125U}, {49388U, 125U}, {44000U, 250U}, {44000U, 125U},

    {52325U, 125U}, {65925U, 250U}, {58733U, 125U}, {52325U, 125U},
    {49388U, 375U},

    {52325U, 125U}, {58733U, 250U}, {65925U, 250U},
    {52325U, 250U}, {44000U, 250U}, {44000U, 250U}, {44000U, 500U},

    {58733U, 250U}, {69846U, 125U}, {88000U, 125U}, {78399U, 250U},
    {69846U, 125U}, {65925U, 125U}, {52325U, 250U}, {65925U, 125U},

    {58733U, 125U}, {52325U, 250U}, {49388U, 125U}, {52325U, 125U},
    {58733U, 375U},

    {65925U, 125U}, {52325U, 250U}, {44000U, 250U}, {44000U, 250U},
    {44000U, 125U}, {49388U, 125U}, {52325U, 125U}, {58733U, 250U},

    {65925U, 250U}, {52325U, 250U}, {65925U, 250U}, {78399U, 250U},

    {88000U, 125U}, {78399U, 125U}, {69846U, 125U}, {65925U, 125U},
    {69846U, 125U}, {78399U, 125U}, {88000U, 250U},

    {69846U, 125U}, {65925U, 125U}, {58733U, 125U}, {52325U, 125U},
    {58733U, 125U}, {65925U, 125U}, {69846U, 250U},

    {58733U, 125U}, {52325U, 125U}, {49388U, 125U}, {44000U, 125U},
    {49388U, 125U}, {52325U, 125U}, {58733U, 250U},

    {52325U, 125U}, {49388U, 125U}, {44000U, 125U}, {39199U, 125U},
    {44000U, 125U}, {49388U, 125U}, {52325U, 250U},

    {88000U, 125U}, {78399U, 125U}, {69846U, 125U}, {65925U, 125U},
    {69846U, 125U}, {78399U, 125U}, {88000U, 250U},

    {69846U, 125U}, {65925U, 125U}, {58733U, 125U}, {52325U, 125U},
    {58733U, 125U}, {65925U, 125U}, {69846U, 250U},

    {58733U, 125U}, {52325U, 125U}, {49388U, 125U}, {44000U, 125U},
    {49388U, 125U}, {52325U, 125U}, {58733U, 250U},

    {52325U, 125U}, {49388U, 125U}, {44000U, 125U}, {39199U, 125U},
    {44000U, 125U}, {49388U, 125U}, {52325U, 250U},

    {65925U, 250U}, {52325U, 250U}, {44000U, 500U}
};

#define TETRIS_THEME_LEN ((uint32_t)(sizeof(TetrisTheme) / sizeof(TetrisTheme[0])))

static int16_t AudioBuf[AUDIO_BUF_FRAMES * NUM_CHANNELS] __attribute__((aligned(64)));

static XAudioFormatter AudioFormatter;
static XI2s_Tx I2sTx;
static XIic AudioIic;
static XGpio MuteGpio;
static uint32_t ActiveNoteBytes = 0U;

static int mute_gpio_init(void)
{
    int status;

#ifdef SDT
    status = XGpio_Initialize(&MuteGpio, XPAR_XGPIO_0_BASEADDR);
#else
    status = XGpio_Initialize(&MuteGpio, XPAR_XGPIO_0_DEVICE_ID);
#endif
    if (status != XST_SUCCESS)
        return status;

    XGpio_SetDataDirection(&MuteGpio, 1, 0x0U);
    XGpio_DiscreteWrite(&MuteGpio, 1, 0x0U);

    return XST_SUCCESS;
}

static void codec_unmute(void)
{
    XGpio_DiscreteWrite(&MuteGpio, 1, 0x1U);
}

static void codec_mute(void)
{
    XGpio_DiscreteWrite(&MuteGpio, 1, 0x0U);
}

static int audio_iic_init(void)
{
    int status;

#ifdef SDT
    status = XIic_Initialize(&AudioIic, XPAR_XIIC_0_BASEADDR);
#else
    status = XIic_Initialize(&AudioIic, XPAR_XIIC_0_DEVICE_ID);
#endif
    if (status != XST_SUCCESS)
        return status;

    status = XIic_Start(&AudioIic);
    if (status != XST_SUCCESS)
        return status;

    return XST_SUCCESS;
}

static int codec_write_reg(uint8_t reg, uint16_t value9)
{
    uint8_t buf[2];
    int sent;

    buf[0] = (uint8_t)((reg << 1) | ((value9 >> 8) & 0x1U));
    buf[1] = (uint8_t)(value9 & 0xFFU);

    sent = XIic_Send(AudioIic.BaseAddress, CODEC_I2C_ADDR, buf, 2, XIIC_STOP);
    return (sent == 2) ? XST_SUCCESS : XST_FAILURE;
}

static int codec_init_ssm2603(void)
{
    int status;

    status = codec_write_reg(15U, 0x000U);
    if (status != XST_SUCCESS)
        return status;

    usleep(10000);

    status = codec_write_reg(6U, 0x030U);
    if (status != XST_SUCCESS)
        return status;

    status = codec_write_reg(0U, 0x017U);
    if (status != XST_SUCCESS)
        return status;

    status = codec_write_reg(1U, 0x017U);
    if (status != XST_SUCCESS)
        return status;

    status = codec_write_reg(2U, 0x079U);
    if (status != XST_SUCCESS)
        return status;

    status = codec_write_reg(3U, 0x079U);
    if (status != XST_SUCCESS)
        return status;

    status = codec_write_reg(4U, 0x010U);
    if (status != XST_SUCCESS)
        return status;

    status = codec_write_reg(5U, 0x000U);
    if (status != XST_SUCCESS)
        return status;

    status = codec_write_reg(7U, 0x002U);
    if (status != XST_SUCCESS)
        return status;

    status = codec_write_reg(8U, 0x000U);
    if (status != XST_SUCCESS)
        return status;

    usleep(100000);

    status = codec_write_reg(9U, 0x001U);
    if (status != XST_SUCCESS)
        return status;

    status = codec_write_reg(6U, 0x020U);
    if (status != XST_SUCCESS)
        return status;

    usleep(10000);
    return XST_SUCCESS;
}

static int i2s_tx_init(void)
{
    int status;
    XI2stx_Config *cfg;

#ifdef SDT
    cfg = XI2s_Tx_LookupConfig(XPAR_XI2STX_0_BASEADDR);
#else
    cfg = XI2s_Tx_LookupConfig(XPAR_XI2STX_0_DEVICE_ID);
#endif
    if (cfg == NULL)
        return XST_FAILURE;

    status = XI2s_Tx_CfgInitialize(&I2sTx, cfg, cfg->BaseAddress);
    if (status != XST_SUCCESS)
        return status;

    status = XI2s_Tx_SelfTest(&I2sTx);
    if (status != XST_SUCCESS)
        return status;

    status = XI2s_Tx_SetSclkOutDiv(&I2sTx, AUDIO_MCLK_HZ, SAMPLE_RATE_HZ);
    if (status != XST_SUCCESS)
        return status;

    status = XI2s_Tx_SetChMux(&I2sTx, XI2S_TX_CHID0, XI2S_TX_CHMUX_AXIS_01);
    if (status != XST_SUCCESS)
        return status;

    XI2s_Tx_Enable(&I2sTx, TRUE);
    return XST_SUCCESS;
}

static int audio_formatter_init(void)
{
    int status;

#ifdef SDT
    status = XAudioFormatter_Initialize(&AudioFormatter, XPAR_XAUDIO_FORMATTER_0_BASEADDR);
#else
    status = XAudioFormatter_Initialize(&AudioFormatter, XPAR_XAUDIO_FORMATTER_0_DEVICE_ID);
#endif
    if (status != XST_SUCCESS)
        return status;

    AudioFormatter.ChannelId = XAudioFormatter_MM2S;
    XAudioFormatterDMAReset(&AudioFormatter);
    XAudioFormatterSetFsMultiplier(&AudioFormatter, AUDIO_MCLK_HZ, SAMPLE_RATE_HZ);
    XAudioFormatter_InterruptClear(&AudioFormatter,
                                   XAUD_STS_IOC_IRQ_MASK |
                                   XAUD_STS_ERROR_IRQ_MASK |
                                   XAUD_STS_TIMEOUT_IRQ_MASK);
    return XST_SUCCESS;
}

static uint32_t phase_inc_from_centi_hz(uint32_t freq_centi_hz)
{
    uint64_t num = ((uint64_t)freq_centi_hz << 32);
    uint64_t den = ((uint64_t)SAMPLE_RATE_HZ * 100ULL);
    return (uint32_t)(num / den);
}

static uint32_t frames_from_ms(uint32_t duration_ms)
{
    return (SAMPLE_RATE_HZ * duration_ms) / 1000U;
}

static uint32_t bytes_from_frames(uint32_t frames)
{
    return frames * FRAME_BYTES;
}

static void build_note_buffer(uint32_t freq_centi_hz, uint32_t frames)
{
    uint32_t i;
    uint32_t phase = 0U;
    uint32_t inc = phase_inc_from_centi_hz(freq_centi_hz);

    for (i = 0U; i < frames; i++) {
        int16_t s;
        uint32_t idx;

        if (freq_centi_hz == 0U) {
            s = 0;
        } else {
            phase += inc;
            s = (phase & 0x80000000U) ? (int16_t)AMP : (int16_t)(-AMP);
        }

        idx = i * NUM_CHANNELS;
        AudioBuf[idx + 0U] = s;
        AudioBuf[idx + 1U] = s;
    }
}

static void print_audio_debug(const char *tag)
{
    uint32_t af_sts;
    uint32_t af_xfer;
    uint32_t i2s_isr;

    af_sts = XAudioFormatter_ReadReg(AudioFormatter.BaseAddress,
                                     XAUD_FORMATTER_STS + XAUD_FORMATTER_MM2S_OFFSET);
    af_xfer = XAudioFormatterGetDMATransferCount(&AudioFormatter);
    i2s_isr = XI2s_Tx_ReadReg(I2sTx.Config.BaseAddress, 0x14U);

    xil_printf("%s\r\n", tag);
    xil_printf("AF_MM2S_STS   = 0x%08x\r\n", af_sts);
    xil_printf("AF_MM2S_XFER  = %u / %u bytes\r\n", af_xfer, ActiveNoteBytes);
    xil_printf("I2S_ISR       = 0x%08x\r\n", i2s_isr);
}

static int setup_note_transfer(uint32_t freq_centi_hz, uint32_t note_ms, uint32_t *note_bytes_out)
{
    XAudioFormatterHwParams hw_params;
    uint32_t note_frames;
    uint32_t note_bytes;
    uint32_t bytes_per_period;

    note_frames = frames_from_ms(note_ms);
    note_bytes = bytes_from_frames(note_frames);

    if ((note_ms == 0U) || (note_ms > MAX_NOTE_DURATION_MS))
        return XST_FAILURE;

    if ((note_frames == 0U) || (note_frames > AUDIO_BUF_FRAMES))
        return XST_FAILURE;

    if ((note_bytes % AF_PERIODS) != 0U)
        return XST_FAILURE;

    bytes_per_period = note_bytes / AF_PERIODS;
    if ((bytes_per_period < 64U) || (bytes_per_period > (50U * 1024U)))
        return XST_FAILURE;

    build_note_buffer(freq_centi_hz, note_frames);
    Xil_DCacheFlushRange((UINTPTR)AudioBuf, note_bytes);

    AudioFormatter.ChannelId = XAudioFormatter_MM2S;
    XAudioFormatterDMAReset(&AudioFormatter);

    hw_params.buf_addr = (UINTPTR)AudioBuf;
    hw_params.active_ch = NUM_CHANNELS;
    hw_params.bits_per_sample = BIT_DEPTH_16;
    hw_params.periods = AF_PERIODS;
    hw_params.bytes_per_period = bytes_per_period;

    XAudioFormatterSetHwParams(&AudioFormatter, &hw_params);
    XAudioFormatter_InterruptClear(&AudioFormatter,
                                   XAUD_STS_IOC_IRQ_MASK |
                                   XAUD_STS_ERROR_IRQ_MASK |
                                   XAUD_STS_TIMEOUT_IRQ_MASK);

    ActiveNoteBytes = note_bytes;

    if (note_bytes_out != NULL)
        *note_bytes_out = note_bytes;

    return XST_SUCCESS;
}

static int play_note_timed(uint32_t freq_centi_hz, const char *name, uint32_t note_ms, uint32_t gap_us)
{
    int status;
    uint32_t note_bytes = 0U;

    xil_printf("Playing %s = %u centi-Hz for %u ms\r\n",
               name,
               (unsigned)freq_centi_hz,
               (unsigned)note_ms);

    status = setup_note_transfer(freq_centi_hz, note_ms, &note_bytes);
    if (status != XST_SUCCESS) {
        xil_printf("setup_note_transfer failed for %s\r\n", name);
        return status;
    }

    xil_printf("Active note bytes = %u\r\n", (unsigned)note_bytes);
    print_audio_debug("Before start");

    codec_unmute();
    XAudioFormatterDMAStart(&AudioFormatter);

    usleep(note_ms * 1000U);

    XAudioFormatterDMAStop(&AudioFormatter);
    print_audio_debug("After stop");

    codec_mute();
    usleep(gap_us);

    return XST_SUCCESS;
}

static int run_audio_test(void)
{
#if TEST_AUDIO_A4_TONE
    xil_printf("Selected test: A4 tone\r\n");
    return play_note_timed(A4_FREQ_CENTI_HZ, A4_NAME, NOTE_DURATION_MS, GAP_DURATION_US);
#elif TEST_AUDIO_SINGLE_NOTE
    xil_printf("Selected test: single note\r\n");
    return play_note_timed(SINGLE_NOTE_FREQ_CENTI_HZ, SINGLE_NOTE_NAME, NOTE_DURATION_MS, GAP_DURATION_US);
#elif TEST_AUDIO_C_SCALE
    uint32_t note;
    xil_printf("Selected test: C major scale up\r\n");
    for (note = 0U; note < SCALE_NOTES; note++) {
        int status = play_note_timed(ScaleFreqCentiHz[note], ScaleNames[note], NOTE_DURATION_MS, GAP_DURATION_US);
        if (status != XST_SUCCESS)
            return status;
    }
    return XST_SUCCESS;
#elif TEST_AUDIO_C_SCALE_UPDOWN
    uint32_t note;
    xil_printf("Selected test: C major scale up and down\r\n");
    for (note = 0U; note < SCALE_NOTES; note++) {
        int status = play_note_timed(ScaleFreqCentiHz[note], ScaleNames[note], NOTE_DURATION_MS, GAP_DURATION_US);
        if (status != XST_SUCCESS)
            return status;
    }
    for (note = SCALE_NOTES - 1U; note > 0U; note--) {
        int status = play_note_timed(ScaleFreqCentiHz[note - 1U], ScaleNames[note - 1U], NOTE_DURATION_MS, GAP_DURATION_US);
        if (status != XST_SUCCESS)
            return status;
    }
    return XST_SUCCESS;
#elif TEST_AUDIO_TETRIS_THEME
    uint32_t note;
    xil_printf("Selected test: Tetris theme\r\n");
    for (note = 0U; note < TETRIS_THEME_LEN; note++) {
        int status;

        xil_printf("Tetris note %u / %u\r\n",
                   (unsigned)(note + 1U),
                   (unsigned)TETRIS_THEME_LEN);

        status = play_note_timed(TetrisTheme[note].freq_centi_hz,
                                 "TETRIS",
                                 TetrisTheme[note].duration_ms,
                                 TETRIS_GAP_DURATION_US);
        if (status != XST_SUCCESS)
            return status;
    }
    return XST_SUCCESS;
#else
#error "No audio test selected"
#endif
}

int main(void)
{
    int status;

    xil_printf("\r\n----------------------------------------\r\n");
    xil_printf("Audio Formatter + I2S TX selectable audio test\r\n");
    xil_printf("AF base   = 0x%08x\r\n", (uint32_t)XPAR_XAUDIO_FORMATTER_0_BASEADDR);
    xil_printf("I2S base  = 0x%08x\r\n", (uint32_t)XPAR_XI2STX_0_BASEADDR);
    xil_printf("IIC base  = 0x%08x\r\n", (uint32_t)XPAR_XIIC_0_BASEADDR);
    xil_printf("GPIO base = 0x%08x\r\n", (uint32_t)XPAR_XGPIO_0_BASEADDR);
    xil_printf("Buffer @  = 0x%08x\r\n", (uint32_t)(UINTPTR)AudioBuf);
    xil_printf("Buffer bytes     = %u\r\n", (uint32_t)AUDIO_BUF_BYTES);
    xil_printf("Max note time    = %u ms\r\n", (unsigned)MAX_NOTE_DURATION_MS);
    xil_printf("Periods          = %u\r\n", (uint32_t)AF_PERIODS);
    xil_printf("Default note time= %u ms\r\n", (unsigned)NOTE_DURATION_MS);
    xil_printf("Default mute gap = %u us\r\n", (unsigned)GAP_DURATION_US);

    Xil_ICacheEnable();
    Xil_DCacheEnable();

    status = mute_gpio_init();
    if (status != XST_SUCCESS) {
        xil_printf("Mute GPIO init failed\r\n");
        while (1) {
        }
    }
    codec_mute();

    status = audio_iic_init();
    if (status != XST_SUCCESS) {
        xil_printf("Audio IIC init failed\r\n");
        while (1) {
        }
    }

    status = codec_init_ssm2603();
    if (status != XST_SUCCESS) {
        xil_printf("Codec init failed\r\n");
        while (1) {
        }
    }

    status = i2s_tx_init();
    if (status != XST_SUCCESS) {
        xil_printf("I2S TX init failed\r\n");
        while (1) {
        }
    }

    status = audio_formatter_init();
    if (status != XST_SUCCESS) {
        xil_printf("Audio Formatter init failed\r\n");
        while (1) {
        }
    }

    status = run_audio_test();
    if (status != XST_SUCCESS) {
        codec_mute();
        xil_printf("Audio test failed\r\n");
        while (1) {
        }
    }

    codec_mute();
    xil_printf("Audio test done\r\n");
    xil_printf("----------------------------------------\r\n");

    while (1) {
    }

    return 0;
}

#endif




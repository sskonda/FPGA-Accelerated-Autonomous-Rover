#include "xparameters.h"
#include "xstatus.h"
#include "sleep.h"
#include "xil_printf.h"
#include "xil_types.h"

/*
 * Set exactly one of these to 1.
 */
#define MOTOR_TEST 0
#define I2C_TEST   1
#define SONAR_TEST 0

#if ((MOTOR_TEST + I2C_TEST + SONAR_TEST) != 1)
#error "Set exactly one of MOTOR_TEST, I2C_TEST, SONAR_TEST to 1"
#endif

#if MOTOR_TEST

#include "xil_io.h"

#define MOTOR_BASE XPAR_MOTOR_CTRL_4CH_0_BASEADDR

#define REG_CTRL   0x00U
#define REG_PERIOD 0x04U
#define REG_DUTY0  0x08U

#define M0_EN      (1u << 0)
#define M0_DIR     (1u << 4)
#define M0_BRAKE   (1u << 8)

static inline void wr(unsigned offset, unsigned value)
{
    Xil_Out32(MOTOR_BASE + offset, value);
}

static void motor0_stop(void)
{
    wr(REG_DUTY0, 0);
    wr(REG_CTRL, 0);
}

static void motor0_brake(void)
{
    wr(REG_DUTY0, 0);
    wr(REG_CTRL, M0_EN | M0_BRAKE);
}

static void motor0_forward(unsigned duty)
{
    wr(REG_DUTY0, duty);
    wr(REG_CTRL, M0_EN);
}

static void motor0_reverse(unsigned duty)
{
    wr(REG_DUTY0, duty);
    wr(REG_CTRL, M0_EN | M0_DIR);
}

int main(void)
{
    const unsigned period = 5000U;
    const unsigned duty10 = 500U;
    const unsigned duty20 = 1000U;

    xil_printf("motor functional test start\r\n");

    wr(REG_PERIOD, period);
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

#elif I2C_TEST

#include "xiicps.h"
#include <stdint.h>

#ifndef SDT
#define IIC_DEVICE_ID   XPAR_XIICPS_1_DEVICE_ID
#else
#define IIC_BASE_ADDR   XPAR_XIICPS_1_BASEADDR
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

#ifndef SDT
    cfg = XIicPs_LookupConfig(IIC_DEVICE_ID);
#else
    cfg = XIicPs_LookupConfig(IIC_BASE_ADDR);
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

#elif SONAR_TEST

#include "xuartlite.h"

#define SONAR_UART_BASEADDR XPAR_XUARTLITE_0_BASEADDR

static XUartLite UartLite;

static int uartlite_init(void)
{
    int status;

    status = XUartLite_Initialize(&UartLite, SONAR_UART_BASEADDR);
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

#endif


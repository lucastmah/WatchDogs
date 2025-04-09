#include "hal/i2c.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <assert.h>

#define I2CDRV_LINUX_BUS "/dev/i2c-1"
#define I2C_TLA2024_DEVICE_ADDRESS 0x48

#define TLA2024_REG_CONFIGURATION 0x01
#define TLA2024_REG_DATA 0x00

#define TLA2024_CHANNEL_CONF_0 0x83C2
#define TLA2024_CHANNEL_CONF_1 0x83D2
#define TLA2024_CHANNEL_CONF_2 0x83F2

#define I2C_IIS_DEVICE_ADDRESS 0x19

#define IIS_REG_CONFIGURATION 0x20
#define IIS_REG_CONF_DATA 0x55

#define I2C_BH1750_DEVICE_ADDRESS 0x23
#define BH1750_POWER_ON_DATA 0x01
#define BH1750_MODE_DATA 0x10

#define IIS_X_LSB 0x28
#define IIS_X_MSB 0x29
#define IIS_Y_LSB 0x2A
#define IIS_Y_MSB 0x2B
#define IIS_Z_LSB 0x2C
#define IIS_Z_MSB 0x2D

struct coordinate_pair {
    int lsb;
    int msb;
};

static _Atomic bool is_initialized = false;

static pthread_mutex_t bus_mutex = PTHREAD_MUTEX_INITIALIZER;

static int device_mapping[] = { TLA2024_CHANNEL_CONF_0, TLA2024_CHANNEL_CONF_1, TLA2024_CHANNEL_CONF_2 };

static int i2c_file_desc = -1;

static struct coordinate_pair iis_mapping[] = { {IIS_X_LSB, IIS_X_MSB}, {IIS_Y_LSB, IIS_Y_MSB}, {IIS_Z_LSB, IIS_Z_MSB} };

static void init_i2c_address(int address) {
    if (i2c_file_desc == -1) {
        printf("I2C bus was not opened prior to usage (%s)\n", I2CDRV_LINUX_BUS);
        perror("Error is:");
        exit(EXIT_FAILURE);
    }
    if (ioctl(i2c_file_desc, I2C_SLAVE, address) == -1) {
        perror("Unable to set I2C device to slave address.");
        exit(EXIT_FAILURE);
    }
}

static void write_i2c_reg16(uint8_t reg_addr, uint16_t value) {
    int tx_size = 1 + sizeof(value);
    uint8_t buff[tx_size];
    buff[0] = reg_addr;
    buff[1] = (value & 0xFF);
    buff[2] = (value & 0xFF00) >> 8;
    int bytes_written = write(i2c_file_desc, buff, tx_size);
    if (bytes_written != tx_size) {
        perror("Unable to write i2c register");
        exit(EXIT_FAILURE);
    }
}

static uint16_t read_i2c_reg16(uint8_t reg_addr) {
    // To read a register, must first write the address
    int bytes_written = write(i2c_file_desc, &reg_addr, sizeof(reg_addr));
    if (bytes_written != sizeof(reg_addr)) {
        perror("Unable to write i2c register.");
        exit(EXIT_FAILURE);
    }
    // Now read the value and return it
    uint16_t value = 0;
    int bytes_read = read(i2c_file_desc, &value, sizeof(value));
    if (bytes_read != sizeof(value)) {
        perror("Unable to read i2c register");
        exit(EXIT_FAILURE);
    }
    return value;
}

static void sleepForMs(long long delayInMs)
{
    const long long NS_PER_MS = 1000 * 1000;
    const long long NS_PER_SECOND = 1000000000;
    long long delayNs = delayInMs * NS_PER_MS;
    int seconds = delayNs / NS_PER_SECOND;
    int nanoseconds = delayNs % NS_PER_SECOND;
    struct timespec reqDelay = {seconds, nanoseconds};
    nanosleep(&reqDelay, (struct timespec *)NULL);
}

uint16_t i2c_getTLAValue(enum TLA_device device) {
    assert(is_initialized);
    uint16_t value;

    pthread_mutex_lock(&bus_mutex);

    init_i2c_address(I2C_TLA2024_DEVICE_ADDRESS);

    write_i2c_reg16(TLA2024_REG_CONFIGURATION, device_mapping[device]);
    sleepForMs(5); // Add delay to allow configuration to set correctly
    uint16_t raw_read = read_i2c_reg16(TLA2024_REG_DATA);

    pthread_mutex_unlock(&bus_mutex);
    
    value = ((raw_read & 0xFF) << 8) | ((raw_read & 0xFF00) >> 8);
    value = value >> 4;

    return value;
}

static void write_i2c_reg8(uint8_t reg_addr, uint8_t value) {
    int tx_size = sizeof(reg_addr) + sizeof(value);
    uint8_t buff[tx_size];
    buff[0] = reg_addr;
    buff[1] = value;
    int bytes_written = write(i2c_file_desc, buff, tx_size);
    if (bytes_written != tx_size) {
        perror("Unable to write i2c register");
        exit(EXIT_FAILURE);
    }
}

static uint8_t read_i2c_reg8(uint8_t reg_addr) {
    // To read a register, must first write the address
    int bytes_written = write(i2c_file_desc, &reg_addr, sizeof(reg_addr));
    if (bytes_written != sizeof(reg_addr)) {
        perror("Unable to write i2c register.");
        exit(EXIT_FAILURE);
    }
    // Now read the value and return it
    uint8_t value = 0;
    int bytes_read = read(i2c_file_desc, &value, sizeof(value));
    if (bytes_read != sizeof(value)) {
        perror("Unable to read i2c register");
        exit(EXIT_FAILURE);
    }
    return value;
}

int16_t i2c_getIISValue(int dimension) {
    assert(is_initialized);
    int16_t value;

    pthread_mutex_lock(&bus_mutex);
    init_i2c_address(I2C_IIS_DEVICE_ADDRESS);

    write_i2c_reg8(IIS_REG_CONFIGURATION, IIS_REG_CONF_DATA);
    uint8_t raw_read_LSB = read_i2c_reg8(iis_mapping[dimension].lsb);
    uint8_t raw_read_MSB = read_i2c_reg8(iis_mapping[dimension].msb);

    pthread_mutex_unlock(&bus_mutex);

    value = ((raw_read_MSB) << 8) | (raw_read_LSB);

    return value;
}

static void write_i2c_raw16(uint8_t value) {
    int bytes_written = write(i2c_file_desc, &value, 1);
    if (bytes_written != 1) {
        perror("Unable to write i2c raw");
        exit(EXIT_FAILURE);
    }
}

static uint16_t read_i2c_raw16() {
    // Now read the value and return it
    uint16_t value = 0;
    int bytes_read = read(i2c_file_desc, &value, sizeof(value));
    if (bytes_read != sizeof(value)) {
        perror("Unable to read i2c register");
        exit(EXIT_FAILURE);
    }
    return value;
}

uint16_t i2c_getBH1750Value(void) {
    assert(is_initialized);

    uint16_t value;

    pthread_mutex_lock(&bus_mutex);

    init_i2c_address(I2C_BH1750_DEVICE_ADDRESS);

    uint16_t raw_read = read_i2c_raw16();

    pthread_mutex_unlock(&bus_mutex);
    
    value = ((raw_read & 0xFF) << 8) | ((raw_read & 0xFF00) >> 8);
    value = value / 1.2;
    // printf("from i2c getBH1740: %d\n", value);

    return value;
}

void i2c_init(void) {
    assert(!is_initialized);

    // Initialize i2c file descriptor
    i2c_file_desc = open(I2CDRV_LINUX_BUS, O_RDWR);
    if (i2c_file_desc == -1) {
        printf("I2C DRV: Unable to open bus for read/write (%s)\n", I2CDRV_LINUX_BUS);
        perror("Error is:");
        exit(EXIT_FAILURE);
    }

    // Initialize light sensor reading
    init_i2c_address(I2C_BH1750_DEVICE_ADDRESS);
    write_i2c_raw16(BH1750_POWER_ON_DATA);
    write_i2c_raw16(BH1750_MODE_DATA);

    is_initialized = true;
}

void i2c_cleanup(void) {
    assert(is_initialized);

    close(i2c_file_desc);
    is_initialized = false;
}
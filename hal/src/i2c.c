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

static struct coordinate_pair iis_mapping[] = { {IIS_X_LSB, IIS_X_MSB}, {IIS_Y_LSB, IIS_Y_MSB}, {IIS_Z_LSB, IIS_Z_MSB} };

static int init_i2c_bus(char* bus, int address) {
    int i2c_file_desc = open(bus, O_RDWR);
    if (i2c_file_desc == -1) {
        printf("I2C DRV: Unable to open bus for read/write (%s)\n", bus);
        perror("Error is:");
        exit(EXIT_FAILURE);
    }
    if (ioctl(i2c_file_desc, I2C_SLAVE, address) == -1) {
        perror("Unable to set I2C device to slave address.");
        exit(EXIT_FAILURE);
    }
    return i2c_file_desc;
}

static void write_i2c_reg16(int i2c_file_desc, uint8_t reg_addr, uint16_t value) {
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

static uint16_t read_i2c_reg16(int i2c_file_desc, uint8_t reg_addr) {
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

uint16_t i2c_getTLAValue(enum TLA_device device) {
    assert(is_initialized);
    uint16_t value;

    pthread_mutex_lock(&bus_mutex);

    int i2c_file_desc = init_i2c_bus(I2CDRV_LINUX_BUS, I2C_TLA2024_DEVICE_ADDRESS);

    write_i2c_reg16(i2c_file_desc, TLA2024_REG_CONFIGURATION, device_mapping[device]);
    uint16_t raw_read = read_i2c_reg16(i2c_file_desc, TLA2024_REG_DATA);

    close(i2c_file_desc);

    pthread_mutex_unlock(&bus_mutex);
    
    value = ((raw_read & 0xFF) << 8) | ((raw_read & 0xFF00) >> 8);
    value = value >> 4;

    return value;
}

static void write_i2c_reg8(int i2c_file_desc, uint8_t reg_addr, uint8_t value) {
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

static uint8_t read_i2c_reg8(int i2c_file_desc, uint8_t reg_addr) {
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
    int i2c_file_desc = init_i2c_bus(I2CDRV_LINUX_BUS, I2C_IIS_DEVICE_ADDRESS);

    write_i2c_reg8(i2c_file_desc, IIS_REG_CONFIGURATION, IIS_REG_CONF_DATA);
    uint8_t raw_read_LSB = read_i2c_reg8(i2c_file_desc, iis_mapping[dimension].lsb);
    uint8_t raw_read_MSB = read_i2c_reg8(i2c_file_desc, iis_mapping[dimension].msb);

    close(i2c_file_desc);

    pthread_mutex_unlock(&bus_mutex);

    value = ((raw_read_MSB) << 8) | (raw_read_LSB);

    return value;
}

static void write_i2c_raw16(int i2c_file_desc, uint8_t value) {
    int bytes_written = write(i2c_file_desc, &value, 1);
    if (bytes_written != 1) {
        perror("Unable to write i2c raw");
        exit(EXIT_FAILURE);
    }
}

static uint16_t read_i2c_raw16(int i2c_file_desc) {
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

    int i2c_file_desc = init_i2c_bus(I2CDRV_LINUX_BUS, I2C_BH1750_DEVICE_ADDRESS);

    uint16_t raw_read = read_i2c_raw16(i2c_file_desc);

    close(i2c_file_desc);

    pthread_mutex_unlock(&bus_mutex);
    
    value = ((raw_read & 0xFF) << 8) | ((raw_read & 0xFF00) >> 8);
    value = value / 1.2;
    printf("%d\n", value);

    return value;
}

void i2c_init(void) {
    assert(!is_initialized);

    int i2c_file_desc = init_i2c_bus(I2CDRV_LINUX_BUS, I2C_BH1750_DEVICE_ADDRESS);

    write_i2c_raw16(i2c_file_desc, BH1750_POWER_ON_DATA);
    write_i2c_raw16(i2c_file_desc, BH1750_MODE_DATA);

    is_initialized = true;
}
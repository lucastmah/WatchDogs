#include "hal/joystick.h"

#define I2CDRV_LINUX_BUS "/dev/i2c-1"
#define I2C_DEVICE_ADDRESS 0x48

#define REG_CONFIGURATION 0x01
#define REG_DATA 0x00

#define TLA2024_CHANNEL_CONF_0 0x83C2
#define TLA2024_CHANNEL_CONF_1 0x83D2
#define TLA2024_CHANNEL_CONF_2 0x83F2

static int joystick_X_min = 10;
static int joystick_X_max = 1600;
static int joystick_Y_min = 10;
static int joystick_Y_max = 1630;

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

static struct joystickState joystick_getState(void) {
    uint16_t X_value;
    uint16_t Y_value;
    float X_scaled;
    float Y_scaled;
    int i2c_file_desc = init_i2c_bus(I2CDRV_LINUX_BUS, I2C_DEVICE_ADDRESS);

    // Y
    write_i2c_reg16(i2c_file_desc, REG_CONFIGURATION, TLA2024_CHANNEL_CONF_0);
    uint16_t Y_raw_read = read_i2c_reg16(i2c_file_desc, REG_DATA);

    Y_value = ((Y_raw_read & 0xFF) << 8) | ((Y_raw_read & 0xFF00) >> 8);
    Y_value = Y_value >> 4;

    if (Y_value < joystick_Y_min) {
        joystick_Y_min = Y_value;
    }

    if (Y_value > joystick_Y_max) {
        joystick_Y_max = Y_value;
    }

    Y_scaled = (float) Y_value / (float) (joystick_Y_max - joystick_Y_min) * 200 - 100;

    // X
    write_i2c_reg16(i2c_file_desc, REG_CONFIGURATION, TLA2024_CHANNEL_CONF_1);
    uint16_t X_raw_read = read_i2c_reg16(i2c_file_desc, REG_DATA);

    X_value = ((X_raw_read & 0xFF) << 8) | ((X_raw_read & 0xFF00) >> 8);
    X_value = X_value >> 4;

    if (X_value < joystick_X_min) {
        joystick_X_min = X_value;
    }

    if (X_value > joystick_X_max) {
        joystick_X_max = X_value;
    }

    X_scaled = (float) X_value / (float) (joystick_X_max - joystick_X_min) * 200 - 100;

    struct joystickState state = {X_scaled, Y_scaled, false};
    close(i2c_file_desc);
    return state;
}

enum DIRECTION joystick_getDirection() {
    struct joystickState state = joystick_getState();
    if(pow(state.X, 2) + pow(state.Y, 2) < pow(DEADZONE, 2)) {
        return CENTER;
    }
    if(fabs(state.X) < fabs(state.Y)) {
        if(state.Y < 0) {
            return DOWN;
        }
        return UP;
    }
    if(state.X < 0) {
        return LEFT;
    }
    return RIGHT;
}
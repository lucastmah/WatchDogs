#include "hal/joystick.h"
#include <time.h>
#include <assert.h>

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

static bool is_initialized = false;
static int joystick_i2c_file_desc;

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

static uint16_t joystick_getAxisValue(char axis) {
    write_i2c_reg16(joystick_i2c_file_desc, REG_CONFIGURATION, axis == 'x' ? TLA2024_CHANNEL_CONF_1 : TLA2024_CHANNEL_CONF_0);
    sleepForMs(5); // Add delay to allow configuration to set correctly
    uint16_t raw_read = read_i2c_reg16(joystick_i2c_file_desc, REG_DATA);
    uint16_t flipped = (raw_read >> 8) | (raw_read << 8);
    return flipped >> 4;
}

static float joystick_applyScaling(float value, char axis) {
    // Update max/min values
    int axis_min, axis_max;
    if (axis == 'y') {
        if (value < joystick_Y_min) {
            joystick_Y_min = value;
        }
    
        if (value > joystick_Y_max) {
            joystick_Y_max = value;
        }
        axis_min = joystick_Y_min;
        axis_max = joystick_Y_max;
    } else {
        if (value < joystick_X_min) {
            joystick_X_min = value;
        }
    
        if (value > joystick_X_max) {
            joystick_X_max = value;
        }
        axis_min = joystick_X_min;
        axis_max = joystick_X_max;
    }
    
    // Scale according to normalized_range
    float scaled = (float) value / (float) (axis_max - axis_min) * JOYSTICK_NORMALIZED_RANGE * 2 - JOYSTICK_NORMALIZED_RANGE;

    // Apply deadzone
    if (fabs(scaled) < JOYSTICK_DEADZONE_SIZE) {
        return 0;
    } else {
        float temp = (fabs(scaled) - JOYSTICK_DEADZONE_SIZE) / (JOYSTICK_NORMALIZED_RANGE - JOYSTICK_DEADZONE_SIZE) * JOYSTICK_NORMALIZED_RANGE;
        if (scaled < 0) {
            scaled = -temp;
        } else {
            scaled = temp;
        }
    }
    return scaled;
}

struct joystickState joystick_getState(void) {
    assert(is_initialized);

    // Y
    uint16_t Y_value = joystick_getAxisValue('y');
    float Y_scaled = joystick_applyScaling(Y_value, 'y');

    // X
    uint16_t X_value = joystick_getAxisValue('x');
    float X_scaled = joystick_applyScaling(X_value, 'x');

    struct joystickState state = {X_scaled, -Y_scaled}; // Y axis is inverted
    return state;
}

void joystick_init(void) {
    joystick_i2c_file_desc = init_i2c_bus(I2CDRV_LINUX_BUS, I2C_DEVICE_ADDRESS);

    assert(!is_initialized);
    is_initialized = true;
}

void joystick_cleanup(void) {
    close(joystick_i2c_file_desc);

    assert(is_initialized);
    is_initialized = false;
}
/* ------------------------------------------------------------ *
 * file:        lsm303d.h                                       *
 * purpose:     header file for getlsm303d.c and i2c_lsm303d.c  *
 *                                                              *
 * author:      09/04/2021 Frank4DD                             *
 *                                                              *
 * I2C wiring for MM-TXS05(LSM303D) to RPi via PMOD2PRI PMOD4:  *
 * ------------------------------------------------------------ *
 * 1:VDD---------------3.3V                                     * 
 * 2:GND---------------GND                                      *
 * 3:SDA---------------A4(I2C:SDA)                              *
 * 4:CLK---------------A5(I2C:SCL)                              *
 * 5:SDO(SA0)----------X (not connected assigns SA0=1)          *
 * 6:INT1--------------X (not connected)                        *
 * 7:INT2--------------X (not connected)                        *
 * 8:CS----------------X (not connected)                        *
 * ------------------------------------------------------------ */

/* ------------------------------------------------------------ *
 * Sensor address is 0x1d / 0b0011101 if  SA0=1(VDD - default)  *
 * or 0x1e / 0b0011110 if SA0=0(GND).                           *
 * ------------------------------------------------------------ */
#define I2CBUS        "/dev/i2c-1" // Raspi default I2C bus
#define I2C_ADDR            "0x1d" // The sensor default I2C addr
#define PRD_ID               0x49  // LSM303D responds with 0x49
#define POWER_MODE_NORMAL    0x00  // sensor default power mode

/* ------------------------------------------------------------ *
 * Sensor register address information                          *
 * ------------------------------------------------------------ */
#define LSM303D_TEMP_OUT_L      0x05    // Sensor temperature data register (read-only) LSB
#define LSM303D_TEMP_OUT_H      0x06    // Sensor temperature data register (read-only) MSB
#define LSM303D_STATUS_M        0x07    // Sensor magnetic status register (read-only)
#define LSM303D_OUT_X_L_M       0x08    // X-axis magnetic data register (read-only) LSB
#define LSM303D_OUT_X_H_M       0x09    // X-axis magnetic data register (read-only) MSB
#define LSM303D_OUT_Y_L_M       0x0A    // Y-axis magnetic register (read-only) LSB
#define LSM303D_OUT_Y_H_M       0x0B    // Y-axis magnetic data register (read-only) MSB
#define LSM303D_OUT_Z_L_M       0x0C    // Z-axis magnetic data register (read-only) LSB
#define LSM303D_OUT_Z_H_M       0x0D    // Z-axis magnetic data register (read-only) MSB
#define LSM303D_WHO_AM_I        0x0F    // Product ID register (read-only, aka WHO_AM_I)
#define LSM303D_CTRL0           0x1F    // rw
#define LSM303D_CTRL1           0x20    // rw
#define LSM303D_CTRL2           0x21    // rw
#define LSM303D_CTRL3           0x22    // rw
#define LSM303D_CTRL4           0x23    // rw
#define LSM303D_CTRL5           0x24    // rw
#define LSM303D_CTRL6           0x25    // rw
#define LSM303D_CTRL7           0x26    // rw
#define LSM303D_STATUS_A        0x27    // Sensor acceleration status register (read-only)
#define LSM303D_OUT_X_L_A       0x28    // X-axis acceleration data register (read-only) LSB
#define LSM303D_OUT_X_H_A       0x29    // X-axis acceleration data register (read-only) MSB
#define LSM303D_OUT_Y_L_A       0x2A    // Y-axis acceleration data register (read-only) LSB
#define LSM303D_OUT_Y_H_A       0x2B    // Y-axis acceleration data register (read-only) MSB
#define LSM303D_OUT_Z_L_A       0x2C    // Z-axis acceleration data register (read-only) LSB
#define LSM303D_OUT_Z_H_A       0x2D    // Z-axis acceleration data register (read-only) MSB

/* ------------------------------------------------------------ *
 * Define byte-as-bits printing for debug output                *
 * ------------------------------------------------------------ */
#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0')

/* ------------------------------------------------------------ *
 * global variables                                             *
 * ------------------------------------------------------------ */
int i2cfd;             // I2C file descriptor
int verbose;           // debug flag, 0 = normal, 1 = debug mode
float offset[3];       // sensor axis offset values
float declination;     // local declination value

/* ------------------------------------------------------------ *
 * LSM303D status and control data structure                      *
 * ------------------------------------------------------------ */
struct lsm303dinf{
   char prd_id;      // reg 0x20 returns 0x06 for type LSM303D
   char ctl_0_mode;  // reg 0x07 cont mode, cont freq, boost
   char ctl_1_mode;  // reg 0x08 resolution, selftest
};

/* ------------------------------------------------------------ *
 * LSM303D measurement data struct.                             *
 * ------------------------------------------------------------ */
struct lsm303ddata{
   float X;        // X component
   float Y;        // Y component
   float Z;        // Z component
};

/* ------------------------------------------------------------ *
 * external function prototypes for I2C bus communication       *
 * ------------------------------------------------------------ */
extern  void get_i2cbus(char*, char*);         // get the I2C bus file handle
extern  void lsm303d_set();                    // charge CAP and execute SET
extern  void lsm303d_reset();                  // charge CAP and execute RESET
extern   int lsm303d_swreset();                // SW reset clears registers
extern  void lsm303d_init();                   // initialize the sensor
extern   int lsm303d_dump();                   // dump the register map data
extern  void lsm303d_info(struct lsm303dinf*); // print sensor information
extern  char get_prdid();                      // get the sensor product id
extern   int set_cmfreq(int);                  // set continuous read frequency
extern   int lsm303d_read();                   // read sensor data
extern float get_heading();                    // calculate heading from raw data
extern   int delay(long msec);                 // create a Arduino-style delay

/* ------------------------------------------------------------ *
 * file:        i2c_lsm303d.c                                   *
 * purpose:     Communication with STMicroelectronic LSM303D    *
 *              sensors. Implements functions for get and set   *
 *              of sensor register data. Ths file belongs to    *
 *              the pi-lsm303d package. Functions are called    *
 *              from getlsm303d.c, globals are in lsm303d.h.    *
 *                                                              *
 * Requires:	I2C development packages i2c-tools libi2c-dev   *
 *                                                              *
 * author:      13/09/2021 Frank4DD                             *
 * note:        LSM303D does not support auto-increment reads   *
 * ------------------------------------------------------------ */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include "lsm303d.h"

/* ------------------------------------------------------------ *
 * get_i2cbus() - Enables the I2C bus communication. RPi 2,3,4  *
 * use /dev/i2c-1, RPi 1 used i2c-0, NanoPi Neo also uses i2c-0 *
 * ------------------------------------------------------------ */
void get_i2cbus(char *i2cbus, char *i2caddr) {

   if((i2cfd = open(i2cbus, O_RDWR)) < 0) {
      printf("Error failed to open I2C bus [%s].\n", i2cbus);
      exit(-1);
   }
   if(verbose == 1) printf("Debug: I2C bus device: [%s]\n", i2cbus);
   /* --------------------------------------------------------- *
    * Set I2C device (LSM303D I2C address is 0x1d or 0x1e)      *
    * --------------------------------------------------------- */
   int addr = (int)strtol(i2caddr, NULL, 16);
   if(verbose == 1) printf("Debug: Sensor address: [0x%02X]\n", addr);

   if(ioctl(i2cfd, I2C_SLAVE, addr) != 0) {
      printf("Error can't find sensor at address [0x%02X].\n", addr);
      exit(-1);
   }
   /* --------------------------------------------------------- *
    * I2C communication test is the only way to confirm success *
    * --------------------------------------------------------- */
   if(get_prdid(addr) == 0) {
      printf("Error: No response from I2C. addr [0x%02X]?\n", addr);
      exit(-1);
   }
   if(verbose == 1) printf("Debug: Got data @addr: [0x%02X]\n", addr);
}

/* --------------------------------------------------------------- *
 * get_prdid() returns the LSM303D product id from register 0x20.  *
 * --------------------------------------------------------------- */
char get_prdid() {
   char reg = LSM303D_WHO_AM_I;
   char buf = 0;
   if(write(i2cfd, &reg, 1) != 1) {
      printf("Error: I2C write failure for register 0x%02X\n", reg);
   }

   if(read(i2cfd, &buf, 1) != 1) {
      printf("Error: I2C read failure for register 0x%02X\n", reg);
   }
   return buf;
}

/* --------------------------------------------------------------- *
 * lsm303d_set() initialize the magnetization in normal direction  *
 * --------------------------------------------------------------- */
void lsm303d_set() {
//   char  buf[2] = {MMC3416_CTL0_ADDR, 0x80}; // set bit-8 in reg 0x07
//   if(verbose == 1) printf("Debug: Write databyte: [0x%02X] to   [0x%02X]\n", buf[1], buf[0]);
//   if(write(i2cfd, buf, 2) != 2) {
//      printf("Error: I2C write failure for register 0x%02X\n", buf[0]);
//      exit(-1);
//   }
//   delay(60);                    // wait >50ms for the CAP charge to finish
//
//   buf[0] = MMC3416_CTL0_ADDR;   // ctl-0 register 0x07
//   buf[1] = 0x20;                // bit-6: send SET CMD
//   if(verbose == 1) printf("Debug: Write databyte: [0x%02X] to   [0x%02X]\n", buf[1], buf[0]);
//   if(write(i2cfd, buf, 2) != 2) {
//      printf("Error: I2C write failure for register 0x%02X\n", buf[0]);
//      exit(-1);
//   }
}

/* --------------------------------------------------------------- *
 * lsm303d_reset()  reverses magnetization (180 degrees opposed)   *
 * --------------------------------------------------------------- */
void lsm303d_reset() {
//   char  buf[2] = {MMC3416_CTL0_ADDR, 0x80}; // set bit-8 in reg 0x07
//   if(verbose == 1) printf("Debug: Write databyte: [0x%02X] to   [0x%02X]\n", buf[1], buf[0]);
//   if(write(i2cfd, buf, 2) != 2) {
//      printf("Error: I2C write failure for register 0x%02X\n", buf[0]);
//      exit(-1);
//   }
//   delay(60);                    // wait >50ms for the CAP charge to finish
//
//   buf[0] = MMC3416_CTL0_ADDR;   // ctl-0 register 0x07
//   buf[1] = 0x40;                // bit-6: send RESET CMD
//   if(verbose == 1) printf("Debug: Write databyte: [0x%02X] to   [0x%02X]\n", buf[1], buf[0]);
//   if(write(i2cfd, buf, 2) != 2) {
//      printf("Error: I2C write failure for register 0x%02X\n", buf[0]);
//      exit(-1);
//   }
}

/* --------------------------------------------------------------- *
 * lsm303d_init() identifies the initial sensor offset, runs the   *
 * SET/RESET function for Null Field output temp compensation, and *
 * clears the sensor residual from strong external magnet exposure *
 * --------------------------------------------------------------- */
void lsm303d_init(struct lsm303ddata *lsm303dd) {
   float ds1[3] = {0, 0, 0};
   float ds2[3] = {0, 0, 0};

   if(verbose == 1) printf("Debug: lsm303d_init(): ...\n");

   /* ------------------------------------------------------------ *
    * Magnetic Resolution M_RES=11 (00 = low res, 11 = high-res)   *
    * Magnetic Output Data Rate M_ODR=001 6.25 Hz ODR (max 50hz)   *
    * ------------------------------------------------------------ */
   char buf[2] = {LSM303D_CTRL5, 0x64};
   if(verbose == 1) printf("Debug: Write databyte: [0x%02X] to   [0x%02X]\n", buf[1], buf[0]);
   if(write(i2cfd, buf, 2) != 2) {
      printf("Error: I2C write failure for register 0x%02X\n", buf[0]);
      exit(-1);
   }

   /* ------------------------------------------------------------ *
    * Magnetic full-scale selection MFS=01 +/- 4 gauss (default)   *
    * ------------------------------------------------------------ */
   buf[0] = LSM303D_CTRL6;
   buf[1] = 0x20;
   if(verbose == 1) printf("Debug: Write databyte: [0x%02X] to   [0x%02X]\n", buf[1], buf[0]);
   if(write(i2cfd, buf, 2) != 2) {
      printf("Error: I2C write failure for register 0x%02X\n", buf[0]);
      exit(-1);
   }

   /* ------------------------------------------------------------ *
    * MLP=0 low power mode off; MD=00 continuous-conversion mode   *
    * ------------------------------------------------------------ */
   buf[0] = LSM303D_CTRL7;
   buf[1] = 0x00;
   if(verbose == 1) printf("Debug: Write databyte: [0x%02X] to   [0x%02X]\n", buf[1], buf[0]);
   if(write(i2cfd, buf, 2) != 2) {
      printf("Error: I2C write failure for register 0x%02X\n", buf[0]);
      exit(-1);
   }

   offset[0] = 0; offset[1] = 0; offset[2] = 0; // clear offset
   if(verbose == 1) printf("Debug: lsm303d_init(): done\n");
}

/* --------------------------------------------------------------- *
 * lsm303d_dump() dumps the complete register map data (15 bytes). *
 * --------------------------------------------------------------- */
int lsm303d_dump() {
   /* ------------------------------------------------------ *
    * Read 64 bytes sensor reg data starting at 0x00         *
    * ------------------------------------------------------ */
   char buf[64] = {0};
   for(int i=0; i<64; i++) {
      if(write(i2cfd, &i, 1) != 1) {
         printf("Error: I2C write failure for register 0x%02X\n", i);
         exit(-1);
      }
      if(read(i2cfd, &buf[i], 1) !=1) {
         printf("Error: I2C read failure for register 0x%02X\n", i);
         exit(-1);
      }
   }

   /* ------------------------------------------------------ *
    * Display Register table                                 *
    * ------------------------------------------------------ */
   printf("------------------------------------------------------\n");
   printf("STMicroelectronics LSM303D register dump:\n");
   printf("------------------------------------------------------\n");
   printf(" reg    0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\n");
   printf("------------------------------------------------------\n");
   printf("[0x00]");

   for(int i=0; i<64; i++) {
      if(i >= 0 && i < 5) { printf(" --"); continue; }
      if(i == 14) {
         printf(" --"); continue; }   // 0x0e factory reserved
      if(i == 16) printf("\n[0x10]");
      if(i == 16 || i == 17) {
         printf(" --"); continue; }   // 0x10, 0x11 factory reserved
      if(i == 32) printf("\n[0x20]");
      if(i == 48) printf("\n[0x30]");
      printf(" %02X", buf[i]);
   }
   printf("\n\n");

   /* ------------------------------------------------------ *
    * Display register name table with hex and binary data   *
    * ------------------------------------------------------ */
   printf("Sensor Reg: hex  binary\n");
   printf("---------------------------\n");
   for(int i=0; i<64; i++) {
      switch(i) {
         case 5:
            printf("TEMP_OUT_L"); break;
         case 6:
            printf("TEMP_OUT_M"); break;
         case 7:
            printf("  STATUS_M"); break;
         case 8:
            printf(" OUT_X_L_M"); break;
         case 9:
            printf(" OUT_X_H_M"); break;
         case 10:
            printf(" OUT_Y_L_M"); break;
         case 11:
            printf(" OUT_Y_H_M"); break;
         case 12:
            printf(" OUT_Z_L_M"); break;
         case 13:
            printf(" OUT_X_H_M"); break;
         case 15:
            printf(" WHO_AMI_I"); break;

         default:
            continue;
      }
      printf(": 0x%02X 0b"BYTE_TO_BINARY_PATTERN"\n", buf[i], BYTE_TO_BINARY(buf[i]));
   }
   exit(0);
}

/* --------------------------------------------------------------- *
 * lsm303d_swreset() resets the sensor, and clears config settings *
 * --------------------------------------------------------------- */
int lsm303d_swreset() {
//   char data[2];
//   data[0] = MMC3416_CTL1_ADDR;
//   data[1] = 0xB6;
//   if(write(i2cfd, data, 2) != 2) {
//      printf("Error: I2C write failure for register 0x%02X\n", data[0]);
//      exit(-1);
//   }
//   if(verbose == 1) printf("Debug: Sensor SW Reset complete\n");
   exit(0);
}

/* ------------------------------------------------------------ *
 * lsm303d_info() - read sensor ID and settings from registers  *
 * 0x07, 0x08, 0x20:                                            *
 * char prd_id;      // reg 0x20 returns 0x06 as product ID     *
 * char cm_freq;     // reg 0x07 cont measurement freq bit-2,3  *
 * char boost_mode;  // reg 0x07 disable CAP charge pump bit-4  *
 * char outres_mode; // reg 0x08 output resolution mode bit-0,1 *
 * ------------------------------------------------------------ */
void lsm303d_info(struct lsm303dinf *lsm303di) {
   lsm303di->prd_id = get_prdid();
//
//   /* Read MMC3416_CTL0_ADDR data */ 
//   char reg = MMC3416_CTL0_ADDR;
//   if(write(i2cfd, &reg, 1) != 1) {
//      printf("Error: I2C write failure for register 0x%02X\n", reg);
//      exit(-1);
//   }
//
//   if(read(i2cfd, &lsm303di->ctl_0_mode, 1) != 1) {
//      printf("Error: I2C read failure for register 0x%02X\n", reg);
//      exit(-1);
//   }
//   if(verbose == 1) printf("Debug: Got ctl-0 byte: [0x%02X]\n",
//                            lsm303di->ctl_0_mode);
//
//   /* Read MMC3416_CTL1_ADDR data */ 
//   reg = MMC3416_CTL1_ADDR;
//   if(write(i2cfd, &reg, 1) != 1) {
//      printf("Error: I2C write failure for register 0x%02X\n", reg);
//      exit(-1);
//   }
//
//   if(read(i2cfd, &lsm303di->ctl_1_mode, 1) != 1) {
//      printf("Error: I2C read failure for register 0x%02X\n", reg);
//      exit(-1);
//   }
//   if(verbose == 1) printf("Debug: Got ctl-1 byte: [0x%02X]\n",
//                            lsm303di->ctl_1_mode);
}

/* --------------------------------------------------------------- *
 * set_cmfreq() set the continuous read frequency in register 0x07 *
 * --------------------------------------------------------------- */
int set_cmfreq(int new_mode) {
   /* ---------------------------------------- */
   /* Check current freq from ctl-0 register   */
   /* ---------------------------------------- */
//   if(verbose == 1) printf("Debug: Set  Read Freq: [0x%02X]\n", new_mode);
//   char reg = MMC3416_CTL0_ADDR;
//   char regdata = 0;
//   if(write(i2cfd, &reg, 1) != 1) {
//      printf("Error: I2C write failure for register 0x%02X\n", reg);
//      exit(-1);
//   }
//   if(read(i2cfd, &regdata, 1) != 1) {
//      printf("Error: I2C read failure for register 0x%02X\n", reg);
//      exit(-1);
//   }
//   if(verbose == 1) printf("Debug: Read data byte: [0x%02X] from [0x%02X]\n", regdata, reg);
//
//   /* ---------------------------------------- */
//   /* frequency mode from reg 0x07 bit-2 and 3 */
//   /* ---------------------------------------- */
//   int current_mode = ((regdata >> 2) & 0x03);
//   if(verbose == 1) printf("Debug: Cont Read Freq: [0x%02X]\n", current_mode);
//
//   /* ---------------------------------------- */
//   /* Check if update is needed, or just exit  */
//   /* ---------------------------------------- */
//   if(new_mode == current_mode) {
//      if(verbose == 1) printf("Debug: New freq = current freq, no change.\n");
//      return(0);
//   }
// 
//   /* ---------------------------------------- */
//   /* Set new freq bits 2, 3 in ctl-0 register */
//   /* ---------------------------------------- */
//   regdata |= 1 << 0;       // bit-0: 1 start measuring
//   regdata |= 1 << 1;       // bit-1: 1 enable continous mode = on
//
//   if(new_mode == 0) {
//      regdata &= ~(1 << 2); // bit-2: 0
//      regdata &= ~(1 << 3); // bit-3: 0
//   }
//   else if(new_mode == 1) {
//      regdata |= 1 << 2;    // bit-2: 1
//      regdata &= ~(1 << 3); // bit-3: 0
//   }
//   else if(new_mode == 2) {
//      regdata &= ~(1 << 2); // bit-2: 0
//      regdata |= 1 << 3;    // bit-3: 1
//   }
//   else if(new_mode == 3) {
//      regdata |= 1 << 2;    // bit-2: 1
//      regdata |= 1 << 3;    // bit-3: 1
//   }
//   regdata |= 1 << 5;       // bit-5: Set the data
//
//   /* ---------------------------------------- */
//   /* write new setting to ctl-0 register 0x07 */
//   /* ---------------------------------------- */
//   char buf[2] = {0};
//   buf[0] = reg;
//   buf[1] = regdata;
//   if(verbose == 1) printf("Debug: Write databyte: [0x%02X] to   [0x%02X]\n", buf[1], buf[0]);
//   if(write(i2cfd, buf, 2) != 2) {
//      printf("Error: I2C write failure for register 0x%02X\n", buf[0]);
//      return(-1);
//   }
//
//   /* ---------------------------------------- */
//   /* read the changed data back from register */
//   /* ---------------------------------------- */
//   if(write(i2cfd, &reg, 1) != 1) {
//      printf("Error: I2C write failure for register 0x%02X\n", reg);
//   }
//
//   regdata = 0;
//   if(read(i2cfd, &regdata, 1) != 1) {
//      printf("Error: I2C read failure for register 0x%02X\n", reg);
//   }
//   if(verbose == 1) printf("Debug: Read data byte: [0x%02X] from [0x%02X]\n", regdata, reg);
//   /* cont read frequency mode from reg 0x07 bit-2 and 3 */
//   current_mode = ((regdata >> 2) & 0x03);
//   if(new_mode != current_mode) {
//      if(verbose == 1) printf("Debug: Update failed. New mode %d\n", current_mode);
//      return -1;
//   }
//   if(verbose == 1) printf("Debug: Update sucess. New mode %d\n", current_mode);
   return(0);
}


/* ------------------------------------------------------------ *
 *  lsm303d_read() - take a single data read over the XYZ axis  *
 *  convert to Milli Gauss, and store under the lsm303d object. *
 * ------------------------------------------------------------ */
int lsm303d_read(struct lsm303ddata *lsm303dd) {
   /* ---------------------------------------- */
   /* Request new measurement through reg 0x07 */
   /* ---------------------------------------- */
//   char buf[2] = {0};
//   buf[0] = MMC3416_CTL0_ADDR;   // ctl-0 register 0x07
//   buf[1] = 0x01;                // bit-0: 1 request a new measurement
//   if(verbose == 1) printf("Debug: Write databyte: [0x%02X] to   [0x%02X]\n", buf[1], buf[0]);
//   if(write(i2cfd, buf, 2) != 2) {
//      printf("Error: I2C write failure for register 0x%02X\n", buf[0]);
//      return(-1);
//   }
//   if(verbose == 1) printf("Debug: Wait for measurement:\n");
//
//   /* ---------------------------------------- */
//   /* Check status "result ready" in reg 0x06  */
//   /* ---------------------------------------- */
//   char reg = MMC3416_STATUS_ADDR;
//   char regdata = 0;
//   while(1) {
//      if(write(i2cfd, &reg, 1) != 1) {
//         printf("Error: I2C write failure for register 0x%02X\n", reg);
//      }
//
//      if(read(i2cfd, &regdata, 1) != 1) {
//         printf("Error: I2C read failure for register 0x%02X\n", reg);
//      }
//      if(verbose == 1) printf("Debug: Read data byte: [0x%02X] from [0x%02X]\n", regdata, reg);
//
//      //if the last bit is 1, data is ready
//      if((regdata & 0x01) == 1) break;
//      delay(10);  // wait time
//   }
//   if(verbose == 1) printf("Debug: measurement is ready.\n");
//
//   /* ---------------------------------------- */
//   /* Data is ready to read from 0x00..0x05    */
//   /* ---------------------------------------- */
//   reg = MMC3416_XOUT_LSB_ADDR;
//   if(write(i2cfd, &reg, 1) != 1) {
//      printf("Error: I2C write failure for register 0x%02X\n", reg);
//   }
//   char measure[6] = {0, 0, 0, 0, 0, 0};
//   if(read(i2cfd, measure, 6) != 6) {
//      printf("Error: I2C read failure for register 0x%02X\n", reg);
//   }
//   if(verbose == 1) {
//      for(int i=0; i<6; i++) {
//         printf("Debug: Read data byte: [0x%02X] from [0x%02X]\n", measure[i], reg+i);
//      }
//   }
//
//   /* ---------------------------------------- */
//   /* Combine LSB/MSB into 16-bit value X Y Z  */
//   /* ---------------------------------------- */
//   uint16_t measured_data[3];
//   measured_data[0] = measure[1] << 8 | measure[0]; // X
//   measured_data[1] = measure[3] << 8 | measure[2]; // Y
//   measured_data[2] = measure[5] << 8 | measure[4]; // Z
//
//   /* ---------------------------------------- */
//   /* Convert raw X Y Z data to milli Gauss    */
//   /* ---------------------------------------- */
//   lsm303dd->X = 0.48828125 * (float) measured_data[0] - offset[0];
//   lsm303dd->Y = 0.48828125 * (float) measured_data[1] - offset[1];
//   lsm303dd->Z = 0.48828125 * (float) measured_data[2] - offset[2];
//   if(verbose == 1) printf("Debug: Measured value: X-[%3.02f] Y-[%3.02f] Z-[%3.02f]\n",
//                            lsm303dd->X, lsm303dd->Y, lsm303dd->Z);
//
   return(0);
}

/* ------------------------------------------------------- *
 * get_heading() convert two-axis value to compass heading *
 * ------------------------------------------------------- */
float get_heading(struct lsm303ddata *lsm303dd) {
   float temp0 = 0; // partial result 0
   float temp1 = 0; // partial result 1
   float deg = 0;   // final result

//   /* -------------------------------------------- */
//   /* Calculate heading from magnetic field data.  */
//   /* each quadrant has its own formula. Quadrant1 */
//   /* -------------------------------------------- */
//   if (lsm303dd->X < 0) {
//      if (lsm303dd->Y > 0) { //Quadrant 1
//         temp0 = lsm303dd->Y;
//         temp1 = -lsm303dd->X;
//         deg = 90 - atan(temp0 / temp1) * (180 / 3.14159);
//      }
//      else { //Quadrant 2
//         temp0 = -lsm303dd->Y;
//         temp1 = -lsm303dd->X;
//         deg = 90 + atan(temp0 / temp1) * (180 / 3.14159);
//      }
//   }
//   else { 
//      if (lsm303dd->Y < 0) { //Quadrant 3
//         temp0 = -lsm303dd->Y;
//         temp1 = lsm303dd->X;
//         deg = 270 - atan(temp0 / temp1) * (180 / 3.14159);
//      }
//      else { //Quadrant 4
//         temp0 = lsm303dd->Y;
//         temp1 = lsm303dd->X;
//         deg = 270 + atan(temp0 / temp1) * (180 / 3.14159);
//      }
//   }
//   deg += declination;
//   if (declination > 0) {
//      if (deg > 360) deg -= 360;
//   } else {
//      if (deg < 0) deg += 360;
//   }
   return deg;
}

/* ------------------------------------------------------- * 
 * delay() Sleep for the requested number of milliseconds. * 
 * ------------------------------------------------------- */
int delay(long msec) {
   struct timespec ts;
   int res;

   if (msec < 0) { errno = EINVAL; return -1; }

   ts.tv_sec = msec / 1000;
   ts.tv_nsec = (msec % 1000) * 1000000;

   do { res = nanosleep(&ts, &ts); }
   while (res && errno == EINTR);

   return res;
}

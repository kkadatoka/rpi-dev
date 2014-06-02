// RDA5807M Radio App for Raspberry Pi - redhawk 04/04/2014
//
// This code is provided to help with programming the RDA chip.
// Some features like auto scan and RDS/RBDS are not included.
//
// When using i2c for the first please read the following article:
// http://learn.adafruit.com/adafruits-raspberry-pi-lesson-4-gpio-setup/configuring-i2c
//
// For the required wiringPi library please visit:
// https://projects.drogon.net/raspberry-pi/wiringpi/i2c-library/
//
// To compile save source file as rda.c and run:
// gcc -Wall -o rda rda.c -lwiringPi
//

#include <wiringPiI2C.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void write();
void read();

typedef unsigned short int	uint16_t;
typedef unsigned char		uint8_t;

uint8_t i2c_write_address = 0x10;
uint8_t i2c_read_address = 0x11;
uint16_t i2c_write_handle;
uint16_t i2c_read_handle;
uint8_t out_buffer[12] = {0};
uint8_t read_bug;

// CHIP ID //
# define rda_CHIP_ID	0x58

// Timing XTAL //
#define rda_32_768kHz	0b0000000000000000
#define rda_12MHz	0b0000000000010000
#define rda_24MHz	0b0000000001010000
#define rda_13MHz	0b0000000000100000
#define rda_26MHz	0b0000000001100000
#define rda_19_2MHz	0b0000000000110000
#define rda_38_4MHz	0b0000000001110000

// Tuning Band //
#define rda_87_108MHz	0b0000000000000000
#define rda_76_91MHz	0b0000000000000100
#define rda_76_108MHz	0b0000000000001000
#define rda_65_76MHz	0b0000000000001100

// Tuning Steps //
#define rda_100kHz	0b0000000000000000
#define rda_200kHz	0b0000000000000001 // not US band compatible **//
#define rda_50kHz	0b0000000000000010
#define rda_25kHz	0b0000000000000011
// ** 200kHz spacing works but frequencies are always out by 100kHz from US frequency slots.
// It's not possible to tune to a US station with 200kHz spacing you must use 100kHz instead.

// De-emphasis //
#define rda_50us 	0b0000100000000000
#define rda_75us 	0b0000000000000000

// REG 0x02
#define rda_DHIZ	0b1000000000000000
#define rda_DMUTE	0b0100000000000000
#define rda_MONO	0b0010000000000000
#define rda_BASS	0b0001000000000000
#define rda_RCLK	0b0000100000000000
#define rda_RCKL_DIM	0b0000010000000000
#define rda_SEEKUP	0b0000001000000000
#define rda_SEEK	0b0000000100000000
#define rda_SKMODE	0b0000000010000000
#define rda_CLK_MODE	0b0000000001110000
#define rda_RDS_EN	0b0000000000001000
#define rda_NEW_METHOD	0b0000000000000100
#define rda_SOFT_RESET	0b0000000000000010
#define rda_ENABLE	0b0000000000000001
// REG 0x03
#define rda_CHAN	0b1111111111000000
#define rda_DIRECT_MODE	0b0000000000100000
#define rda_TUNE	0b0000000000010000
#define rda_BAND	0b0000000000001100
#define rda_SPACE	0b0000000000000011
// REG 0x04
#define rda_DE		0b0000100000000000
#define rda_SOFTMUTE_EN	0b0000001000000000
#define rda_AFCD	0b0000000100000000
// REG 0x05
#define rda_INT_MODE	0b1000000000000000
#define rda_SEEKTH	0b0000111100000000
#define rda_VOLUME	0b0000000000001111
// REG 0x06
#define rda_OPEN_MODE	0b0110000000000000
// REG 0x07
#define rda_BLEND_TH	0b0111110000000000
#define rda_65_50M_MODE	0b0000001000000000
#define rda_SEEK_TH_OLD	0b0000000011111100
#define rda_BLEND_EN	0b0000000000000010
#define rda_FREQ_MODE	0b0000000000000001
// REG 0x0A
#define rda_RDSR	0b1000000000000000
#define rda_STC		0b0100000000000000
#define rda_SF		0b0010000000000000
#define rda_RDSS	0b0001000000000000
#define rda_BLK_E	0b0000100000000000
#define rda_ST		0b0000010000000000
#define rda_READCHAN	0b0000001111111111
// REG 0x0B
#define rda_RSSI	0b1111110000000000
#define rda_FM_TRUE	0b0000001000000000
#define rda_FM_READY	0b0000000100000000
#define rda_ABCD_E	0b0000000000010000
#define rda_BLERA	0b0000000000001100
#define rda_BLERB	0b0000000000000011

//    -        -         -            -             -            -    //

uint16_t read_chip(uint8_t offset) {
  uint16_t data = wiringPiI2CReadReg16(i2c_read_handle, offset);
  if (read_bug != 0) {
    data = data>>8|((data<<8)&0xffff); // fix high_byte low_byte swap bug
  }
  return data;
}

void write_chip(uint8_t size) {
  write (i2c_write_handle, (unsigned int)out_buffer, size);
}

void write_setting(char *name, uint16_t value) {
  uint16_t data;
  uint8_t loop;
  if (strcmp(name, "init") == 0) {
    // REG 02
    // normal output, enable mute, stereo, no bass boost, clock = 32.768kHz, RDS enabled, new demod method, power on
    data = rda_DHIZ|(rda_DMUTE&0)|(rda_MONO&0)|(rda_BASS&0)|rda_32_768kHz|rda_RDS_EN|rda_NEW_METHOD|rda_ENABLE;
    out_buffer[0] = data >>8;
    out_buffer[1] = data &0xff;
    // REG 03 - no auto tune, 76-108 band, 0.1 spacing
    data = (rda_TUNE&0)|rda_76_108MHz|rda_100kHz;
    out_buffer[2] = data >>8;
    out_buffer[3] = data &0xff;
    // REG 04 - audio 50us, no soft mute, disble AFC
    data = rda_50us|(rda_SOFTMUTE_EN&0)|rda_AFCD;
    out_buffer[4] = data >>8;
    out_buffer[5] = data &0xff;
    // REG 05 - max volume
    data = rda_INT_MODE|0x0880|rda_VOLUME;
    out_buffer[6] = data >>8;
    out_buffer[7] = data &0xff;
    // REG 06 - reserved
    out_buffer[8] = 0;
    out_buffer[9] = 0;
    // REG 07
    uint16_t blend_threshold = 0b0011110000000000; // mix L+R with falling signal strength
    data = blend_threshold|rda_65_50M_MODE|0x80|0x40|rda_BLEND_EN|(rda_FREQ_MODE&0);
    out_buffer[10] = data >>8;
    out_buffer[11] = data &0xff;
    write_chip(12);
  }
  if (strcmp(name, "off") == 0) {
    data = (read_chip(2)|rda_ENABLE)^rda_ENABLE;
    out_buffer[0] = data >>8;
    out_buffer[1] = data &0xff;
  }
  if (strcmp(name, "dmute") == 0) {
    data = (read_chip(2)|rda_DMUTE);
    if (value == 0) { data = data^rda_DMUTE; }
    out_buffer[0] = data >> 8;
    out_buffer[1] = data & 0xff;
  }
  if (strcmp(name, "mono") == 0) {
    data = (read_chip(2)|rda_MONO);
    if (value == 0) { data = data^rda_MONO; }
    out_buffer[0] = data >> 8;
    out_buffer[1] = data & 0xff;
  }
  if (strcmp(name, "bass") == 0) {
    data = (read_chip(2)|rda_BASS);
    if (value == 0) { data = data^rda_BASS; }
    out_buffer[0] = data >> 8;
    out_buffer[1] = data & 0xff;
  }
  if (strcmp(name, "chan") == 0) {
    data = (read_chip(3)|rda_CHAN)^rda_CHAN;
    out_buffer[2] = (value / 4)|(data >> 8);
    out_buffer[3] = (value * 64)|(data & 0xff);
  }
  if (strcmp(name, "tune") == 0) {
    data = ((out_buffer[2]<<8)|out_buffer[3])|rda_TUNE;
    if (value == 0) { data = data^rda_TUNE; }
    out_buffer[2] = data >> 8;
    out_buffer[3] = data & 0xff;
  }
  if (strcmp(name, "de") == 0) {
    write_setting("read_chip",0);
    data = (read_chip(4)|rda_DE)^rda_DE;
    if (value == 0) { data = data | rda_50us; }
    if (value == 1) { data = data | rda_75us; }
    out_buffer[4] = data >> 8;
    out_buffer[5] = data & 0xff;
  }
  if (strcmp(name, "volume") == 0) {
    write_setting("read_chip",0);
    data = (read_chip(5)|rda_VOLUME)^rda_VOLUME;
    if (value > rda_VOLUME) { value = rda_VOLUME; }
    data = data|value;
    out_buffer[6] = data >> 8;
    out_buffer[7] = data & 0xff;
 }
  if (strcmp(name, "read_chip") == 0) {
    for (loop = 2; loop < 8; loop++) {
      data = read_chip(loop);
      out_buffer[(loop*2)-4] = data >> 8;
      out_buffer[(loop*2)-3] = data & 0xff;
    }
    write_setting("tune",0); // disable tuning
  }
}

uint16_t read_setting(char *name) {
  uint16_t data = 0;
  if (strcmp(name, "dmute") == 0) { data = read_chip(2) & rda_DMUTE; }
  if (strcmp(name, "mono") == 0) { data = read_chip(2) & rda_MONO; }
  if (strcmp(name, "bass") == 0) { data = read_chip(2) & rda_BASS; }
  if (strcmp(name, "band") == 0) { data = read_chip(3) & rda_BAND; }
  if (strcmp(name, "space") == 0) { data = read_chip(3) & rda_SPACE; }
  if (strcmp(name, "de") == 0) { data = read_chip(4) & rda_DE; }
  if (strcmp(name, "volume") == 0) { data = read_chip(5) & rda_VOLUME; }
  if (strcmp(name, "st") == 0) { data = read_chip(10) & rda_ST; }
  if (strcmp(name, "rssi") == 0) { data = ((read_chip(11) & rda_RSSI)>>10); }
  return data;
}

uint16_t init_chip() {
  uint16_t data;
  uint8_t found = 0;
  uint8_t result;
  i2c_write_handle = wiringPiI2CSetup(i2c_write_address);
  if (i2c_write_handle < 0){
    printf("i2c device not found\n");
    exit(1);
  }
  i2c_read_handle = wiringPiI2CSetup(i2c_read_address);
  if (i2c_read_handle < 0){
    printf("i2c device not found\n");
    exit(1);
  }
  data = read_chip(0);
  if ((data&0xff) == rda_CHIP_ID) {
    found = 1;
    read_bug = 1;
  }
  data = data<<8;
  if (data == rda_CHIP_ID) {
    found = 1;
    read_bug = 0;
  }
  if (found == 0) {
    printf("i2c device not found\n");
    exit(1);
  }
  if ((read_chip(13) == 0x5804) && (read_chip(15) == 0x5804)) { result = 0; } // not set up
  							 else { result = 1; } // already used
  return result;
}

void set_frequency(double freq_request) {
  uint8_t spacing;
  uint16_t start_freq;
  uint16_t data;
  uint16_t new_freq;
  uint16_t freq;
  if (freq_request < 760) { freq = freq_request * 10; }
	             else { freq = freq_request; }
  data = read_setting("band");
  if (data == rda_87_108MHz) { start_freq = 870; }
  if (data == rda_76_108MHz) { start_freq = 760; }
  if (data == rda_76_91MHz)  { start_freq = 760; }
  if (data == rda_65_76MHz)  { start_freq = 650; }
  data = read_setting("space");
  if (data == rda_200kHz)  { spacing = 0; }
  if (data == rda_100kHz)  { spacing = 1; }
  if (data == rda_50kHz)   { spacing = 2; }
  if (data == rda_25kHz)   { spacing = 4; }
  if (spacing > 0) { new_freq = (freq - start_freq) * spacing; }
              else { new_freq = (freq - start_freq) / 2; }
  write_setting("dmute",1);
  write_setting("chan",new_freq);
  write_setting("tune",1);
  write_chip(4);
}

void help(char *prog) {
  printf("\nRaspberry Pi - RDA5807M radio application\n\n");
  printf("Usage: %s freq (MHz) | off | other options\n",prog);
  printf("other options: mute | unmute\n");
  printf("volume [n] | vol [n] (n = 0 - 15)\n");
  printf("bass [on | off] (bass boost)\n");
  printf("50us | 50 | 75us | 75 (de-emphasis)\n");
  printf("stereo | s | mono | m (channel mode)\n\n");
  exit(0);
}

int main( int argc, char *argv[]) {
  if (argc < 2) { help(argv[0]); }
  uint16_t data;
  uint8_t state = init_chip();
  data = atoi(argv[1]);
  if (data > 75) {
    if (state == 0) {
      write_setting("init",0);
      system("sleep 0.2"); // delay next command after init
    }
    double freq_request = strtod(argv[1],NULL);
    set_frequency(freq_request);
  } else {
    if (strcmp(argv[1], "off") == 0) {
      write_setting("off",0);
      write_chip(2);
      exit(0);
    }
    if (state == 0) { printf("Radio is off / not tuned\n"); exit(1); }
    if (strcmp(argv[1], "mute") == 0) {
      write_setting("dmute",0);
      write_chip(2);
    }
    if (strcmp(argv[1], "unmute") == 0) {
      write_setting("dmute",1);
      write_chip(2);
    }
    if ((strcmp(argv[1], "volume") == 0)||(strcmp(argv[1], "vol") == 0)) {
      if (argc > 2) {
        data = atoi(argv[2]);
        write_setting("volume",data);
        write_chip(8);
      }
    }
    if (strcmp(argv[1], "bass") == 0) {
      if (argc > 2) {
        if (strcmp(argv[2], "on")  == 0) {
          write_setting("bass",1);
          write_chip(2);
        }
        if (strcmp(argv[2], "off") == 0) {
          write_setting("bass",0);
          write_chip(2);
        }
      }
    }
    if ((strcmp(argv[1], "stereo") == 0)||(strcmp(argv[1], "s") == 0)) {
      write_setting("mono",0);
      write_chip(2);
    }
    if ((strcmp(argv[1], "mono") == 0)||(strcmp(argv[1], "m") == 0)) {
      write_setting("mono",1);
      write_chip(2);
    }
    if ((strcmp(argv[1], "50us") == 0)||(strcmp(argv[1], "50") == 0)) {
      write_setting("de",0);
      write_chip(6);
    }
    if ((strcmp(argv[1], "75us") == 0)||(strcmp(argv[1], "75") == 0)) {
      write_setting("de",1);
      write_chip(6);
    }
  }
  return 0;
}
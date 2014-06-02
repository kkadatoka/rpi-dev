/*
 * Read Device code on SPI interfaced LCD Controller, with Start byte. Particularly the HY28A.
 *
 * Wire up the SPI bus: SCLK, MOSI and MISO. LCDB_CS => CE0
 * Wire up RESET to GPIO25
 *
 * SPI code taken from: https://raw.github.com/torvalds/linux/master/Documentation/spi/spidev_test.c
 *
 * GPIO code from: http://elinux.org/RPi_Low-level_peripherals#C (Dom and Gert)
 *            and: https://github.com/torvalds/linux/blob/master/drivers/gpio/gpiolib.c
 *
 */

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <sys/mman.h>


#define RESET_GPIO 22
#define SPIDEV "/dev/spidev0.0"
#define SPIMODE SPI_MODE_3
#define SPISPEED 500000


/* LCD Controller Start byte definitions */
#define SPI_START (0x04)   /* Start byte for SPI transfer */
#define SPI_RD (0x01)      /* WR bit 1 within start */
#define SPI_WR (0x00)      /* WR bit 0 within start */
#define SPI_DATA (0x02)    /* RS bit 1 within start byte */
#define SPI_INDEX (0x00)   /* RS bit 0 within start byte */


/* from include/linex/gpio.h */
#define GPIOF_DIR_OUT   (0 << 0)
#define GPIOF_DIR_IN   (1 << 0)
#define GPIOF_INIT_LOW   (0 << 1)
#define GPIOF_INIT_HIGH   (1 << 1)
#define GPIOF_IN      (GPIOF_DIR_IN)
#define GPIOF_OUT_INIT_LOW   (GPIOF_DIR_OUT | GPIOF_INIT_LOW)
#define GPIOF_OUT_INIT_HIGH   (GPIOF_DIR_OUT | GPIOF_INIT_HIGH)
int gpio_request_one(unsigned gpio, unsigned long flags, const char *label);
void gpio_free(unsigned gpio);
int gpio_direction_input(unsigned gpio);
int gpio_direction_output(unsigned gpio, int value);
void gpio_set_value(unsigned gpio, int value);

/* SPI functions */
int open_spidev(const char *device, uint8_t mode, uint32_t speed);
static void transfer(int fd, uint8_t *tx, uint8_t *rx, int len);

unsigned read_devicecode(int fd);




int main(int argc, char **argv)
{
   int rep;
   int spifd;

   gpio_request_one(RESET_GPIO, GPIOF_INIT_HIGH, "");

   /* reset lcd controller */
   gpio_set_value(RESET_GPIO, 0);
   gpio_set_value(RESET_GPIO, 1);

   spifd = open_spidev(SPIDEV, SPIMODE, SPISPEED);

   printf("\nDevice code: 0x%04X\n", read_devicecode(spifd));

   close(spifd);

   gpio_free(RESET_GPIO);

   return 0;
}


/*
 * SPI functions
 *
 */

static void pabort(const char *s)
{
   perror(s);
   abort();
}

int open_spidev(const char *device, uint8_t mode, uint32_t speed)
{
   int spifd, ret;

   spifd = open(device, O_RDWR);
   if (spifd < 0)
      pabort("can't open device");

   /*
    * spi mode
    */
   ret = ioctl(spifd, SPI_IOC_WR_MODE, &mode);
   if (ret == -1)
      pabort("can't set spi mode");

   ret = ioctl(spifd, SPI_IOC_RD_MODE, &mode);
   if (ret == -1)
      pabort("can't get spi mode");

   /*
    * max speed hz
    */
   ret = ioctl(spifd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
   if (ret == -1)
      pabort("can't set max speed hz");

   ret = ioctl(spifd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
   if (ret == -1)
      pabort("can't get max speed hz");

   return spifd;
}

void print_hex(const uint8_t *buf, int len)
{
   int i;

   for (i = 0; i < len; i++) {
//      if (!(i % 16))
//         puts("");
      printf("%.2X ", buf[i]);
   }
   puts("");
}


static void transfer(int fd, uint8_t *tx, uint8_t *rx, int len)
{
   int ret;
   struct spi_ioc_transfer tr = {
      .len = len,
   };

   printf("\n%s(len=%d):\n", __func__);

   if (tx) {
      tr.tx_buf = (unsigned long)tx;
      printf("  TX: ");
      print_hex(tx, len);
   }
   if (rx)
      tr.rx_buf = (unsigned long)rx;

   ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
   if (ret < 1)
      pabort("can't send spi message");

   if (rx) {
      printf("  RX: ");
      print_hex(rx, len);
   }
}

unsigned read_devicecode(int fd)
{
   int ret;
   uint8_t regtxbuf[16] = { SPI_START | SPI_WR | SPI_INDEX, 0, 0, };
   uint8_t txbuf[16] = { SPI_START | SPI_RD | SPI_DATA, 0, 0, 0, };
   uint8_t rxbuf[16] = {0, };

   /* Set index register 0x0000 */
   transfer(fd, regtxbuf, NULL, 3);

   /* Read register contents */
   transfer(fd, txbuf, rxbuf, 16);

   return (rxbuf[2] << 8) | rxbuf[3];
}



/*
 * 
 * Kernel gpiolib like functions 
 *
 */

//
//  How to access GPIO registers from C-code on the Raspberry-Pi
//  Example program
//  15-January-2012
//  Dom and Gert
//  Revised: 15-Feb-2013

#define PAGE_SIZE (4*1024)
#define BLOCK_SIZE (4*1024)

#define BCM2708_PERI_BASE        0x20000000
#define GPIO_BASE                (BCM2708_PERI_BASE + 0x200000) /* GPIO controller */

// GPIO setup macros. Always use INP_GPIO(x) before using OUT_GPIO(x) or SET_GPIO_ALT(x,y)
#define INP_GPIO(g) *(gpiop+((g)/10)) &= ~(7<<(((g)%10)*3))
#define OUT_GPIO(g) *(gpiop+((g)/10)) |=  (1<<(((g)%10)*3))
#define SET_GPIO_ALT(g,a) *(gpiop+(((g)/10))) |= (((a)<=3?(a)+4:(a)==4?3:2)<<(((g)%10)*3))

#define GPIO_SET *(gpiop+7)  // sets   bits which are 1 ignores bits which are 0
#define GPIO_CLR *(gpiop+10) // clears bits which are 1 ignores bits which are 0

// I/O access
volatile unsigned *gpiop;

unsigned gpio_initialized = 0;
void setup_io();

int gpio_request_one(unsigned gpio, unsigned long flags, const char *label)
{
   int err;

   if (!gpio_initialized)
      gpio_initialized = 1;
      setup_io();

   if (flags & GPIOF_DIR_IN)
      err = gpio_direction_input(gpio);
   else
      err = gpio_direction_output(gpio,
            (flags & GPIOF_INIT_HIGH) ? 1 : 0);

   if (err)
      goto free_gpio;

   return 0;

 free_gpio:
   gpio_free(gpio);
   return err;
}

void gpio_free(unsigned gpio)
{
   gpio_direction_input(gpio);
}

int gpio_direction_input(unsigned gpio)
{
   INP_GPIO(gpio);
   return 0;
}

int gpio_direction_output(unsigned gpio, int value)
{
   INP_GPIO(gpio);
   OUT_GPIO(gpio);
   gpio_set_value(gpio, value);
   return 0;
}

void gpio_set_value(unsigned gpio, int value)
{
   if (value)
      GPIO_SET = 1<<gpio;
   else
      GPIO_CLR = 1<<gpio;
}


//
// Set up a memory region to access GPIO
//
void setup_io()
{
   int  mem_fd;
   void *gpio_map;

   /* open /dev/mem */
   if ((mem_fd = open("/dev/mem", O_RDWR|O_SYNC) ) < 0) {
      printf("can't open /dev/mem \n");
      exit(-1);
   }

   /* mmap GPIO */
   gpio_map = mmap(
      NULL,             //Any adddress in our space will do
      BLOCK_SIZE,       //Map length
      PROT_READ|PROT_WRITE,// Enable reading & writting to mapped memory
      MAP_SHARED,       //Shared with other processes
      mem_fd,           //File to map
      GPIO_BASE         //Offset to GPIO peripheral
   );

   close(mem_fd); //No need to keep mem_fd open after mmap

   if (gpio_map == MAP_FAILED) {
      printf("mmap error %d\n", (int)gpio_map);//errno also set!
      exit(-1);
   }

   // Always use volatile pointer!
   gpiop = (volatile unsigned *)gpio_map;


} // setup_io

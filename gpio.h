#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <mach/platform.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/init.h>
#include <linux/slab.h>//allocation functions
#include <linux/spinlock.h>
#include <linux/gpio.h>
#include <linux/fs.h>//alloc and free chrdev region
#define GPIO_MASK(g) *(addr + ((g) / 10)) &= ~(7 << (((g) % 10)*3))
#define GPIO_SET_FUNC(g, f) \
	({ GPIO_MASK(g); \
	*(addr + ((g) / 10)) |= ((f) << (((g) % 10)*3)); })
#define GPIO_GET_FUNC(g) \
	(*(addr + ((g) / 10)) >> (((g) % 10) * 3)) & 7
#define GPIO_SET(g) *(addr + 7) = 1 << (g)
#define GPIO_CLR(g) *(addr + 10) = 1 << (g)
#define BUF_SIZE 512
#define CONDITION(i) \
	(i != 0 && i !=1 && i !=3 && i !=5 && i != 8 \
	&& i !=13 && i !=16 && i !=19 && i !=24 && i !=26 \
	&& i !=27 && i != 29 && i !=33 && i != 38 && i != 30)
#define DEVICE_NAME "raspi-gpio"
#define MAX_GPIO_NUMBER 40
#define GPIO_NUMBER 28
#define IN 0
#define OUT 1
dev_t first;
enum state{low, high};
struct class * gpio_class;
struct gpio_dev{
	struct cdev cdev;
	enum state state;
	int in_out;
	int gpio_pin;
};
struct gpio_dev * gpio_dev[GPIO_NUMBER];
static int gpio_open(struct inode *i, struct file *f);
static int gpio_close(struct inode *i, struct file *f);
static ssize_t gpio_read(struct file *f, char __user *buf, size_t len, loff_t *off);
static ssize_t gpio_write(struct file *f, const char __user *buf, size_t len,loff_t *off);
struct file_operations gpio_fops = {
	.read = gpio_read,
	.owner = THIS_MODULE,
	.write = gpio_write,
	.open = gpio_open,
	.release = gpio_close
};
uint32_t * gpio_register;

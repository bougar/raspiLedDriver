#include "gpio.h"
static void SetGPIOFunction(int GPIO, int functionCode, uint32_t * addr)
{
	GPIO_SET_FUNC(GPIO,functionCode);
}

static void SetGPIOOutputValue(int GPIO, enum state state,uint32_t *  addr)
{
    if (state == high)
	{
		GPIO_SET(GPIO);
	}
    else
	{
		GPIO_CLR(GPIO);
	}
}

static ssize_t gpio_read(struct file *f, char __user *buf, size_t len, loff_t *off){
	struct gpio_dev * dev = f->private_data; 
	if (dev->state == low)
	{
		len = 3;
    	if (copy_to_user(buf, "Low", len))
		{
		     return -EFAULT;
		}
	}
	else
	{
		len=4;
		if (copy_to_user(buf, "High", len))
		{
			return -EFAULT;
		}
	}
	if (*off==0)
	{
		*off+=len;
		return len;
	}else
		return 0;
}

static ssize_t gpio_write(struct file *f, const char __user *buf, size_t len,loff_t *off){
	char kbuf[BUF_SIZE];
	struct gpio_dev * dev = f->private_data;
	len = len < BUF_SIZE ? len : BUF_SIZE-1;
	if(copy_from_user(kbuf, buf, len) != 0)
		return -EFAULT;
	kbuf[len]='\0';
	if ( dev->in_out == OUT ){
		printk(KERN_INFO "TRY\n");
		if (strcmp(kbuf, "high") == 0){
			dev->state=high;
			SetGPIOOutputValue(dev->gpio_pin,high,gpio_register);
			*off+=len;	
			return len;
		}else if (strcmp(kbuf, "low") == 0){   
			dev->state=low;
			SetGPIOOutputValue(dev->gpio_pin,low,gpio_register);   
			*off+=len;
			return len;
		}
	}
  	if (strcmp(kbuf, "out") == 0){
		dev->in_out = OUT;
		SetGPIOFunction(dev->gpio_pin,OUT,gpio_register);
		*off+=len;
		return len;
	}else if (strcmp(kbuf, "in") == 0){
		dev->in_out = IN;
		dev->state = low;
		SetGPIOOutputValue(dev->gpio_pin,low,gpio_register);
		SetGPIOFunction(dev->gpio_pin,IN,gpio_register);
		*off+=len;
		return len;
	}
	printk(KERN_INFO "Wrong option in gpio write\n");
	return -EINVAL;		
	
}
static irqreturn_t r_irq_handler(int irq, void *dev_id, struct pt_regs *regs) {
	printk(KERN_INFO "INTERRUPT IS HERE\n");
	return IRQ_HANDLED;
}

static int gpio_open(struct inode *i, struct file *f){
	struct gpio_dev * dev;
	int irq;
	dev = container_of(i->i_cdev,struct gpio_dev,cdev);
	f->private_data=dev;
	if ( dev->in_out == IN ){
		if ( (irq=gpio_to_irq(dev->gpio_pin)) < 0)
			printk(KERN_INFO "Error in gpio_to_irq\n");
		if ( (request_irq ( irq, (irq_handler_t) r_irq_handler, IRQF_TRIGGER_RISING,"gpioi",dev)) )
			printk(KERN_INFO "Error requesting irq\n");
	}
	return 0;
}
static int gpio_close(struct inode *i, struct file *f){
	return 0;
}

static void  __exit gpio_exit(void)
{
	int i, index = 0;
	for (i=0; i < GPIO_NUMBER; i++)
	{
			device_destroy(gpio_class,MKDEV(MAJOR(first), MINOR(first)+i));
			cdev_del(&gpio_dev[index]->cdev);
			index++;
			SetGPIOOutputValue(i, low, gpio_register);
			kfree(gpio_dev[i]);
	}
	class_destroy(gpio_class);
	unregister_chrdev_region(first,GPIO_NUMBER);
}

static int __init gpio_init(void)
{
	int ret;
	int i,index = 0;
	gpio_register = (uint32_t *)__io_address(GPIO_BASE);
	if ( alloc_chrdev_region(&first,0,GPIO_NUMBER,DEVICE_NAME) < 0 )
	{
		printk(KERN_DEBUG "Cannot register device\n");
		return -EINVAL;
	}
	if ( ! (gpio_class = class_create ( THIS_MODULE, DEVICE_NAME)) )
	{
		printk(KERN_DEBUG "Cannot create a class");
		unregister_chrdev_region(first,GPIO_NUMBER);
		return -EINVAL;
	}
	for (i=0; i < GPIO_NUMBER; i++)
	{
			gpio_dev[index]=kmalloc(sizeof(struct gpio_dev), GFP_KERNEL);
			cdev_init(&gpio_dev[index]->cdev,&gpio_fops);
			ret=cdev_add(&gpio_dev[index]->cdev,(first + i), 1);
			if ( ret )
				printk(	KERN_INFO "Error %d\n",ret);
			SetGPIOFunction(i,0b001,gpio_register);
			SetGPIOOutputValue(i, low, gpio_register);
			gpio_dev[index]->state=low;
			gpio_dev[index]->in_out = OUT;
			gpio_dev[index]->gpio_pin=i;
			gpio_dev[index]->cdev.owner = THIS_MODULE;
			if (!(device_create ( gpio_class, NULL, MKDEV(MAJOR(first), MINOR(first)+i), NULL,"raspiGpio%d",i) ) )
			{
				printk( KERN_DEBUG "Cannot create device for gpio %d\n",i);
			}	
			index++;
	}
	return 0;
}
module_init(gpio_init);
module_exit(gpio_exit);
MODULE_LICENSE("GPL");

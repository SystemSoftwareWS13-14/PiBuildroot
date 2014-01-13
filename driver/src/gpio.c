#include <linux/version.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <asm/barrier.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/ioport.h>
#include <linux/slab.h>

#define DRIVER_NAME "gpio_syso"
#define DRIVER_FILE_NAME "mygpio"
#define CLASS_NAME "myDriver_class"
#define START_MINOR 0
#define MINOR_COUNT 1

#define REG_GPIO_COUNT 10
#define REG_GPIO_SIZE 3

#define IN_GPIO 17
#define OUT_GPIO 25

#define GPFSEL(pin) (u32*)(gpio + (pin / 10))
#define GPFSET(pin) (u32*)(gpio + 7 + (pin / 32))
#define GPFCLR(pin) (u32*)(gpio + 10 + (pin / 32)) 
#define GPFLEV(pin) (u32*)(gpio + 13 + (pin / 32))

#define BCM2708_PERI_BASE 0x20000000
#define GPIO_BASE (BCM2708_PERI_BASE + 0x200000)
#define MEM_REG_LEN 4096

//static void *mem;
static volatile unsigned *gpio;

static struct file_operations fops;
static dev_t treiber_dev;
static struct cdev *treiber_object;
static struct class *treiber_class;

//prototypes
static int register_driver(void);
static void unregister_driver(void);

static int open(struct inode *inode, struct file *filp);
static int close(struct inode *inode, struct file *filp);
static ssize_t read(struct file *filp, char *buff, size_t count, loff_t *offp);
static ssize_t write(struct file *filp, const char *buf, size_t count, loff_t *offp);

static int is_buff_zero(const char *buff, int count);
static int is_buff_one(const char *buff, int count);
static int setup_gpio(void);

static void gpio_set_as_output(int pin);
static void gpio_set_as_input(int pin);

// File operations
static struct file_operations fops = {
	.write = write,
	.read = read,
	.open = open,
	.release = close
};

static int __init mod_init(void)
{
	printk("mod_init called\n");

	if(register_driver())
		return -EIO;
	if(setup_gpio())
		return -EIO;
	return 0;
	
}

static int register_driver(void)
{
	// Reserviert Geraetenummer fuer Treiber und Anzahl minornr
	// dev_t haelt Geraetenummer
	if(alloc_chrdev_region(&treiber_dev, START_MINOR, MINOR_COUNT, DRIVER_NAME) < 0)
		return -EIO;
	
	// Haelt ein Kernelobjekt und verweist auf Modul/Treiber
	// verbindet Kernelobjekt und Treiber
	treiber_object = cdev_alloc();
	if( treiber_object == NULL )
		goto free_treiber_dev;

	treiber_object->owner = THIS_MODULE;
	treiber_object->ops = &fops;

	// Fuegt einen Treiber hinzu / registriert ihn in fops
 	if(cdev_add(treiber_object, treiber_dev, MINOR_COUNT))
		goto free_object;

	// An Geraeteklasse anmelden -> meldet sich im Geraetemodell in sysfs an
	// so wird dem hotplugmanager mitgeteilt, dass Geraetedatei angelegt
	// werden soll
	treiber_class = class_create( THIS_MODULE, CLASS_NAME);
	device_create( treiber_class, NULL, treiber_dev, NULL,
		       "%s", DRIVER_FILE_NAME);

	printk("Major: %d\n", MAJOR(treiber_dev));
	pr_info("Registered driver\n");
	return 0;

free_object:
	kobject_put( &treiber_object->kobj );
free_treiber_dev:
	unregister_chrdev_region( treiber_dev, MINOR_COUNT );
	return -EIO;
}

static void __exit mod_exit(void)
{
	printk("mod_exit called\n");

	unregister_driver();
	release_mem_region(GPIO_BASE, MEM_REG_LEN);

	pr_info("unregistered driver\n");
}

static void unregister_driver(void)
{
	device_destroy(treiber_class, treiber_dev);
	class_destroy(treiber_class);
	cdev_del(treiber_object);
	unregister_chrdev_region(treiber_dev, MINOR_COUNT);
}

static int open(struct inode *inode, struct file *filp)
{
	//set in
	printk(KERN_DEBUG "opening 'in'\n");
	gpio_set_as_input(IN_GPIO);

	//set out
	printk(KERN_DEBUG "opening 'out'\n");
	gpio_set_as_output(OUT_GPIO);

	filp->private_data = kmalloc(sizeof(int), GFP_KERNEL);
	*((int*)filp->private_data) = 0;

	return 0;
}

static int close(struct inode *inode, struct file *filp)
{
	kfree(filp->private_data);
	return 0;
}

static ssize_t read(struct file *filp, char *buff, size_t count, loff_t *offp)
{
	char result[10];
	int not_copied, to_copy;
	u32 value, bitmask;
	u32 *ptr;
	
	if(  *((int*) filp->private_data) )
		return 0;

	//read from ingpio
	ptr = GPFLEV(IN_GPIO);
	
	value = readl(ptr) ;
	printk(KERN_DEBUG "gpio read -> %.8x\n", value);
	rmb();
	bitmask = 0x1 << IN_GPIO;
	value = value & bitmask;
	printk(KERN_DEBUG "gpio read -> %.8x\n", value);

	if(value != bitmask)
		strcpy(result,"0\n\0");
	else
		strcpy(result,"1\n\0");

	to_copy = min( count , strlen(result) + 1);
	not_copied = copy_to_user(buff, result, to_copy);

	*((int*)filp->private_data) = 1;	

	return to_copy - not_copied;
}

static ssize_t write(struct file *filp, const char *buf, size_t count, loff_t *offp)
{
	u32 *ptr;
	u32 value, bitmask;
	char buff[count];
	int not_copied;

	not_copied = copy_from_user(buff, buf, count);

	printk(KERN_DEBUG "write: %s\n", buff);

	if(is_buff_zero(buff, count))
	{
		//put zero on leitung
		printk(KERN_DEBUG "write is zero\n");
		ptr = GPFCLR(OUT_GPIO);
	}
	else if(is_buff_one(buff, count))
	{
		//put one on leitung
		printk(KERN_DEBUG "write is one\n");
		ptr = GPFSET(OUT_GPIO);
	}
	else
		return -EINVAL;
	
	rmb();
	bitmask = 0x1 << OUT_GPIO;
	printk(KERN_DEBUG "write bitmask (set) %.8x\n", bitmask);
	value = bitmask;
	wmb();
	writel(value, ptr);

	return count;
}

static int is_buff_zero(const char *buff, int count)
{
	if(count <= 0)
		return 0;

	return buff[0] == '0';
}

static int is_buff_one(const char *buff, int count)
{
	if(count <= 0)
		return 0;
	
	return buff[0] == '1';
}

static int setup_gpio(void)
{
	/*mem = request_mem_region(GPIO_BASE, MEM_REG_LEN, "mygpio");
	if(mem == NULL)
	{
		printk(KERN_INFO "mem region not got\n");
		return -EBUSY;
	}*/

   	gpio = ioremap(GPIO_BASE, MEM_REG_LEN);
	if(gpio == NULL)
	{
		printk(KERN_INFO "gpio not got\n");
		release_mem_region(GPIO_BASE, MEM_REG_LEN);
		return -EBUSY;
	}
	return 0;
}

static void gpio_set_as_input(int pin)
{
	u32 *ptr;
	u32 value, bitmask;

	ptr = GPFSEL(pin);
	value = readl(ptr);
	rmb();

	//shift at right position
	bitmask = 0x7 << ((pin % REG_GPIO_COUNT) * REG_GPIO_SIZE);
	//invert
	bitmask ^= 0xFFFFFFFF;

 	printk(KERN_DEBUG "in bitmask %.8x\n", bitmask);

	value = value & bitmask;
	wmb();
	writel(value, ptr);
}

static void gpio_set_as_output(int pin)
{
	u32 *ptr;
	u32 value, bitmask;

	gpio_set_as_input(pin);

	ptr = GPFSEL(pin);
	value = readl(ptr);
	rmb();
	
	bitmask = 0x1 << ((pin % REG_GPIO_COUNT) * REG_GPIO_SIZE);

	printk(KERN_DEBUG "out bitmask %.8x\n", bitmask);
	wmb();
	writel(value | bitmask, ptr);
}

module_init(mod_init);
module_exit(mod_exit);

MODULE_LICENSE("GPL");

MODULE_AUTHOR("JeFa");
MODULE_DESCRIPTION("Modul Template");
MODULE_SUPPORTED_DEVICE("None");

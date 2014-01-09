#include <linux/version.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <asm/barrier.h>
#include <asm/io.h>
#include <asm/uaccess.h>

#define DRIVER_NAME "gpio_syso"
#define DRIVER_FILE_NAME "mygpio"
#define CLASS_NAME "myDriver_class"
#define START_MINOR 0
#define MINOR_COUNT 1

#define GPIO_REG_SIZE 3

#define IN_GPIO 17
#define IN_CLR_DIR_BIT_MASK 0xFF1FFFFF
#define IN_BIT_MASK 0xFFFEFFFF


#define OUT_GPIO 25
//direction bitmasks
#define OUT_CLR_DIR_BIT_MASK 0xFFF1FFFF
#define OUT_SET_DIR_BIT_MASK 0x00020000
//value bitmasks
#define OUT_SET_VAL_BIT_MASK 0x01000000
#define OUT_CLR_VAL_BIT_MASK 0xFEFFFFFF

#define GPFSEL1 0xF2200004
#define GPFSEL2 0xF2200008
#define GPFSET0 0xF220001c
#define GPFCLR0 0xF2200028

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
static ssize_t write(struct file *filp, const char *buff, size_t count, loff_t *offp);

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
	u32 *ptr;
	u32 old_value; 
 
	//set in
	ptr = (u32 *)GPFSEL1;
	old_value = readl(ptr);
	rmb();
	old_value = old_value & IN_CLR_DIR_BIT_MASK;
	wmb();
	writel(old_value, ptr);

	//set out
	ptr = (u32 *)GPFSEL2;
	old_value = readl(ptr);
	rmb(); 
	old_value = old_value & OUT_CLR_DIR_BIT_MASK;
	wmb();
	writel(old_value | OUT_SET_DIR_BIT_MASK, ptr);

	return 0;
}

static int close(struct inode *inode, struct file *filp)
{
	return 0;
}

static ssize_t read(struct file *filp, char *buff, size_t count, loff_t *offp)
{
	char result[2];
	int not_copied, to_copy;
	u32 value;

	u32 *ptr = (u32 *) GPFSET0;
	value = *ptr & IN_BIT_MASK;
	
	if(value == 0)
		strcpy(result,"0");
	else if(value == 1)
		strcpy(result,"1");
	else
		return -EAGAIN;

	to_copy = min((int) count , 2);
	not_copied = copy_to_user(buff, result, to_copy);
	
	return to_copy - not_copied;
}

static ssize_t write(struct file *filp, const char *buff, size_t count, loff_t *offp)
{
	u32 *ptr_clear;
	u32 *ptr_set;

	if(count < 2)
		return -EAGAIN;	

	if(strcmp(buff, "0") == 0)
	{
		//put zero on leitung
		ptr_set = (u32 *) GPFCLR0;
		ptr_clear = (u32 *) GPFSET0;
	}
	else if(strcmp(buff, "1") == 0)
	{
		//put one on leitung
		ptr_set = (u32 *) GPFSET0;
		ptr_clear = (u32 *) GPFCLR0;
	}
	else
		return -EAGAIN;
	
	writel(*ptr_set | OUT_SET_VAL_BIT_MASK, ptr_set);
	writel(*ptr_clear & OUT_CLR_VAL_BIT_MASK, ptr_clear);

	return 0;
}

module_init(mod_init);
module_exit(mod_exit);

MODULE_LICENSE("GPL");

MODULE_AUTHOR("JeFa");
MODULE_DESCRIPTION("Modul Template");
MODULE_SUPPORTED_DEVICE("None");

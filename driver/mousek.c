#include <linux/module.h>			
#include <linux/usb/input.h>			
#include <linux/poll.h>

/*
 * Version Information
 */
#define DRIVER_AUTHOR "Nguyen Van Trung"
#define DRIVER_DESC "Mouse driver"
#define DRIVER_LICENSE "GPL"



#define DEVICE_NAME "mousek"

int Major;

static struct input_dev *input_dev;

/* write accepts data and converts it to mouse movement
 * Called when a process writes to dev file: echo "hi" > /dev/hello
 */
ssize_t mousek_write(struct file *filp, const char *buf, size_t count,
		    loff_t *offp)
{

    static char localbuf[1];

    if (count >1) count=1;
    copy_from_user(localbuf, buf, count);

    struct input_dev *dev = input_dev;
    
    switch (localbuf[0]) 
    {
	case 'l':
		{
			input_report_rel(dev, REL_X, -1);
			input_sync(dev);
			break;
		}
	case 'r':
		{
			input_report_rel(dev, REL_X, 1);
			input_sync(dev);
			break;
		}
	case 'u':
		{
			input_report_rel(dev, REL_Y, -1);
			input_sync(dev);
			break;
		}
	case 'd':
		{
			input_report_rel(dev, REL_Y, 1);
			input_sync(dev);
			break;
		}
	case 'q':
		{
			input_report_key(dev, BTN_LEFT, 1);
			input_sync(dev);
			input_report_key(dev, BTN_LEFT, 0);
			input_sync(dev);
			break;
		}
    }
	return count;
}


struct file_operations mousek_fops = {
	write:    mousek_write,
};

/*
 * This function is called when the module is loaded
 */
static int __init init_mousek(void)
{
    Major = register_chrdev(0, DEVICE_NAME, &mousek_fops);
    input_dev = input_allocate_device();

    input_dev->evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_REL);
    input_dev->keybit[BIT_WORD(BTN_MOUSE)] = BIT_MASK(BTN_LEFT);
    input_dev->relbit[0] = BIT_MASK(REL_X) | BIT_MASK(REL_Y);

    input_dev->name = DEVICE_NAME;	
    input_register_device(input_dev);
	
    printk(KERN_ALERT "Installed.");
    printk(KERN_INFO "'mknod /dev/%s c %d 0'.\n", DEVICE_NAME, Major);
    return 0;
}

/*
 * This function is called when the module is unloaded
 */
static void __exit exit_mousek(void)
{
    input_unregister_device(input_dev);
    unregister_chrdev(Major, DEVICE_NAME);    
    printk(KERN_ALERT "Uninstalled.");
}

module_init(init_mousek);
module_exit(exit_mousek);

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE(DRIVER_LICENSE);


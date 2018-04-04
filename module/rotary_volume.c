#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/slab.h>
#include <linux/module.h>
#include <linux/input.h>
#include <linux/init.h>
#include <linux/device.h>

MODULE_AUTHOR("James Kent");
MODULE_DESCRIPTION("rotary encoder as volume control module");
MODULE_LICENSE("GPL");

static char *devicename = "rotary@4";
module_param(devicename, charp, 0);
MODULE_PARM_DESC(devicename, "name of rotary input device");

static int reltype = REL_MISC; // 0x09
module_param(reltype, int, 0);
MODULE_PARM_DESC(reltype, "type of relative event to listen for");

static int count_per_press = 10;
module_param(count_per_press, int, 0);
MODULE_PARM_DESC(count_per_press, "event count before a press is generated");

static struct input_dev *button_dev;

static void send_key(int key) {
	input_report_key(button_dev, key, 1);
	input_sync(button_dev);
	input_report_key(button_dev, key, 0);
	input_sync(button_dev);
}

int count = 0;

static void rotary_event(struct input_handle *handle, unsigned int type, unsigned int code, int value) {
	printk(KERN_DEBUG pr_fmt("Event. Dev: %s, Type: %d, Code: %d, Value: %d\n"), dev_name(&handle->dev->dev), type, code, value);
	if (type == EV_REL) {
		if (code == reltype) {
			int i;
			int inc = (value > 0) ? 1 : -1;
			if ((inc > 0 && count < 0) || (inc < 0 && count > 0)) { // if change of direction reset count
				count = 0;
			}
			for (i=0; i!=value; i+=inc) {
				count += inc;
				if (abs(count) >= count_per_press) {
					send_key( (inc > 0) ? KEY_VOLUMEUP : KEY_VOLUMEDOWN);
					count = 0;
				}
			}
		}
	}
}

static int rotary_connect(struct input_handler *handler, struct input_dev *dev, const struct input_device_id *id) {
	struct input_handle *handle;
	int error;

	handle = kzalloc(sizeof(struct input_handle), GFP_KERNEL);
	if (!handle)
		return -ENOMEM;

	handle->dev = dev;
	handle->handler = handler;
	handle->name = "gpio_volume";

	error = input_register_handle(handle);
	if (error)
		goto err_free_handle;

	error = input_open_device(handle);
	if (error)
		goto err_unregister_handle;

	printk(KERN_DEBUG pr_fmt("Connected device: %s (%s at %s)\n"),
	       dev_name(&dev->dev),
	       dev->name ?: "unknown",
	       dev->phys ?: "unknown");

	return 0;

 err_unregister_handle:
	input_unregister_handle(handle);
 err_free_handle:
	kfree(handle);
	return error;
}

bool startsWith(const char *pre, const char *str) {
    size_t lenpre = strlen(pre),
           lenstr = strlen(str);
    return lenstr < lenpre ? false : strncmp(pre, str, lenpre) == 0;
}

// keep record of match and remove record is disconnect
bool matched = false;

static bool rotary_match(struct input_handler *handler, struct input_dev *dev) {
	if (matched)
		return false;
	matched = startsWith(devicename, dev->name);
	return matched;
}

static void rotary_disconnect(struct input_handle *handle) {
	printk(KERN_DEBUG pr_fmt("Disconnected device: %s\n"), dev_name(&handle->dev->dev));

	input_close_device(handle);
	input_unregister_handle(handle);
	kfree(handle);
	matched = false;
}

static const struct input_device_id rotary_ids[] = {
	{ .driver_info = 1 },	/* Matches all devices */
	{ },			/* Terminating zero entry */
};

MODULE_DEVICE_TABLE(input, rotary_ids);

static struct input_handler rotary_handler = {
	.event =	rotary_event,
	.match =	rotary_match,
	.connect =	rotary_connect,
	.disconnect =	rotary_disconnect,
	.name =		"rotary_volume",
	.id_table =	rotary_ids,
};

static int __init button_init(void) {
	int error;
	int i;
	
	button_dev = input_allocate_device();
	if (!button_dev) {
		printk(KERN_ERR pr_fmt("Not enough memory\n"));
		error = -ENOMEM;
		return error;
	}

	button_dev->name = "Rotary Encoder Volume";
	button_dev->evbit[0] = BIT_MASK(EV_KEY);// | BIT_MASK(EV_REP);
	set_bit(KEY_VOLUMEDOWN, button_dev->keybit);
	set_bit(KEY_VOLUMEUP, button_dev->keybit);

	for (i=KEY_ESC; i<=KEY_KPDOT; i++) { // add a load of extra keys
		set_bit(i, button_dev->keybit);
	}

	error = input_register_device(button_dev);
	if (error) {
		printk(KERN_ERR pr_fmt("Failed to register device\n"));
		goto err_free_dev;
	}
	return 0;
 err_free_dev:
	input_free_device(button_dev);
	return error;
}

static int __init rotary_volume_init(void) {
	int error = button_init();
	if (count_per_press < 1) // sanitise input
		count_per_press = 1;
	if (error == 0) {
		if (input_register_handler(&rotary_handler)==0) {
			printk(KERN_INFO pr_fmt("loaded.\n"));
			return 0;
		} else {
			input_unregister_device(button_dev);
			input_free_device(button_dev);
		}
	}
	return error;
}

static void __exit rotary_volume_exit(void) {
	input_unregister_device(button_dev);
	input_free_device(button_dev);
	input_unregister_handler(&rotary_handler);
}

module_init(rotary_volume_init);
module_exit(rotary_volume_exit);

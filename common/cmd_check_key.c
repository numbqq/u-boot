#include <common.h>
#include <command.h>
#include <errno.h>
#include <dm.h>
#include <asm/gpio.h>

static void hdelay(int us)
{
	int i;
	while (us--) {
		i = 10000;
		while (i--) ;
	}
}

#define msleep(i)   hdelay(i)

static int do_check_key(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unsigned int gpio;
	ulong value;
	const char *str_gpio = NULL;
	int ret;

	if (argc != 2)
show_usage:
		return CMD_RET_USAGE;

	str_gpio = argv[1];

	if (!str_gpio)
		goto show_usage;

#if defined(CONFIG_DM_GPIO)
	/*
	 * TODO(sjg@chromium.org): For now we must fit into the existing GPIO
	 * framework, so we look up the name here and convert it to a GPIO number.
	 * Once all GPIO drivers are converted to driver model, we can change the
	 * code here to use the GPIO uclass interface instead of the numbered
	 * GPIO compatibility layer.
	 */
	ret = gpio_lookup_name(str_gpio, NULL, NULL, &gpio);
	if (ret)
		return cmd_process_error(cmdtp, ret);
#else
	/* turn the gpio name into a gpio number */
	gpio = name_to_gpio(str_gpio);
	if (gpio < 0)
		goto show_usage;
#endif
	/* grab the pin before we tweak it */
	ret = gpio_request(gpio, "check_gpio");
	if (ret && ret != -EBUSY) {
		printf("gpio: requesting pin %u failed\n", gpio);
		return -1;
	}

	gpio_direction_input(gpio);

	while (1) {
		value = gpio_get_value(gpio);
		msleep(100);
		value = gpio_get_value(gpio);
		if (value == 0) {
			break;
		}
	}

	printf("%s pressed!\n", str_gpio);

	gpio_free(gpio);

	return value;
}
U_BOOT_CMD(check_key, 2, 0, do_check_key,
	   "wait until specified key pressed",
	   "<pin>\n");

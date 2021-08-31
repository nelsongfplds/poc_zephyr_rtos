#include <device.h>
#include <sensor.h>
#include <zephyr.h>
#include <devicetree.h>
#include <drivers/gpio.h>

#define SLEEP_TIME_MS   5000

/* The devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_ALIAS(led0)

#if DT_NODE_HAS_STATUS(LED0_NODE, okay)
#define LED0	DT_GPIO_LABEL(LED0_NODE, gpios)
#define PIN	DT_GPIO_PIN(LED0_NODE, gpios)
#define FLAGS	DT_GPIO_FLAGS(LED0_NODE, gpios)
#else
/* A build error here means your board isn't set up to blink an LED. */
#error "Unsupported board: led0 devicetree alias is not defined"
#define LED0	""
#define PIN	0
#define FLAGS	0
#endif

/* The devicetree node identifier for the "shtc3" alias. */
#define SHTC3_NODE DT_ALIAS(shtc3)

#if !DT_NODE_HAS_STATUS(SHTC3_NODE, okay)
#error "Unsupported board: shtc3 devicetree alias is not defined"
#endif

/* The devicetree node identifier for the "quectel_bg9x". */
#define BG96_NODE DT_NODELABEL(quectel_bg9x)

#if !DT_NODE_HAS_STATUS(BG96_NODE, okay)
#error "Unsupported board: bg96 devicetree alias is not defined"
#endif

static const struct device *init_led() {
	int ret;
	const struct device *led_dev = device_get_binding(LED0);

	if (led_dev == NULL) {
		printk("LED0 not found\n");
		return NULL;
	}

	ret = gpio_pin_configure(led_dev, PIN, GPIO_OUTPUT_ACTIVE | FLAGS);
	if (ret < 0) {
		printk("LED0 not configured\n");
		return NULL;
	}

	return led_dev;
}

static const struct device *init_shtc3() {
	const struct device *shtc3_dev = device_get_binding(DT_LABEL(SHTC3_NODE));

	if (shtc3_dev == NULL) {
		printk("SHTC3 not found\n");
		return NULL;
	}

	return shtc3_dev;
}

static const struct device *init_bg96() {
	const struct device *bg96_dev = device_get_binding(DT_LABEL(BG96_NODE));

	if (bg96_dev == NULL) {
		printk("BG96 not found\n");
		return NULL;
	}

	return bg96_dev;
}

static void shtc3_sensor_read(const struct device *shtc3_dev) {
	int ret;
	struct sensor_value shtc3_sv;

	ret = sensor_sample_fetch(shtc3_dev);
	if (ret != 0) {
		printk("sensor_sample_fetch error: %d\n", ret);
	}

	ret = sensor_channel_get(shtc3_dev, SENSOR_CHAN_AMBIENT_TEMP, &shtc3_sv);
	if (ret != 0) {
		printk("sensor_channel_get error: %d\n", ret);
	}

	/* printk("Temperature: %f C\n", sensor_value_to_double(&shtc3_sv)); */
	printk("Temperature: %d.%06d C\n", shtc3_sv.val1, shtc3_sv.val2);

	ret = sensor_channel_get(shtc3_dev, SENSOR_CHAN_HUMIDITY, &shtc3_sv);
	if (ret != 0) {
		printk("sensor_channel_get error: %d\n", ret);
	}

	/* printk("Humidity: %f C\n", sensor_value_to_double(&shtc3_sv)); */
	printk("Humidity: %d.%06d C\n", shtc3_sv.val1, shtc3_sv.val2);
}

void main(void)
{
	const struct device *led_dev = init_led();
	const struct device *bg96_dev = init_bg96();
	const struct device *shtc3_dev = init_shtc3();

	if (led_dev == NULL) {
		printk("Led not found, stopping...\n");
		return;
	}

	if (shtc3_dev == NULL) {
		printk("SHTC3 sensor not found, stopping...\n");
		return;
	}

	if (bg96_dev == NULL) {
		printk("BG96 modem not found, stopping...\n");
		while (true) {
			printk("BG96 modem not found, stopping...\n");
			printk("prop: %s\n", DT_PROP(DT_NODELABEL(quectel_bg9x), label));
			k_msleep(2000);
		}
		return;
	}

	printk("Devices initialized!\n");
	gpio_pin_set(led_dev, PIN, 0);

	printk("Led set, begin main loop");
	while (true) {
		shtc3_sensor_read(shtc3_dev);

		k_msleep(SLEEP_TIME_MS);
	}
}

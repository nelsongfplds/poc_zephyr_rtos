/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/gpio.h>
/* #include <shtcx.h> */
#include <sensor.h>

/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS   10000

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

#define SHTC3_NODE DT_ALIAS(shtc3)
/* #define SHTC3_NODE DT_PATH(soc, i2c_40004000, opt3001_44) */

void main(void)
{
	const struct device *dev;
	const struct device *shtc3_dev;
	bool led_is_on = true;
	int ret;

	printk("Main\n");
	dev = device_get_binding(LED0);
	if (dev == NULL) {
		printk("LED0 not found\n");
		return;
	}

	ret = gpio_pin_configure(dev, PIN, GPIO_OUTPUT_ACTIVE | FLAGS);
	if (ret < 0) {
		printk("LED0 not configured\n");
		return;
	}

	printk("Sensor\n");
	struct sensor_value sv;
#if DT_NODE_HAS_STATUS(SHTC3_NODE, okay)
	shtc3_dev = device_get_binding(DT_LABEL(SHTC3_NODE));
	printk("Node is enabled\n");
#else
#error "Node is disabled"
	printk("Node is disabled\n");
#endif
	if (shtc3_dev == NULL) {
		printk("SHTC3 not found\n");
		return;
	}

	ret = sensor_sample_fetch(shtc3_dev);
	if (ret != 0) {
		printk("sensor_sample_fetch error: %d\n", ret);
	}

	ret = sensor_channel_get(shtc3_dev, SENSOR_CHAN_AMBIENT_TEMP, &sv);
	if (ret != 0) {
		printk("sensor_channel_get error: %d\n", ret);
	}

	/* printk("Temperature: %f C\n", sensor_value_to_double(&sv)); */
	printk("Temperature: %d.%06d C\n", sv.val1, sv.val2);

	ret = sensor_channel_get(shtc3_dev, SENSOR_CHAN_HUMIDITY, &sv);
	if (ret != 0) {
		printk("sensor_channel_get error: %d\n", ret);
	}

	printk("Humidity: %f C\n", sensor_value_to_double(&sv));

	while (1) {
		gpio_pin_set(dev, PIN, (int)led_is_on);
		led_is_on = !led_is_on;
		k_msleep(SLEEP_TIME_MS);
	}
}

/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/gpio.h>
#include <shtcx.h>
#include <sensor.h>

/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS   1000

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

void main(void)
{
	const struct device *dev;
	const struct device *shtc3;
	bool led_is_on = true;
	int ret;

	dev = device_get_binding(LED0);
	if (dev == NULL) {
		return;
	}

	ret = gpio_pin_configure(dev, PIN, GPIO_OUTPUT_ACTIVE | FLAGS);
	if (ret < 0) {
		return;
	}

	struct sensor_value sv;
	shtc3 = device_get_binding("SHTC3");
	if (shtc3 == NULL) {
		return;
	}

	ret = sensor_sample_fetch(shtc3);
	if (ret != 0) {
		printk("sensor_sample_fetch error: %d\n", ret);
	}

	ret = sensor_channel_get(shtc3, SENSOR_CHAN_AMBIENT_TEMP, &sv);
	if (ret != 0) {
		printk("sensor_channel_get error: %d\n", ret);
	}

	printk("Temperature: %g C\n", sensor_value_to_double(&sv));

	ret = sensor_channel_get(shtc3, SENSOR_CHAN_HUMIDITY, &sv);
	if (ret != 0) {
		printk("sensor_channel_get error: %d\n", ret);
	}

	printk("Humidity: %g C\n", sensor_value_to_double(&sv));

	while (1) {
		gpio_pin_set(dev, PIN, (int)led_is_on);
		led_is_on = !led_is_on;
		k_msleep(SLEEP_TIME_MS);
	}
}

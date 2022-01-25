#include "sensors.h"

/* Statics */
static const struct device *led_dev;
static const struct device *shtc3_dev;

static bool init_led() {
	int ret;
	led_dev = device_get_binding(LED0);

	if (led_dev == NULL) {
		printk("LED0 not found\n");
		return false;
	}

	ret = gpio_pin_configure(led_dev, PIN, GPIO_OUTPUT_ACTIVE | FLAGS);
	if (ret < 0) {
		printk("LED0 not configured\n");
		return false;
	}
	gpio_pin_set(led_dev, PIN, 0);

	return true;
}

static bool init_shtc3() {
	shtc3_dev = device_get_binding(DT_LABEL(SHTC3_NODE));

	if (shtc3_dev == NULL) {
		printk("SHTC3 not found\n");
		return false;
	}

	return true;
}

/* Methods */
bool init_board_sensors() {
	bool ret;

	ret = init_led();
	if (ret == false) {
		return false;
	}

	ret = init_shtc3();
	if (ret == false) {
		return false;
	}

	return true;
}

bool shtc3_sensor_read(char *temp, char *ur, size_t temp_len, size_t ur_len) {
	int ret;
	struct sensor_value shtc3_sv;

	if (temp_len != TEMP_SENSOR_BUFF_LEN) {
		printk("Temperature buffer with incorrect size: %d\n", temp_len);
		return false;
	}
	if (ur_len != TEMP_SENSOR_BUFF_LEN) {
		printk("Umidity buffer with incorrect size: %d\n", temp_len);
		return false;
	}

	ret = sensor_sample_fetch(shtc3_dev);
	if (ret != 0) {
		printk("sensor_sample_fetch error: %d\n", ret);
		return false;
	}

	ret = sensor_channel_get(shtc3_dev, SENSOR_CHAN_AMBIENT_TEMP, &shtc3_sv);
	if (ret != 0) {
		printk("sensor_channel_get error: %d\n", ret);
		return false;
	}

	snprintk(temp, TEMP_SENSOR_BUFF_LEN, "%d.%d", shtc3_sv.val1, shtc3_sv.val2);

	ret = sensor_channel_get(shtc3_dev, SENSOR_CHAN_HUMIDITY, &shtc3_sv);
	if (ret != 0) {
		printk("sensor_channel_get error: %d\n", ret);
		return false;
	}

	snprintk(ur, TEMP_SENSOR_BUFF_LEN, "%d.%d", shtc3_sv.val1, shtc3_sv.val2);
	return true;
}

int get_batt_reading() {
	int batt_mv = battery_sample();
		if (batt_mv < 0) {
			printk("Failed to read battery voltage: %d\n",
			       batt_mv);
			return -1;
		}

	/* printk("Level: %d mV\n", batt_mv); */

	return batt_mv;
}

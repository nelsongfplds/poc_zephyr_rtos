#include "main.h"

static bool init() {
	if (init_board_sensors() == false) {
		printk("Failed sensor initialization, stopping...\n");
		return false;
	}

	if (init_bg96() == false) {
		printk("Failed BG96 initialization, stopping...\n");
		return false;
	}

	return true;
}

void main(void)
{
	if (!init()) {
		return;
	}
	printk("Devices initialized!\n");

	int batt_lvl;
	char imei[IMEI_SIZE + 1];
	char temp[TEMP_SENSOR_BUFF_LEN];
	char ur[TEMP_SENSOR_BUFF_LEN];

	memset(temp, 0, TEMP_SENSOR_BUFF_LEN);
	memset(ur, 0, TEMP_SENSOR_BUFF_LEN);

	get_imei(imei);
	shtc3_sensor_read(temp, ur);
	batt_lvl = get_batt_reading();

	/* printk("IMEI at MAIN: %s\n", imei); */
	/* printk("TEMP at MAIN: %s\n", temp); */
	/* printk("HUM at MAIN: %s\n", ur); */
	/* printk("VBATT at MAIN: %d\n", batt_lvl); */

	/* char *buffer = set_payload(imei, batt_lvl, temp, ur); */
	/* printk("%s\n", buffer); */

	int ret = server_connect();
	if (ret) {
		printk("Connected\n");
		char *buffer = set_payload(imei, batt_lvl, temp, ur);
		printk("%s\n", buffer);
		send_payload(buffer, strlen(buffer));
	} else {
		printk("Not connected\n");
	}

	printk("Exiting main\n");
}

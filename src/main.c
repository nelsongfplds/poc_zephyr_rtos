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
	/* int batt_mv = battery_sample(); */
	/* while (true) { */
	/* 	if (batt_mv < 0) { */
	/* 		printk("Failed to read battery voltage: %d\n", */
	/* 		       batt_mv); */
	/* 		break; */
	/* 	} */

	/* 	printk("Level: %d mV\n", batt_mv); */

	/* 	k_msleep(SLEEP_TIME_MS); */
	/* } */
	if (!init()) {
		return;
	}
	printk("Devices initialized!\n");
	k_msleep(SLEEP_TIME_MS*2);

	int ret = server_connect();
	if (ret) {
		printk("Connected\n");
		char *buffer = set_payload("abc123456", "9100mV");
		printk("%s\n", buffer);
		printk("AA");
		send_payload(buffer, strlen(buffer));
	} else {
		printk("Not connected\n");
	}

	/* memset(rsp, 0, 100); */
	/* printk("\n\nGet IMEI\n"); */
	/* send_at_command("AT+GSN", strlen("AT+GSN"), rsp); */
	/* 	printk("main, returned: %s\n", rsp); */
	/* k_msleep(10000); */

	printk("Exiting main\n");
}

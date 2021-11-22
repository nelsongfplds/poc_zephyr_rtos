#include "main.h"

void main(void)
{
	if (init_board_sensors() == false) {
		printk("Failed sensor initialization, stopping...\n");
		return;
	}

	if (init_bg96() == false) {
		printk("Failed BG96 initialization, stopping...\n");
		return;
	}

	printk("Devices initialized!\n");

	k_msleep(SLEEP_TIME_MS*2);

	int ret = server_connect();
	if (ret) {
		printk("Connected\n");
	} else {
		printk("Not connected\n");
	}

	/* memset(rsp, 0, 100); */
	/* printk("\n\nGet IMEI\n"); */
	/* send_at_command("AT+GSN", strlen("AT+GSN"), rsp); */
	/* 	printk("main, returned: %s\n", rsp); */
	k_msleep(10000);

	printk("Exiting main\n");
}

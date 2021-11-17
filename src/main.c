#include "main.h"

/* Static variables */
static uint8_t send_buffer[RING_BUFFER_SIZE];

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
	memset(send_buffer, 0, RING_BUFFER_SIZE);
	memcpy(send_buffer, "ATI", 3);

	k_msleep(SLEEP_TIME_MS*2);

	int reps = 0;
	char rsp[100];
	memset(rsp, 0, 100);
	int ret = server_connect();
	if (ret) {
		printk("Connected\n");
	} else {
		printk("Not connected\n");
	}
	/* printk("Begin main loop\n"); */
	/* while (reps <= 1) { */
	/* 	printk("Iteration: %d\n", reps); */
	/* 	shtc3_sensor_read(); */

	/* 	send_at_command(send_buffer, strlen(send_buffer), rsp); */
	/* 	printk("main, returned: %s\n", rsp); */
	/* 	k_msleep(10000); */

	/* 	reps++; */
	/* } */

	/* memset(rsp, 0, 100); */
	/* printk("\n\nGet IMEI\n"); */
	/* send_at_command("AT+GSN", strlen("AT+GSN"), rsp); */
	/* 	printk("main, returned: %s\n", rsp); */
	k_msleep(10000);

	printk("Exiting main\n");
}

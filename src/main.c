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
	memcpy(send_buffer, "ATI\r", 10);

	k_msleep(SLEEP_TIME_MS*2);

	int reps = 0;
	printk("Begin main loop\n");
	while (reps < 1) {
		shtc3_sensor_read();

		char rsp[100];
		send_at_command(send_buffer, strlen(send_buffer), rsp);

		/* memset(send_buffer, 0, RING_BUFFER_SIZE); */
		/* memcpy(send_buffer, "AT+GMI\r", 7); */
		/* /1* memcpy(send_buffer, "AT+QPOWD=0\r", 11); *1/ */
		/* ret = uart_tx(uart_dev, send_buffer, strlen(send_buffer), 100); */
		/* printk("uart_tx end, ret = %d:\n", ret); */
		reps++;
	}
	printk("Exiting main...");
}

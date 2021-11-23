#include "main.h"

#define PAYLOAD_MAX_SIZE 600

char *set_payload(char *imei, char *vbatt) {
	char *buffer = k_malloc(PAYLOAD_MAX_SIZE*sizeof(char));
	memset(buffer, 0, PAYLOAD_MAX_SIZE);

	snprintk(buffer, PAYLOAD_MAX_SIZE,
		 "{"
			 "\"id_can\":\"%s\","
			 "\"id_sku\":\"%s\","
			 "\"id_device\":\"%s\","
			 "\"bat_val\":\"%s\""
		 "}",
	"dev0",
	"sku_dev_0",
	imei,
	vbatt);

	return buffer;
}

void main(void)
{
	printk("Hello\n");

	/* char *buffer = k_malloc(PAYLOAD_MAX_SIZE*sizeof(char)); */
	/* char buffer[PAYLOAD_MAX_SIZE]; */
	/* memset(buffer, 0, PAYLOAD_MAX_SIZE); */
	char *buffer = set_payload("abc123456", "9100mV");
	/* set_payload(buffer, "abc123456", "9100mV"); */
	printk("%s\n", buffer);
	free(buffer);
	/* if (init_board_sensors() == false) { */
	/* 	printk("Failed sensor initialization, stopping...\n"); */
	/* 	return; */
	/* } */

	/* if (init_bg96() == false) { */
	/* 	printk("Failed BG96 initialization, stopping...\n"); */
	/* 	return; */
	/* } */

	/* printk("Devices initialized!\n"); */

	/* k_msleep(SLEEP_TIME_MS*2); */

	/* int ret = server_connect(); */
	/* if (ret) { */
	/* 	printk("Connected\n"); */
	/* 	send_payload("testetesteteste", strlen("testetesteteste")); */
	/* } else { */
	/* 	printk("Not connected\n"); */
	/* } */

	/* memset(rsp, 0, 100); */
	/* printk("\n\nGet IMEI\n"); */
	/* send_at_command("AT+GSN", strlen("AT+GSN"), rsp); */
	/* 	printk("main, returned: %s\n", rsp); */
	/* k_msleep(10000); */

	printk("Exiting main\n");
}

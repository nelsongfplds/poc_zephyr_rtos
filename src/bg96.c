#include "bg96.h"

/* Statics */
static pthread_mutex_t uart_mutex;
static pthread_cond_t uart_cond;
static const struct device *uart_dev;
static const struct device *gpio0_dev;
static const struct device *gpio1_dev;
static uint8_t recv_buffer[RING_BUFFER_SIZE]; //TODO: Change to ring buffer
static uint8_t bg96_resp[BG96_AT_RSP_MAX_LEN];
static uint32_t bg96_resp_len = 0;

static void uart_callback(const struct device *uart_dev, struct uart_event *evt, void *data) {
	int ret;

	ret = pthread_mutex_lock(&uart_mutex);
	if (ret) {
		printk("Error locking mutex, %d\n", ret);
	}

	switch (evt->type) {
		case UART_TX_DONE:
			printk("UART_TX_DONE\n");
			break;
		case UART_TX_ABORTED:
			printk("UART_TX_ABORTED\n");
			break;
		case UART_RX_RDY:
			printk("UART_RX_RDY\n");
			/* printk("\n\n\n\nOFFSET: %d\n\n\n\n", evt->data.rx.offset); */
			memcpy(bg96_resp, &evt->data.rx.buf[evt->data.rx.offset], evt->data.rx.len);
			bg96_resp_len = evt->data.rx.len;
			printk("[UART_CALLBACK]: %s\n", bg96_resp);
			printk("signal wakeup to sleeping thread\n");
			pthread_cond_signal(&uart_cond);
			break;
		case UART_RX_BUF_REQUEST:
			printk("UART_RX_BUF_REQUEST\n");
			break;
		case UART_RX_BUF_RELEASED:
			printk("UART_RX_BUF_RELEASED\n");
			break;
		case UART_RX_DISABLED:
			printk("UART_RX_DISABLED\n");
			break;
		case UART_RX_STOPPED:
			printk("UART_RX_STOPPED\n");
			break;
		default:
			printk("Unknown event: %d\n", evt->type);
			break;
	}

	ret = pthread_mutex_unlock(&uart_mutex);
	if (ret) {
		printk("Error unlocking mutex, %d\n", ret);
	}
}

static bool init_uart() {
	uart_dev = device_get_binding(DT_LABEL(UART0_NODE));
	if (uart_dev == NULL) {
		printk("UART0 not found");
		return false;
	}

	memset(recv_buffer, 0, RING_BUFFER_SIZE);
	uart_callback_set(uart_dev, uart_callback, NULL);
	uart_rx_enable(uart_dev, recv_buffer, RING_BUFFER_SIZE, 100);

	return true;
}

static bool init_gpio1() {
	int ret;
	gpio1_dev = device_get_binding(DT_LABEL(GPIO1_NODE));

	if (gpio1_dev == NULL) {
		printk("GPIO1 not found");
		return false;
	}

	/* 3V8 */
	ret = gpio_pin_configure(gpio1_dev, MDM_3V8_PIN, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		printk("Error configuring MDM_3V8_PIN: %d", ret);
		return false;
	}

	gpio_pin_set(gpio1_dev, MDM_3V8_PIN, 1);

	return true;
}

static bool init_gpio0() {
	int ret;
	gpio0_dev = device_get_binding(DT_LABEL(GPIO0_NODE));

	if (gpio0_dev == NULL) {
		printk("GPIO0 not found");
		return false;
	}

	/* power pin */
	ret = gpio_pin_configure(gpio0_dev, MDM_PWR_PIN, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		printk("Error configuring MDM_PWR_PIN: %d", ret);
		return false;
	}

	gpio_pin_set(gpio0_dev, MDM_PWR_PIN, 0);
	k_msleep(60);
	gpio_pin_set(gpio0_dev, MDM_PWR_PIN, 1);
	k_msleep(300);

	return true;
}

static void setup_cat_m1() {
	char rsp[100];

	memset(rsp, 0, 100);

	send_at_command("AT+QCFG=\"nwscanseq\",01,1", strlen("AT+QCFG=\"nwscanseq\",01,1"), NULL);
	send_at_command("AT+QCFG=\"nwscanmode\",0,1", strlen("AT+QCFG=\"nwscanmode\",0,1"), NULL);
	send_at_command("AT+QCFG=\"iotopmode\",0,1", strlen("AT+QCFG=\"iotopmode\",0,1"), NULL);
	send_at_command("AT+QCFG=\"band\",f,8000004,8000004,1", strlen("AT+QCFG=\"band\",f,8000004,8000004,1"), NULL);
	send_at_command("AT+CGDCONT=1,\"IP\",\"zap.vivo.com.br\"", strlen("AT+CGDCONT=1,\"IP\",\"zap.vivo.com.br\""), NULL);
	printk("==== LTE-CAT-M1 and 2G networks initialized ====\n");
}

static bool network_registration() {
	char rsp[200];

	memset(rsp, 0, 200);
	send_at_command("AT+CEREG?", strlen("AT+CEREG?"), rsp);
	if (strstr(rsp, "+CEREG: 0,1") != NULL) {
		return true;
	}

	memset(rsp, 0, 200);
	send_at_command("AT+CGREG?", strlen("AT+CGREG?"), rsp);
	if (strstr(rsp, "+CGREG: 0,1") != NULL) {
		return true;
	}

	return false;
}

static void init_mqtt () {
	send_at_command("AT+QMTCFG=\"version\",0,4", strlen("AT+QMTCFG=\"version\",0,4"), NULL);
	send_at_command("AT+QMTCFG=\"keepalive\",0,60", strlen("AT+QMTCFG=\"keepalive\",0,60"), NULL);
	send_at_command("AT+QSSLCFG=\"ignorelocaltime\",2,1", strlen("AT+QSSLCFG=\"ignorelocaltime\",2,1"), NULL);
	send_at_command("AT+QSSLCFG=\"ciphersuite\",2,0xFFFF", strlen("AT+QSSLCFG=\"ciphersuite\",2,0xFFFF"), NULL);
	send_at_command("AT+QMTCFG=\"SSL\",0,1,2", strlen("AT+QMTCFG=\"SSL\",0,1,2"), NULL);
	send_at_command("AT+QSSLCFG=\"seclevel\",2,0", strlen("AT+QSSLCFG=\"seclevel\",2,0"), NULL);
	printk("==== MQTT stack initialized ====\n\n");
}

//TODO: place this after comm closure
static void deinit_mqtt() {
	send_at_command("AT+QMTCLOSE=0", strlen("AT+QMTCLOSE=0"), NULL);
}

static bool mqtt_connect() {
	printk("mqtt_connect() called\n"); //TODO:REMOVE
	char open_cmd[] = "AT+QMTOPEN=0,\"GEOKEG-DEV.azure-devices.net\",8883";
	char conn_cmd[] = "AT+QMTCONN=0,\"dev0\",\"GEOKEG-DEV.azure-devices.net/dev0/?api-version=2018-06-30\",\"SharedAccessSignature sr=GEOKEG-DEV.azure-devices.net%2Fdevices%2Fdev0&sig=tK%2BpCrjLHbJ5ghCbyo%2ByZ7I9%2BSjUOJnhhInfF8JTfNE%3D&se=1642703697\"";

	// TODO: need a way to timeout the next two commands, maybe change to pthread_cond_timedwait
	printk("ATTEMPT TO OPEN CONNECTION\n");
	send_at_command(open_cmd, strlen(open_cmd), NULL);

	printk("Sleep until QMTOPEN returns\n");
	pthread_mutex_lock(&uart_mutex);
	pthread_cond_wait(&uart_cond, &uart_mutex);
	printk("QMTOPEN returned! buffer: %s\n", bg96_resp);
	if (strstr(bg96_resp, "+QMTOPEN: 0,0") == NULL) {
		pthread_mutex_unlock(&uart_mutex);
		return false;
	}
	pthread_mutex_unlock(&uart_mutex);


	printk("ATTEMPT TO CONNECT TO AZURE\n");
	send_at_command(conn_cmd, strlen(conn_cmd), NULL);

	printk("Sleep until QMTCONN returns\n");
	pthread_mutex_lock(&uart_mutex);
	pthread_cond_wait(&uart_cond, &uart_mutex);
	printk("QMTCONN returned! buffer: %s\n", bg96_resp);
	if (strstr(bg96_resp, "+QMTCONN: 0,0,0") == NULL) {
		pthread_mutex_unlock(&uart_mutex);
		return false;
	}
	pthread_mutex_unlock(&uart_mutex);

	return true;
}

bool init_bg96() {
	bool ret;

	pthread_mutex_init(&uart_mutex, NULL);
	pthread_cond_init(&uart_cond, NULL);

	ret = init_uart();
	if (ret == false) {
		return false;
	}

	ret = init_gpio0();
	if (ret == false) {
		return false;
	}

	ret = init_gpio1();
	if (ret == false) {
		return false;
	}
	printk("Sleeping for 10s to power up the modem...\n");
	k_msleep(10000);

	char rsp[100];
	printk("Sending ATE0 to disable echo and allow thread synchronization...\n");
	send_at_command("ATE0", 4, rsp);
	k_msleep(10000);

	setup_cat_m1();

	return true;
}

bool server_connect() {
	bool ret;

	ret = network_registration();
	if (ret == false) {
		return false;
	}

	init_mqtt();
	printk("Need to call mqtt_connect\n"); //TODO:REMOVE
	ret = mqtt_connect();
	/* deinit_mqtt(); */
	return ret;
}

bool send_payload(char *payload, uint32_t payload_len) {
	printk("Begin routine to send payload...\n");

	char send_cmd[BG96_AT_CMD_MAX_LEN];

	memset(send_cmd, 0, BG96_AT_CMD_MAX_LEN);
	printk("payload: %s, size: %u\n", payload, payload_len);
	memcpy(send_cmd, payload, payload_len);
	send_cmd[payload_len] = '\x1A';
	printk("send_cmd: %s\n", send_cmd);

	char cmd[] = "AT+QMTPUB=0,0,0,0,\"devices/dev0/messages/events/geoKegEvents/\"";
	send_at_command(cmd, strlen(cmd), NULL);

	printk("QMTPUB returned! buffer: %s\n", bg96_resp);
	if (strstr(bg96_resp, ">") == NULL) {
		printk("Failed to send payload\n");
		return false;
	}

	printk("Send payload...\n");
	send_at_command(send_cmd, strlen(send_cmd), NULL);

	printk("Sleep until QMTPUB returns\n");
	pthread_mutex_lock(&uart_mutex);
	pthread_cond_wait(&uart_cond, &uart_mutex);
	printk("QMTPUB returned! buffer: %s\n", bg96_resp);
	if (strstr(bg96_resp, "+QMTPUB: 0,0,0") == NULL) {
		pthread_mutex_unlock(&uart_mutex);
		return false;
	}
	pthread_mutex_unlock(&uart_mutex);

	return true;
}

uint32_t send_at_command(char *cmd, uint32_t cmd_len, char *cmd_resp) {
	if (cmd_len > BG96_AT_CMD_MAX_LEN) {
		return 0;
	}

	/* Lock in order to block calling thread while waiting for the UART response */
	pthread_mutex_lock(&uart_mutex);

	int ret;
	char send_cmd[BG96_AT_CMD_MAX_LEN];
	char send_cmd_len = cmd_len + 1;

	memset(bg96_resp, 0, BG96_AT_RSP_MAX_LEN);
	memset(send_cmd, 0, BG96_AT_CMD_MAX_LEN);
	memcpy(send_cmd, cmd, cmd_len);
	send_cmd[cmd_len] = '\r';

	ret = uart_tx(uart_dev, send_cmd, send_cmd_len, 100);

	/* sleep until callback signals to wake up */
	pthread_cond_wait(&uart_cond, &uart_mutex);
	/* woke up, continuing execution */

	if (cmd_resp != NULL) {
		memcpy(cmd_resp, bg96_resp, bg96_resp_len);
	}

	pthread_mutex_unlock(&uart_mutex);

	return bg96_resp_len;
}


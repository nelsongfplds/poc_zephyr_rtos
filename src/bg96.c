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
			printk("\n\n\n\nOFFSET: %d\n\n\n\n", evt->data.rx.offset);
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

	/* status pin */
	ret = gpio_pin_configure(gpio0_dev, MDM_STATUS_PIN, GPIO_INPUT);
	if (ret < 0) {
		printk("Error configuring MDM_STATUS_PIN: %d", ret);
		return false;
	}

	/* dtr pin */
	ret = gpio_pin_configure(gpio0_dev, MDM_DTR_PIN, GPIO_INPUT);
	if (ret < 0) {
		printk("Error configuring MDM_DTR_PIN: %d", ret);
		return false;
	}

	/* ap ready pin */
	ret = gpio_pin_configure(gpio0_dev, MDM_AP_RDY_PIN, GPIO_INPUT);
	if (ret < 0) {
		printk("Error configuring MDM_AP_RDY_PIN: %d", ret);
		return false;
	}

	/* psm pin */
	ret = gpio_pin_configure(gpio0_dev, MDM_PSM_PIN, GPIO_INPUT);
	if (ret < 0) {
		printk("Error configuring MDM_PSM_PIN: %d", ret);
		return false;
	}

	/* reset pin */
	ret = gpio_pin_configure(gpio0_dev, MDM_RST_PIN, GPIO_OUTPUT_INACTIVE);
	if (ret < 0) {
		printk("Error configuring MDM_RST_PIN: %d", ret);
		return false;
	}

	/* power pin */
	ret = gpio_pin_configure(gpio0_dev, MDM_PWR_PIN, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		printk("Error configuring MDM_PWR_PIN: %d", ret);
		return false;
	}

	/* w disable pin */
	ret = gpio_pin_configure(gpio0_dev, MDM_W_DISABLE_PIN, GPIO_OUTPUT_INACTIVE);
	if (ret < 0) {
		printk("Error configuring MDM_W_DISABLE_PIN: %d", ret);
		return false;
	}

	gpio_pin_set(gpio0_dev, MDM_RST_PIN, 0);
	gpio_pin_set(gpio0_dev, MDM_W_DISABLE_PIN, 0);
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

bool mqtt_connect() {
	char auth_rsp[200];
	char conn_rsp[200];
	int port = 8883;

	memset(auth_rsp, 0, 200);
	memset(conn_rsp, 0, 200);
	send_at_command("", strlen(""), conn_rsp);
	send_at_command("", strlen(""), auth_rsp);

	if (strstr(auth_rsp, "+QMTCONN: 0,0,0") != NULL && strstr(conn_rsp, "+QMTOPEN: 0,0") != NULL) {
		return true;
	}

	return false;
}

bool server_connect() {
	bool ret;

	ret = network_registration();
	if (ret == false) {
		return false;
	}

	init_mqtt();

	return mqtt_connect();
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


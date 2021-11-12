#include "bg96.h"

/* Statics */
static uint8_t gambi_counter = 0; //FIXME: workaround
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
			gambi_counter++;
			memcpy(bg96_resp, &evt->data.rx.buf[evt->data.rx.offset], evt->data.rx.len);
			bg96_resp_len = evt->data.rx.len;
			if (gambi_counter >= 2) {
				printk("signal wakeup to sleeping thread\n");
				pthread_cond_signal(&uart_cond);
			}
			/* printk("recv_buffer: %s\n", (char*) recv_buffer); */
			/* printk("[@rys] len: %d buff: %s\n", evt->data.rx.len, &evt->data.rx.buf[evt->data.rx.offset]); */
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

	//TODO: REMOVE?
	/* /1* RTS pin *1/ */
	/* ret = gpio_pin_configure(gpio_dev, 7, GPIO_OUTPUT_INACTIVE); */
	/* if (ret < 0) { */
	/* 	printk("Error configuring RTS: %d", ret); */
	/* } */
	/* /1* CTS pin *1/ */
	/* ret = gpio_pin_configure(gpio_dev, 11, GPIO_OUTPUT_INACTIVE); */
	/* if (ret < 0) { */
	/* 	printk("Error configuring CTS: %d", ret); */
	/* } */
	/* gpio_pin_set(gpio_dev, 7, 0); */
	/* gpio_pin_set(gpio_dev, 11, 0); */
	/* k_msleep(1000); */

	gpio_pin_set(gpio0_dev, MDM_RST_PIN, 0);
	gpio_pin_set(gpio0_dev, MDM_W_DISABLE_PIN, 0);
	gpio_pin_set(gpio0_dev, MDM_PWR_PIN, 0);
	k_msleep(60);
	gpio_pin_set(gpio0_dev, MDM_PWR_PIN, 1);
	k_msleep(300);

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

	return true;
}

uint32_t send_at_command(char *cmd, uint32_t cmd_len, char *cmd_resp) {
	gambi_counter = 0;
	if (cmd_len > BG96_AT_CMD_MAX_LEN) {
		return 0;
	}

	/* Lock in order to block calling thread while waiting for the UART response */
	pthread_mutex_lock(&uart_mutex);

	int ret;
	char send_cmd[BG96_AT_CMD_MAX_LEN];
	char send_cmd_len = cmd_len + 1;

	printk("received cmd: %s\n", cmd);

	memset(bg96_resp, 0, BG96_AT_RSP_MAX_LEN);
	memset(send_cmd, 0, BG96_AT_CMD_MAX_LEN);
	memcpy(send_cmd, cmd, cmd_len);
	send_cmd[cmd_len] = '\r';

	ret = uart_tx(uart_dev, send_cmd, send_cmd_len, 100);

	printk("sleep until callback signals to wake up\n");
	pthread_cond_wait(&uart_cond, &uart_mutex);
	printk("woke up, continuing execution\n");

	memcpy(cmd_resp, bg96_resp, bg96_resp_len);

	pthread_mutex_unlock(&uart_mutex);

	// TODO: set response to cmd_resp
	return bg96_resp_len;
}

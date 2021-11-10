#include <device.h>
#include <sensor.h>
#include <zephyr.h>
#include <string.h>
#include <stdlib.h>
#include <devicetree.h>
#include <drivers/gpio.h>
#include <drivers/uart.h>
#include <sys/ring_buffer.h>

#define SLEEP_TIME_MS   5000

/* The devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_ALIAS(led0)

#if DT_NODE_HAS_STATUS(LED0_NODE, okay)
#define LED0	DT_GPIO_LABEL(LED0_NODE, gpios)
#define PIN	DT_GPIO_PIN(LED0_NODE, gpios)
#define FLAGS	DT_GPIO_FLAGS(LED0_NODE, gpios)
#else
/* A build error here means your board isn't set up to blink an LED. */
#error "Unsupported board: led0 devicetree alias is not defined"
#define LED0	""
#define PIN	0
#define FLAGS	0
#endif

/* The devicetree node identifier for the "shtc3" alias. */
#define SHTC3_NODE DT_ALIAS(shtc3)

#if !DT_NODE_HAS_STATUS(SHTC3_NODE, okay)
#error "Unsupported board: shtc3 devicetree alias is not defined"
#endif

#define UART0_NODE DT_NODELABEL(uart0)

#if !DT_NODE_HAS_STATUS(UART0_NODE, okay)
#error "Unsupported board: uart0 devicetree node is not defined"
#endif

#define GPIO0_NODE DT_NODELABEL(gpio0)
#if DT_NODE_HAS_STATUS(GPIO0_NODE, okay)
#define MDM_STATUS_PIN    31
#define MDM_RST_PIN       28
#define MDM_PWR_PIN       2
#define MDM_W_DISABLE_PIN 29
#define MDM_DTR_PIN       26
#define MDM_AP_RDY_PIN    30
#define MDM_PSM_PIN       3
#else
#error "Unsupported board: gpio0 devicetree node is not defined"
#endif

#define GPIO1_NODE DT_NODELABEL(gpio1)
#if DT_NODE_HAS_STATUS(GPIO1_NODE, okay)
#define MDM_3V8_PIN       9
#else
#error "Unsupported board: gpio1 devicetree node is not defined"
#endif

#define RING_BUFFER_SIZE 1024

/* Static variables */
static uint8_t recv_buffer[RING_BUFFER_SIZE];
static uint8_t send_buffer[RING_BUFFER_SIZE];

/* Static methods */
static const struct device *init_led() {
	int ret;
	const struct device *led_dev = device_get_binding(LED0);

	if (led_dev == NULL) {
		printk("LED0 not found\n");
		return NULL;
	}

	ret = gpio_pin_configure(led_dev, PIN, GPIO_OUTPUT_ACTIVE | FLAGS);
	if (ret < 0) {
		printk("LED0 not configured\n");
		return NULL;
	}

	return led_dev;
}

static const struct device *init_shtc3() {
	const struct device *shtc3_dev = device_get_binding(DT_LABEL(SHTC3_NODE));

	return shtc3_dev;
}


static void uart_callback(const struct device *uart_dev, struct uart_event *evt, void *data) {
	switch (evt->type) {
		case UART_TX_DONE:
			printk("UART_TX_DONE\n");
			break;
		case UART_TX_ABORTED:
			printk("UART_TX_ABORTED\n");
			break;
		case UART_RX_RDY:
			printk("UART_RX_RDY\n");
			/* printk("recv_buffer: %s\n", (char*) recv_buffer); */
			printk("[@rys] len: %d buff: %s\n", evt->data.rx.len, &evt->data.rx.buf[evt->data.rx.offset]);
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
}

static const struct device *init_uart() {
	const struct device *uart0_dev = device_get_binding(DT_LABEL(UART0_NODE));
	if (uart0_dev == NULL) {
		return NULL;
	}

	memset(recv_buffer, 0, RING_BUFFER_SIZE);
	memset(send_buffer, 0, RING_BUFFER_SIZE);
	memcpy(send_buffer, "ATI\r", 10);
	uart_callback_set(uart0_dev, uart_callback, NULL);
	uart_rx_enable(uart0_dev, recv_buffer, RING_BUFFER_SIZE, 100);

	return uart0_dev;
}

static void init_gpio1() {
	int ret;
	const struct device *gpio_dev = device_get_binding(DT_LABEL(GPIO1_NODE));

	if (gpio_dev == NULL) {
		printk("GPIO1 not found");
	}

	/* 3V8 */
	ret = gpio_pin_configure(gpio_dev, MDM_3V8_PIN, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		printk("Error configuring MDM_3V8_PIN: %d", ret);
	}

	gpio_pin_set(gpio_dev, MDM_3V8_PIN, 1);
}

static const struct device *init_gpio0() {
	int ret;
	const struct device *gpio_dev = device_get_binding(DT_LABEL(GPIO0_NODE));

	if (gpio_dev == NULL) {
		printk("GPIO0 not found");
		return NULL;
	}

	/* status pin */
	ret = gpio_pin_configure(gpio_dev, MDM_STATUS_PIN, GPIO_INPUT);
	if (ret < 0) {
		printk("Error configuring MDM_STATUS_PIN: %d", ret);
	}

	/* dtr pin */
	ret = gpio_pin_configure(gpio_dev, MDM_DTR_PIN, GPIO_INPUT);
	if (ret < 0) {
		printk("Error configuring MDM_DTR_PIN: %d", ret);
	}

	/* ap ready pin */
	ret = gpio_pin_configure(gpio_dev, MDM_AP_RDY_PIN, GPIO_INPUT);
	if (ret < 0) {
		printk("Error configuring MDM_AP_RDY_PIN: %d", ret);
	}

	/* psm pin */
	ret = gpio_pin_configure(gpio_dev, MDM_PSM_PIN, GPIO_INPUT);
	if (ret < 0) {
		printk("Error configuring MDM_PSM_PIN: %d", ret);
	}

	/* reset pin */
	ret = gpio_pin_configure(gpio_dev, MDM_RST_PIN, GPIO_OUTPUT_INACTIVE);
	if (ret < 0) {
		printk("Error configuring MDM_RST_PIN: %d", ret);
	}

	/* power pin */
	ret = gpio_pin_configure(gpio_dev, MDM_PWR_PIN, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		printk("Error configuring MDM_PWR_PIN: %d", ret);
	}

	/* w disable pin */
	ret = gpio_pin_configure(gpio_dev, MDM_W_DISABLE_PIN, GPIO_OUTPUT_INACTIVE);
	if (ret < 0) {
		printk("Error configuring MDM_W_DISABLE_PIN: %d", ret);
	}

	/* RTS pin */
	ret = gpio_pin_configure(gpio_dev, 7, GPIO_OUTPUT_INACTIVE);
	if (ret < 0) {
		printk("Error configuring RTS: %d", ret);
	}
	/* CTS pin */
	ret = gpio_pin_configure(gpio_dev, 11, GPIO_OUTPUT_INACTIVE);
	if (ret < 0) {
		printk("Error configuring CTS: %d", ret);
	}
	gpio_pin_set(gpio_dev, 7, 0);
	gpio_pin_set(gpio_dev, 11, 0);
	k_msleep(1000);

	gpio_pin_set(gpio_dev, MDM_RST_PIN, 0);
	gpio_pin_set(gpio_dev, MDM_W_DISABLE_PIN, 0);
	gpio_pin_set(gpio_dev, MDM_PWR_PIN, 0);
	k_msleep(60);
	gpio_pin_set(gpio_dev, MDM_PWR_PIN, 1);
	k_msleep(300);

	return gpio_dev;
}

static void shtc3_sensor_read(const struct device *shtc3_dev) {
	int ret;
	struct sensor_value shtc3_sv;

	ret = sensor_sample_fetch(shtc3_dev);
	if (ret != 0) {
		printk("sensor_sample_fetch error: %d\n", ret);
	}

	ret = sensor_channel_get(shtc3_dev, SENSOR_CHAN_AMBIENT_TEMP, &shtc3_sv);
	if (ret != 0) {
		printk("sensor_channel_get error: %d\n", ret);
	}

	/* printk("Temperature: %f C\n", sensor_value_to_double(&shtc3_sv)); */
	printk("Temperature: %d.%06d C\n", shtc3_sv.val1, shtc3_sv.val2);

	ret = sensor_channel_get(shtc3_dev, SENSOR_CHAN_HUMIDITY, &shtc3_sv);
	if (ret != 0) {
		printk("sensor_channel_get error: %d\n", ret);
	}

	/* printk("Humidity: %f C\n", sensor_value_to_double(&shtc3_sv)); */
	printk("Humidity: %d.%06d C\n", shtc3_sv.val1, shtc3_sv.val2);
}

void main(void)
{
	const struct device *led_dev = init_led();
	const struct device *uart_dev = init_uart();
	const struct device *gpio0_dev = init_gpio0();
	const struct device *shtc3_dev = init_shtc3();
	init_gpio1();

	if (led_dev == NULL) {
		printk("Led not found, stopping...\n");
		return;
	}

	if (shtc3_dev == NULL) {
		printk("SHTC3 sensor not found, stopping...\n");
		return;
	}

	if (uart_dev == NULL) {
		printk("UART0 not found, stopping...\n");
		return;
	}

	if (gpio0_dev == NULL) {
		printk("GPIO_0 not found, stopping...\n");
		return;
	}

	printk("Devices initialized!\n");
	gpio_pin_set(led_dev, PIN, 0);

	k_msleep(SLEEP_TIME_MS*2);

	int reps = 0;
	/* printk("Led set, begin main loop\n"); */
	while (reps < 1) {
		/* shtc3_sensor_read(shtc3_dev); */

		printk("uart_tx begin:\n");
		int ret = uart_tx(uart_dev, send_buffer, strlen(send_buffer), 100);
		printk("uart_tx end, ret = %d:\n", ret);
		k_msleep(500);
		k_msleep(SLEEP_TIME_MS*2);

		memset(send_buffer, 0, RING_BUFFER_SIZE);
		memcpy(send_buffer, "AT+GMI\r", 7);
		/* memcpy(send_buffer, "AT+QPOWD=0\r", 11); */
		ret = uart_tx(uart_dev, send_buffer, strlen(send_buffer), 100);
		printk("uart_tx end, ret = %d:\n", ret);
		reps++;
	}
	printk("Exiting main...");
}

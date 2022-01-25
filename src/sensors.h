#ifndef GEOCAN_SENSORS
#define GEOCAN_SENSORS

/* Includes */
#include <sensor.h>
#include <device.h>
#include <drivers/gpio.h>
#include "battery.h"

/* Defines */
/* The devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_ALIAS(led0)

#if DT_NODE_HAS_STATUS(LED0_NODE, okay)
#define LED0	DT_GPIO_LABEL(LED0_NODE, gpios)
#define PIN	DT_GPIO_PIN(LED0_NODE, gpios)
#define FLAGS	DT_GPIO_FLAGS(LED0_NODE, gpios)
#else
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

#define TEMP_SENSOR_BUFF_LEN 35

/* Methods */
bool init_board_sensors();
bool shtc3_sensor_read(char *temp, char *ur, size_t temp_len, size_t ur_len);
int get_batt_reading();

#endif /* GEOCAN_SENSORS */

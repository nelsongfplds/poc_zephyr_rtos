#ifndef GEOCAN_BG96
#define GEOCAN_BG96

/* Includes */
#include <stdlib.h>
#include <pthread.h>
#include <drivers/gpio.h>
#include <drivers/uart.h>
#include <sys/ring_buffer.h>
#include <string.h>
#include "server.h"

/* Defines */
#define OPEN_CONN_TIMEOUT_S (75+5)
#define CONN_TIMEOUT_S (5+5)
#define SLEEP_TIME_MS   5000
#define RING_BUFFER_SIZE 1024
#define BG96_AT_CMD_MAX_LEN 400
#define BG96_AT_RSP_MAX_LEN 200 //TODO: Maybe resize this to a bigger number
#define IMEI_SIZE 15
#define UTC_TIME_ID 0
#define LATITUDE_ID 1
#define LONGITUDE_ID 2
#define ALTITUDE_ID 4
#define LATITUDE_LEN 30
#define LONGITUDE_LEN 30

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
#define GPS_EN_PIN        7
#define MDM_3V8_PIN       9
#else
#error "Unsupported board: gpio1 devicetree node is not defined"
#endif

/* Methods */
bool init_bg96();
bool server_connect();
bool send_payload(char *payload, uint32_t payload_len);
uint32_t send_at_command(char *cmd, uint32_t cmd_len, char *cmd_resp);
void get_imei(char *imei);
void turn_on_gps();
void turn_off_gps();
void determine_position(char *latitude, char *longitude, int *altitude);

#endif /* GEOCAN_BG96 */

#ifndef JSON_UTILS
#define JSON_UTILS

#include <device.h>
#include <stdlib.h>
#include <string.h>

/* Defines */
#define PAYLOAD_MAX_SIZE 500

/* Methods */
char *set_payload(char *imei, int vbatt, char *temp, char *ur, char *latitude, char *longitude);

#endif /* JSON_UTILS */

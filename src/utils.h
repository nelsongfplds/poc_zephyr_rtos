#ifndef JSON_UTILS
#define JSON_UTILS

#include <device.h>
#include <stdlib.h>
#include <string.h>

/* Defines */
#define PAYLOAD_MAX_SIZE 1024

/* Methods */
void set_payload(char *imei, int vbatt, char *temp, char *ur, char *latitude, char *longitude, char *buffer, size_t len);

#endif /* JSON_UTILS */

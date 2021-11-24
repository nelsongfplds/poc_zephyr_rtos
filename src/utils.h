#ifndef JSON_UTILS
#define JSON_UTILS

#include <device.h>
#include <stdlib.h>
#include <string.h>

/* Defines */
#define PAYLOAD_MAX_SIZE 500

/* Methods */
char *set_payload(char *imei, char *vbatt);

#endif /* JSON_UTILS */

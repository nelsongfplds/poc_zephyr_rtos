#include "utils.h"

char *set_payload(char *imei, int vbatt, char *temp, char *ur) {
	char *buffer = k_malloc(PAYLOAD_MAX_SIZE*sizeof(char));
	memset(buffer, 0, PAYLOAD_MAX_SIZE);

	snprintk(buffer, PAYLOAD_MAX_SIZE,
		 "{"
			 "\"id_sku\":\"%s\","
			 "\"id_geo_can\":\"%s\","
			 "\"lat\":\"%s\","
			 "\"long\":\"%s\","
			 "\"weight_dev\":\"%s\","
			 "\"temp_dev\":\"%s C\","
			 "\"ur_dev\":\"%s %%\","
			 "\"accel_dev\":\"%s\","
			 "\"bat_val\":\"%d mv\""
		 "}\x1A",
	"-/-", //id sku
	imei,
	"-/-", //lat
	"-/-", //long
	"-/-", //weight dev
	temp,
	ur,
	"-/-", //accel dev
	vbatt);

	return buffer;
}


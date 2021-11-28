#include "utils.h"

char *set_payload(char *imei, char *vbatt) {
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
			 "\"bat_val\":\"%s\""
		 "}\x1A",
	"-/-",
	imei,
	"-/-",
	"-/-",
	"-/-",
	"-/-", //temp dev
	"-/-", //ur dev
	"-/-", //accel dev
	vbatt);

	return buffer;
}


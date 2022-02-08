#include "utils.h"

void set_payload(char *imei, int vbatt, char *temp, char *ur, char *latitude, char *longitude, char *buffer, size_t len) {
	memset(buffer, 0, len);

	snprintk(buffer, len,
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
	latitude, //lat
	longitude, //long
	"-/-", //weight dev
	temp,
	ur,
	"-/-", //accel dev
	vbatt);
}


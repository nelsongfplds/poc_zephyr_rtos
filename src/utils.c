#include "utils.h"

char *set_payload(char *imei, char *vbatt) {
	char *buffer = k_malloc(PAYLOAD_MAX_SIZE*sizeof(char));
	memset(buffer, 0, PAYLOAD_MAX_SIZE);

	snprintk(buffer, PAYLOAD_MAX_SIZE,
		 "{"
			 "\"id_can\":\"%s\","
			 "\"id_sku\":\"%s\","
			 "\"id_device\":\"%s\","
			 "\"bat_val\":\"%s\""
		 "}\x1A",
	"dev0",
	"sku_dev_0",
	imei,
	vbatt);

	return buffer;
}


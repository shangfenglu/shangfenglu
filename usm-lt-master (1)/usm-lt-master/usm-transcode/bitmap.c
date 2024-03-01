#include "bitmap.h"
#include "debug_msg.h"
#include <stdio.h>
#include <stdlib.h>

struct bitmap* bitmap_init(uint64_t bufflen)
{
	struct bitmap* bitmap = (struct bitmap*)calloc(1, sizeof(struct bitmap));
	if (!bitmap) {
		DB_WAR("calloc bitmap failed\n");
		return NULL;
	}

	bitmap->bufflen = bufflen + 32;
	bitmap->buffer = (uint8_t*)calloc(1,sizeof(uint8_t) * (bufflen + 32));
	if (NULL == bitmap->buffer) {
		DB_WAR("calloc bitmap buffer failed\n");
		return NULL;
	}
	bitmap->resolution = (struct resolution*)calloc(1, sizeof(struct resolution));
	if (NULL == bitmap->resolution) {
		DB_WAR("calloc bitmap resolution failed\n");
		return NULL;
	}
	return bitmap;
}

int bitmap_free(struct bitmap* bitmap)
{
	if (!bitmap)
		return -1;
	DB_INFO("bitmap_free");
	if (bitmap->buffer)
		free(bitmap->buffer);
	DB_INFO("free bitmap->buffer success");
	free(bitmap);
	return 0;
}

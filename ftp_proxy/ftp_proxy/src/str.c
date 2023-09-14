#include "str.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int str_alloc(struct mystr* str, int alloc_length)
{
	str->data = (char*)malloc(sizeof(char) * alloc_length);
	str->alloc_length = alloc_length;
	str->data_length = 0;
	if (NULL == str->data)
	{
		printf("%s:malloc is null\n", __FUNCTION__);
		return -1;
	}
	return 0;
}

int str_free(struct mystr* str)
{
	str->alloc_length = 0;
	str->data_length = 0;
	free(str->data);
	str->data = NULL;

	return 0;
}

int str_append_alloc(struct mystr* str, int cat_length)
{
	str->alloc_length += cat_length;
	void* addr = malloc(sizeof(char) * str->alloc_length);
	if (NULL == addr)
		return -1;
	memcpy(addr, str->data, sizeof(char) * (str->data_length));
	free(str->data);
	str->data = NULL;
	str->data = (char*)addr;

	return 0;
}

int str_isempty(const struct mystr* str)
{
	if (0 == str->data_length)
		return 1;
	return 0;
}

int str_getlen(const struct mystr* str)
{
	return str->data_length;
}

const char* str_getbuf(const struct mystr* str)
{
	return str->data;
}

char str_at(const struct mystr* str, const unsigned int index)
{
	char ret;
	if (index >= str->alloc_length)
		return -1;
	ret = str->data[index];
	return ret;
}

int str_upper(struct mystr* str)
{
	unsigned int i = 0;
	for (i = 0; i < str->data_length; i++) {
		str->data[i] = toupper(str->data[i]);
	}
	return 0;
}

int str_lower(struct mystr* str)
{
	unsigned int i = 0;
	for (i = 0; i < str->data_length; i++) {
		str->data[i] = tolower(str->data[i]);
	}
	return 0;
}

int str_upper_length(struct mystr* str, const unsigned int length)
{
	unsigned int i = 0;
	if (length > str->data_length) {
		return -1;
	}
	for (i = 0; i < length; i++) {
		str->data[i] = toupper(str->data[i]);
	}

	return 0;
}

int str_lower_length(struct mystr* str, const unsigned int length)
{
	unsigned int i = 0;
	if (length > str->data_length) {
		return -1;
	}
	for (i = 0; i < length; i++) {
		str->data[i] = tolower(str->data[i]);
	}
	return 0;
}

int str_copy_str(struct mystr* str_dest, const struct mystr* str_src)
{
	if (str_dest->alloc_length < str_src->data_length + 1) {
		if (str_dest->data != NULL) {
			free(str_dest->data);
		}
		str_dest->alloc_length = str_src->data_length + 1;
		str_dest->data = (char*)malloc(str_dest->alloc_length);

		if (str_dest->data == NULL) {
			return -1;
		}
	}

	memcpy(str_dest->data, str_src->data, sizeof(char) * str_src->data_length);
	str_dest->data[str_src->data_length] = '\0';
	str_dest->data_length = str_src->data_length;
	return 0;
}

int str_copy_text(struct mystr* str_dest, const char* text, unsigned int length)
{
	if (str_dest->alloc_length < (unsigned int)length + 1) {
		char* new_data = (char*)realloc(str_dest->data, length + 1);
		if (new_data == NULL) {
			return -1;
		}
		str_dest->data = new_data;
		str_dest->alloc_length = length + 1;
	}

	memcpy(str_dest->data, text, sizeof(char) * length);
	str_dest->data[length] = '\0';
	str_dest->data_length = length;
	return 0;
}

int str_equal_str(const struct mystr* str1, const struct mystr* str2)
{
	if (str1->data_length != str2->data_length) {
		return 0;
	}

	return strcmp(str1->data, str2->data) == 0 ? 1 : 0;
}

int str_equal_text(const struct mystr* str, const char* text)
{
	return strcmp(str->data, text) == 0 ? 1 : 0;
	return 0;
}

int str_append_str(struct mystr* str_dst, const struct mystr* str_src)
{
	unsigned int new_length = str_dst->data_length + str_src->data_length;

	if (str_dst->alloc_length < new_length + 1) {
		char* addr = (char*)malloc(new_length + 1);

		if (addr == NULL) {
			return -1;
		}

		memcpy(addr, str_dst->data, sizeof(char) * str_dst->data_length);

		free(str_dst->data);

		str_dst->data = addr;
		str_dst->alloc_length = new_length + 1;
	}

	// 追加源字符串数据到目标字符串
	strcat(str_dst->data, str_src->data);
	str_dst->data_length = new_length;
	return 0;
}

int str_append_text(struct mystr* str_dst, const char* text_src, unsigned int length)
{
	unsigned int text_length = length;
	unsigned int new_length = str_dst->data_length + text_length;

	if (str_dst->alloc_length < new_length + 1) {

		char* addr = (char*)malloc(new_length + 1);

		if (addr == NULL) {
			return -1;
		}

		memcpy(addr, str_dst->data, sizeof(char) * str_dst->data_length);

		free(str_dst->data);

		str_dst->data = addr;
		str_dst->alloc_length = new_length + 1;
	}

	// 追加文本到目标字符串
	memcpy(str_dst->data+str_dst->data_length, text_src, sizeof(char) * text_length);
	str_dst->data_length = new_length;
	return 0;
}

#ifndef __STR_
#define __STR_

struct mystr {
	char* data;                   // 字符串数据
	unsigned int alloc_length;          // 申请的字节空间大小
	unsigned int data_length;            // 字符串长度
};

//开辟字符串空间
int str_alloc(struct mystr* str, int alloc_length);

//释放字符串空间
int str_free(struct mystr* str);

//追加字符串空间
int str_append_alloc(struct mystr* str, int cat_length);

//判断是否为空
int str_isempty(const struct mystr* str);

//得到长度
int str_getlen(const struct mystr* str);

//得到内容
const char* str_getbuf(const struct mystr* str);

//下标得字符
char str_at(const struct mystr* str, const unsigned int index);

//转为大写
int str_upper(struct mystr* str);

//转为小写
int str_lower(struct mystr* str);

//转一定长度大写
int str_upper_length(struct mystr* str, const unsigned int length);

//转一定长度小写
int str_lower_length(struct mystr* str, const unsigned int length);

//拷贝字符串
int str_copy_str(struct mystr* str_dest, const struct mystr* str_src);
int str_copy_text(struct mystr* str_dest, const char* text, unsigned int length);

//比较字符string
int str_equal_str(const struct mystr* str1, const struct mystr* str2);
int str_equal_text(const struct mystr* str, const char* text);

//追加
int str_append_str(struct mystr* str_dst, const struct mystr* str_src);
int str_append_text(struct mystr* str_dst, const char* text_src, unsigned int length);

//分割


#endif  //__STR_

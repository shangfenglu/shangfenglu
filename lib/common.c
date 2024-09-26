#include <sys/uio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <errno.h>
#include <openssl/sha.h>
#include <ctype.h>
#include <limits.h>
#include <sys/poll.h> 
#include <unistd.h>
#include <netinet/in.h>   
#include <arpa/inet.h>  
#include <assert.h>
#include "common.h"
#include "debug_msg.h"
#include <stdlib.h>

extern void log_debug(int level, FILE *fp, char *sfile, int line, const char *fmt, ...);
#define DB_INFO(fmt, ARGS...)	log_debug(DBG_LEVEL_INFO, g_debug_fp?g_debug_fp:stdout, __FILENAME__, __LINE__, fmt, ##ARGS)

static const char *ALPHA_BASE = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
static const char *STANDARD_ALPHA_BASE = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

const char DeBase64Tab[] =
{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	62,        // '+'
	0, 0, 0,
	63,        // '/'
	52, 53, 54, 55, 56, 57, 58, 59, 60, 61,        // '0'-'9'
	0, 0, 0, 0, 0, 0, 0,
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
	13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,        // 'A'-'Z'
	0, 0, 0, 0, 0, 0,
	26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38,
	39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51,        // 'a'-'z'
};
char *comm_strrchr(const char *src, char c)
{
	assert(src);
	const char *psrc = src;
	char *dst = NULL;
	while (*psrc)
	{
		if (*psrc == c)
			dst =(char *) psrc;
		psrc++;
	}
	return dst;
}

/*
 * Copy src to string dst of size siz.  At most siz-1 characters
 * will be copied.  Always NUL terminates (unless siz == 0).
 * Returns strlen(src); if retval >= siz, truncation occurred.
 */
size_t oc_strlcpy(char *dst, char const *src, size_t siz)
{
	char *d = dst;
	char const *s = src;
	size_t n = siz;

	/* Copy as many bytes as will fit */
	if (n != 0 && --n != 0) {
		do {
			if ((*d++ = *s++) == 0)
				break;
		} while (--n != 0);
	}

	/* Not enough room in dst, add NUL and traverse rest of src */
	if (n == 0) {
		if (siz != 0)
			*d = '\0';	/* NUL-terminate dst */
		while (*s++) ;
	}

	return (s - src - 1);	/* count does not include NUL */
}


//code review 注释掉的宏先清理掉
int cmd_request_to_str(unsigned _cmd, char *out, int len)
{
	cmd_request_t cmd = _cmd;
	int ret;
	switch (cmd) {

	case AUTH_COOKIE_REP:
		ret = snprintf(out, len, "auth cookie reply");
		break;
	case AUTH_COOKIE_REQ:
		ret = snprintf(out, len, "auth cookie request");
		break;
	case CMD_COSIGN_EQ_REQ:
		ret = snprintf(out, len, "auth cosign request");
		break;
	case CMD_COSIGN_EQ_REP:
		ret = snprintf(out, len, "auth cosign reply");
		break;
#if 0
	case RESUME_STORE_REQ:
		return "resume data store request";
	case RESUME_DELETE_REQ:
		return "resume data delete request";
	case RESUME_FETCH_REQ:
		return "resume data fetch request";
	case RESUME_FETCH_REP:
		return "resume data fetch reply";
	case CMD_UDP_FD:
		return "udp fd";
	case CMD_TUN_MTU:
		return "tun mtu change";
	case CMD_TERMINATE:
		return "terminate";
	case CMD_SESSION_INFO:
		return "session info";
	case CMD_BAN_IP:
		return "ban IP";
	case CMD_BAN_IP_REPLY:
		return "ban IP reply";

	case CMD_SEC_CLI_STATS:
		return "sm: worker cli stats";
	case CMD_SECM_CLI_STATS:
		return "sm: main cli stats";
#endif
	case CMD_SEC_AUTH_INIT:
		ret = snprintf(out, len, "sm: auth init");
		break;
	case CMD_SEC_AUTH_REPLY:
		ret = snprintf(out, len, "sm: auth rep");
		break;
#if 0
	case CMD_SEC_AUTH_CONT:
		return "sm: auth cont";
	case CMD_SEC_DECRYPT:
		return "sm: decrypt";
	case CMD_SEC_SIGN:
		return "sm: sign";
	case CMD_SECM_STATS:
		return "sm: stats";
	case CMD_SECM_SESSION_CLOSE:
		return "sm: session close";
	case CMD_SECM_SESSION_OPEN:
		return "sm: session open";
	case CMD_SECM_SESSION_REPLY:
		return "sm: session reply";
	case CMD_SECM_BAN_IP:
		return "sm: ban IP";
	case CMD_SECM_BAN_IP_REPLY:
		return "sm: ban IP reply";
	case CMD_SECM_RELOAD:
		return "sm: reload";
	case CMD_SECM_RELOAD_REPLY:
		return "sm: reload reply";
	case CMD_SECM_LIST_COOKIES:
		return "sm: list cookies";
	case CMD_SECM_LIST_COOKIES_REPLY:
		return "sm: list cookies reply";
#endif
	// case CMD_SEC_USB_KEY_INIT:
	// 		ret = snprintf(out, len, "sm: usb key auth init");
	// 		break;
	// case CMD_SEC_USB_KEY_REPLY:
	// 		ret = snprintf(out, len, "sm: usb key auth rep");
	// 		break;
	default:
		ret = snprintf(out, len, "unknown (%u)", _cmd);
	}
	return ret;
}

ssize_t force_read_timeout(int sockfd, void *buf, size_t len, unsigned sec)
{
	int left = len;
	int ret;
	uint8_t *p = buf;
	struct pollfd pfd;

	while (left > 0) {
		if (sec > 0) {
			pfd.fd = sockfd;
			pfd.events = POLLIN;
			pfd.revents = 0;

			do {
				ret = poll(&pfd, 1, sec * 1000);
			} while (ret == -1 && errno == EINTR);

			if (ret == -1 || ret == 0) {
				errno = ETIMEDOUT;
				return -1;
			}
		}

		ret = read(sockfd, p, left);
		if (ret == -1) {
			if (errno != EAGAIN && errno != EINTR)
				return ret;
		} else if (ret == 0 && left != 0) {
			errno = ENOENT;
			return -1;
		}

		if (ret > 0) {
			left -= ret;
			p += ret;
		}
	}

	return len;
}

/* A hash of the input, to a 20-byte output. The goal is one-wayness.
 */
// static void safe_hash(const uint8_t *data, unsigned data_size, uint8_t output[20])
// {
// 	SHA_CTX ctx;

// 	SHA1_Init(&ctx);

// 	SHA1_Update(&ctx, data, data_size );
// 	SHA1_Final(output, &ctx );
// }

char *Base64Encode(const char *buf, const long size, char *base64Char) 
{
    int a = 0;
    int i = 0;
    while (i < size) {
        char b0 = buf[i++];
        char b1 = (i < size) ? buf[i++] : 0;
        char b2 = (i < size) ? buf[i++] : 0;
         
        int int63 = 0x3F; //  00111111
        int int255 = 0xFF; // 11111111
        base64Char[a++] = ALPHA_BASE[(b0 >> 2) & int63];
        base64Char[a++] = ALPHA_BASE[((b0 << 4) | ((b1 & int255) >> 4)) & int63];
        base64Char[a++] = ALPHA_BASE[((b1 << 2) | ((b2 & int255) >> 6)) & int63];
        base64Char[a++] = ALPHA_BASE[b2 & int63];
    }
    switch (size % 3) {
        case 1:
            base64Char[--a] = 'Z';
        case 2:
            base64Char[--a] = 'Z';
    }
    return base64Char;
}

char *Base64Decode(const char *base64Char, const long base64CharSize, char *originChar, long originCharSize) 
{
    int toInt[128] = {-1};
	int i;

    for (i = 0; i < 64; i++) {
        toInt[(int)ALPHA_BASE[i]] = i;
    }
    int int255 = 0xFF;
    int index = 0;
    for (i = 0; i < base64CharSize; i += 4) {
        int c0 = toInt[(int)base64Char[i]];
        int c1 = toInt[(int)base64Char[i + 1]];
        originChar[index++] = (((c0 << 2) | (c1 >> 4)) & int255);
        if (index >= originCharSize) {
            return originChar;
        }
        int c2 = toInt[(int)base64Char[i + 2]];
        originChar[index++] = (((c1 << 4) | (c2 >> 2)) & int255);
        if (index >= originCharSize) {
            return originChar;
        }
        int c3 = toInt[(int)base64Char[i + 3]];
        originChar[index++] = (((c2 << 6) | c3) & int255);
    }
    return originChar;
}

int EncodeBase64(const unsigned char* pSrc, char* pDst, int nSrcLen)
{
	unsigned char c1, c2, c3;    // 输入缓冲区读出3个字节
	int nDstLen = 0;             // 输出的字符计数
	int nLineLen = 0;            // 输出的行长度计数
	int nDiv = nSrcLen / 3;      // 输入数据长度除以3得到的倍数
	int nMod = nSrcLen % 3;      // 输入数据长度除以3得到的余数
	int i;
								 // 每次取3个字节，编码成4个字符
	for (i = 0; i < nDiv; i++)
	{
		// 取3个字节
		c1 = *pSrc++;
		c2 = *pSrc++;
		c3 = *pSrc++;

		// 编码成4个字符
		*pDst++ = STANDARD_ALPHA_BASE[c1 >> 2];
		*pDst++ = STANDARD_ALPHA_BASE[((c1 << 4) | (c2 >> 4)) & 0x3f];
		*pDst++ = STANDARD_ALPHA_BASE[((c2 << 2) | (c3 >> 6)) & 0x3f];
		*pDst++ = STANDARD_ALPHA_BASE[c3 & 0x3f];
		nLineLen += 4;
		nDstLen += 4;
	}

	// 编码余下的字节
	if (nMod == 1)
	{
		c1 = *pSrc++;
		*pDst++ = STANDARD_ALPHA_BASE[(c1 & 0xfc) >> 2];
		*pDst++ = STANDARD_ALPHA_BASE[((c1 & 0x03) << 4)];
		*pDst++ = '=';
		*pDst++ = '=';
		nLineLen += 4;
		nDstLen += 4;
	}
	else if (nMod == 2)
	{
		c1 = *pSrc++;
		c2 = *pSrc++;
		*pDst++ = STANDARD_ALPHA_BASE[(c1 & 0xfc) >> 2];
		*pDst++ = STANDARD_ALPHA_BASE[((c1 & 0x03) << 4) | ((c2 & 0xf0) >> 4)];
		*pDst++ = STANDARD_ALPHA_BASE[((c2 & 0x0f) << 2)];
		*pDst++ = '=';
		nDstLen += 4;
	}

	// 输出加个结束符
	*pDst = '\0';

	return nDstLen;
};

int DecodeBase64(const char* pSrc, unsigned char* pDst, int nSrcLen)
{
    int nDstLen;            // 输出的字符计数
	int nValue;             // 解码用到的长整数
	int i;

	i = 0;
	nDstLen = 0;

	// 取4个字符，解码到一个长整数，再经过移位得到3个字节
	while (i < nSrcLen)
	{
		if (*pSrc != '\r' && *pSrc != '\n')
		{
			nValue = DeBase64Tab[(int)*pSrc++] << 18;
			nValue += DeBase64Tab[(int)*pSrc++] << 12;
			*pDst++ = (nValue & 0x00ff0000) >> 16;
			nDstLen++;

			if (*pSrc != '=')
			{
				nValue += DeBase64Tab[(int)*pSrc++] << 6;
				*pDst++ = (nValue & 0x0000ff00) >> 8;
				nDstLen++;

				if (*pSrc != '=')
				{
					nValue += DeBase64Tab[(int)*pSrc++];
					*pDst++ = nValue & 0x000000ff;
					nDstLen++;
				}
			}

			i += 4;
		}
		else        // 回车换行，跳过
		{
			pSrc++;
			i++;
		}
	}

	// 输出加个结束符
	*pDst = '\0';

	return nDstLen;

}

int firewall_port_opt(int port, int open)
{
	char cmd[256] = "";
	int ret;

	if( open )
		sprintf(cmd, "iptables -A INPUT -p tcp --dport %d -j ACCEPT", port);
	else 
		sprintf(cmd, "iptables -D INPUT -p tcp --dport %d -j ACCEPT", port);
	ret = com_system(cmd);
	if( ret ) {
		return -1;
	}

	return 0;
}

int tcp_port_change_handle(int old_port, int new_port)
{
	int ret;

	ret = firewall_port_opt(old_port, 0);
	if( ret < 0 ) {
		DB_ERR("close port error, port:%d, ret:%d", old_port, ret);
	}

	ret = firewall_port_opt(new_port, 1);
	if( ret < 0 ) {
		DB_ERR("open port error, port:%d, ret:%d", new_port, ret);
		return -1;
	}

	return 0;
}

void oc_base64_encode (const char *__restrict in, size_t inlen,
                       char *__restrict out, size_t outlen)
{
	char tmp[BASE64_SID_SIZE] = "";
	unsigned raw = BASE64_ENCODE_RAW_LENGTH(inlen);
	if (outlen < raw+1) {
		snprintf(out, outlen, "(too long data)");
		return;
	}
	
	Base64Encode(in, inlen, tmp);
	//tmp = Base64Encode( in, inlen);
	memcpy(out, tmp, raw );
	//base64_encode_raw((void*)out, inlen, (uint8_t*)in);
	out[raw] = 0;
	//SAFE_FREE(tmp);
	return;
}
					   
// char *calc_safe_id(const uint8_t *data, unsigned size, char *output, unsigned output_size)
// {
// 	uint8_t safe_id[20];

// 	safe_hash(data, size, safe_id);
// 	oc_base64_encode((char*)safe_id, 20, output, output_size);

// 	return output;
// }

void _malloc_free2(void *ctx, void *ptr)
{
	free(ptr);	
}
void *_malloc_size2(void *ctx, size_t size)
{
	return malloc(size);
}

char *malloc_strdup(const char *p)
{
	char *ptr = (char *)malloc(strlen(p)+1);
	if (ptr) 
		memcpy(ptr, p, strlen(p)+1);
	return ptr;
}

char *malloc_strndup(const char *p, int len)
{
	char *ptr = (char *)malloc(len+1);
	if (ptr) {
		memcpy(ptr, p, len);
		ptr[len] = '\0';
	} else {
		DB_ERR("Alloc error:%s, len:%d", strerror(errno), len);
	}
	
	return ptr;
}

void *get_brackets_string1(const char *str)
{
	char *p, *p2;
	unsigned len;

	p = strchr(str, '[');
	if (p == NULL) {
		return NULL;
	}
	p++;
	while (c_isspace(*p))
		p++;

	p2 = strchr(p, ',');
	if (p2 == NULL) {
		p2 = strchr(p, ']');
		if (p2 == NULL) {
			fprintf(stderr, "error parsing %s\n", str);
			exit(1);
		}
	}

	len = p2 - p;

	return malloc_strndup(p, len);
}

/* Returns the number of suboptions processed.
*/
unsigned expand_brackets_string(const char *str, subcfg_val_st out[MAX_SUBOPTIONS])
{
	char *p, *p2, *p3;
	unsigned len, len2;
	unsigned pos = 0, finish = 0;

	if (str == NULL)
	   return 0;

	p = strchr(str, '[');
	if (p == NULL) {
	   return 0;
	}
	p++;
	while (c_isspace(*p))
	   p++;

	do {
	   p2 = strchr(p, '=');
	   if (p2 == NULL) {
		   fprintf(stderr, "error parsing %s\n", str);
		   exit(1);
	   }
	   len = p2 - p;

	   p2++;
	   while (c_isspace(*p2))
		   p2++;

	   p3 = strchr(p2, ',');
	   if (p3 == NULL) {
		   p3 = strchr(p2, ']');
		   if (p3 == NULL) {
			   fprintf(stderr, "error parsing %s\n", str);
			   exit(1);
		   }
		   finish = 1;
	   }
	   len2 = p3 - p2;

	   if (len > 0) {
		   while (c_isspace(p[len-1]))
			   len--;
	   }
	   if (len2 > 0) {
		   while (c_isspace(p2[len2-1]))
			   len2--;
	   }

	   out[pos].name = malloc_strndup(p, len);
	   out[pos].value = malloc_strndup(p2, len2);
	   pos++;
	   p = p2+len2;
	   while (c_isspace(*p)||*p==',')
		   p++;
	} while(finish == 0 && pos < MAX_SUBOPTIONS);

	return pos;
}

void free_expanded_brackets_string(subcfg_val_st out[MAX_SUBOPTIONS], unsigned size)
{
	unsigned i;
	for (i=0;i<size;i++) {
		free(out[i].name);
		free(out[i].value);
	}
}

int
c_strcasecmp (const char *s1, const char *s2)
{
  register const unsigned char *p1 = (const unsigned char *) s1;
  register const unsigned char *p2 = (const unsigned char *) s2;
  unsigned char c1, c2;

  if (p1 == p2)
    return 0;

  do
    {
      c1 = tolower (*p1);
      c2 = tolower (*p2);

      if (c1 == '\0')
        break;

      ++p1;
      ++p2;
    }
  while (c1 == c2);

  if (UCHAR_MAX <= INT_MAX)
    return c1 - c2;
  else
    /* On machines where 'char' and 'int' are types of the same size, the
       difference of two 'unsigned char' values - including the sign bit -
       doesn't fit in an 'int'.  */
    return (c1 > c2 ? 1 : c1 < c2 ? -1 : 0);
}

/******************************************************************************
    Function:       net_get_ip_by_dec
    Description:    将点分十进制ip地址转换为整数型
    Input:
        ip_addr     --点分十进制IP地址
    Output:         无
    Return:
                    --整数型ip地址
    Others:         <其它说明>
 ******************************************************************************/
unsigned int net_ip_string2int(char *ip_addr)
{
	if (ip_addr == NULL || strlen(ip_addr) <= 0) {
		return 0;
	}
    struct in_addr s;

    inet_pton(AF_INET, ip_addr, (void *)&s);

    return s.s_addr;
}

/******************************************************************************
    Function:       net_get_ip_by_net
    Description:    将整数型IP地址转换为点分十进制格式
    Input:
    Output:         无
    Return:
                    --IPV4点分十进制格式
    Others:         <其它说明>
 ******************************************************************************/
#if 0
char *net_ip_int2string(int ip_addr)
{
    struct in_addr s;
    static char buff[IPV4_LEN];

    memset(buff, 0, sizeof(buff));
    s.s_addr = ip_addr;
    inet_ntop(AF_INET, &s.s_addr, buff, INET_ADDRSTRLEN);
    return buff;
}
#endif

const char *net_ip_int2string(unsigned int ip_addr, char *buff)
{
    struct in_addr s;
    s.s_addr = ip_addr;
    return inet_ntop(AF_INET, &s.s_addr, buff, INET_ADDRSTRLEN);
}

char *net_ip6_2string(struct in6_addr *in6)
{
    static char buff[64];

    memset(buff, 0, sizeof(buff));
	inet_ntop(AF_INET6, in6, buff, sizeof(buff)-1);
    return buff;
}

inline unsigned int net_get_mask_by_ip_range(unsigned int start, unsigned int end)
{
	int i = 0;
	unsigned int mask, tmp;

	mask = ~(start^end);
	tmp = mask;
	while (mask & 0x80000000)
	{
		i++;
		mask = (mask << 1);
	}

	return ((tmp >> (32-i)) << (32-i));
}

int com_system(char *cmd_string)
{
    int     ret;
    void    (*wasCld)(), (*wasChld)();
	if (cmd_string == NULL) {
		return -1;
	}

    wasCld  = signal(SIGCLD, SIG_DFL);
    wasChld = signal(SIGCHLD, SIG_DFL);
    ret = system(cmd_string);
    signal(SIGCLD, wasCld);
    signal(SIGCHLD, wasChld);
    return ret;
}

int calc_num_of_1(int in)
{
	int cnt = 0;
	int i;

	for( i = 0; i < sizeof(in) * 8; i++ ) {
		if(in&(1 << i))cnt++;
	}

	return cnt;
}

int com_port_idle_check(int port)
{
	FILE * p_file = NULL;
	char sys_cmd[256];
	char buf[2] = "";

	sprintf(sys_cmd, "netstat -ntlp |grep -w %d", port);
	p_file = popen(sys_cmd, "r");
	if (!p_file) {  
        return -1;  
    }

	fgets(buf, 2, p_file);
	if( buf[0] != '\0' ) {
		pclose(p_file);
		return -1;
	}
	
	return 0;
}

int two_dimen_alloc( size_t row, int column, char ***buffer )
{
	int malloc_size;
	int i;

	malloc_size = sizeof(char) * row;
	*buffer = (char **)malloc(malloc_size);
	if( *buffer == NULL ) {
		DB_ERR("malloc row error:%s, malloc_size:%d!", strerror(errno), malloc_size);
		return -1;
	}

	for( i = 0; i < malloc_size; i++ ) {
		(*buffer)[i] = (char *)malloc(column);
		if( (*buffer)[i] == NULL ) {
			DB_ERR("malloc column error:%s, malloc_size:%d!", strerror(errno), column);
			return -1;
		}
	}

	return 0;
}

/* date格式：2020-02-02-14:14:14 */
void timestamp2date(time_t* tm, char *str_date)
{	
	struct tm *info;
	if( tm <= 0 ) {
		return;
	}
	
	info = gmtime(tm);
	
	//info = gmtime((const time_t*)&tm);
	sprintf(str_date, "%d-%d-%d", info->tm_year+1900,info->tm_mon+1,info->tm_mday);
}

long date2timestamp(char *str_date)
{
	struct tm stm;
	int y,m,d,h,min,s;
	
	y = atoi(str_date);
	m = atoi(str_date+5);
	d = atoi(str_date+8);
	h = atoi(str_date+11);
	min = atoi(str_date+14);
	s = atoi(str_date+17);

	stm.tm_year=y - 1900;  
    stm.tm_mon=m - 1;  
    stm.tm_mday=d;  
    stm.tm_hour=h;  
    stm.tm_min=min;  
    stm.tm_sec=s;  

	return mktime(&stm);
}

time_t get_cur_timestamp()
{
	time_t t;
	t = time(NULL);
	return time(&t);
}

/* 根据接口名获取接口索引 */
/* int comm_get_ifindex_by_ifname(int fd, const char *devname)
{
	struct ifreq ifr;

	if (fd <= 0 || devname == NULL) {
		return -1;
	}

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, devname, sizeof(ifr.ifr_name));
	ifr.ifr_name[sizeof(ifr.ifr_name) -1] = '\0';

	if (ioctl(fd, SIOCGIFINDEX, &ifr) == -1)
	{
		return -1;
	}
	return ifr.ifr_ifindex;
}

unsigned int comm_get_ip_by_ifname(char *ifname)
{
	int sock;
	struct sockaddr_in sin;
	struct ifreq ifreq;

	if (ifname == NULL) {
		return 0;
	}

	if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("socket error");
		return 0;
	}

	strcpy(ifreq.ifr_name, ifname);
	if(ioctl(sock, SIOCGIFADDR, &ifreq) < 0)
	{
		perror("socket error");
		return 0;
	}

	memcpy(&sin, &ifreq.ifr_addr, sizeof(struct sockaddr_in));
	close(sock);
	return sin.sin_addr.s_addr;
}

int comm_set_tun_mtu(const char *name, unsigned mtu)
{
	int fd, ret;
	struct ifreq ifr;

	fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd == -1)
		return -1;

	memset(&ifr, 0, sizeof(ifr));
	strlcpy(ifr.ifr_name, name, IFNAMSIZ);
	ifr.ifr_mtu = mtu;

	ret = ioctl(fd, SIOCSIFMTU, &ifr);
	if (ret != 0) {
		ret = -1;
		goto fail;
	}
	ret = 0;
 fail:
	close(fd);
	return ret;
} */

void comm_create_file(const char *path)
{
    //文件指针
    FILE *filep;
    
    //使用“读入”方式打开文件
    filep = fopen(path, "r");

    //如果文件不存在
    if (filep == NULL)
    {
        //使用“写入”方式创建文件
        filep = fopen(path, "w");
    }
    
    //关闭文件
    fclose(filep);
}

/*
 * 描述：根据接口名获取指定接口的ip地址
 */
/* int comm_get_ipstr_by_ifname(const char *ifName, char *ipAddr, int addrLen)
{
    int ret = -1;
    struct ifreq ifr;
    struct sockaddr_in *sin;
    int sock = -1;

	//参数检测
	if(ipAddr == NULL || ifName == NULL || strlen(ifName)<=0){
		DB_ERR("输入参数错误");
		return -1;
	}
	//
    DB_INFO("get ip by ifname[%s]", ifName);
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    strncpy(ifr.ifr_name, ifName, sizeof(ifr.ifr_name));

    if (0 > (ret = ioctl(sock, SIOCGIFADDR, &ifr)))
    {
        DB_ERR("ioctl SIOCGIFADDR failed:%s", strerror(errno));
        ret = -1;
		goto end;
    }

    sin = (struct sockaddr_in *)&(ifr.ifr_addr);
    if (NULL == inet_ntoa(sin->sin_addr))
    {
    	DB_ERR("inet_ntoa get ip failed!!");
        ret = -1;
		goto end;
    }
    strncpy(ipAddr, inet_ntoa(sin->sin_addr), addrLen);
	ret = 0;
end:
	if(sock >= 0)
		close(sock);
	return ret;
}*/
/*
 * 描述：根据接口名获取指定接口的mask地址
 */
/* int comm_get_maskstr_by_ifname(const char *ifName, char *ipAddr, int addrLen)
{
    int ret = -1;
	struct ifreq ifr;
    struct sockaddr_in *sin;
    int sock = -1;

	//参数检测
	if(ipAddr == NULL || ifName == NULL || strlen(ifName)<=0){
		DB_ERR("输入参数错误");
		return -1;
	}
	//
    DB_INFO("get ip by ifname[%s]", ifName);
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    strncpy(ifr.ifr_name, ifName, sizeof(ifr.ifr_name));

	if(0 > (ret = ioctl(sock, SIOCGIFNETMASK, &ifr)))
	{
        DB_ERR("ioctl SIOCGIFNETMASK failed:%s", strerror(errno));
		ret = -1;
        goto end;
	}

    sin = (struct sockaddr_in *)&(ifr.ifr_netmask);
    if (NULL == inet_ntoa(sin->sin_addr))
    {
    	DB_ERR("inet_ntoa get mask failed!!");
        ret = -1;
		goto end;
    }
    strncpy(ipAddr, inet_ntoa(sin->sin_addr), addrLen);
	ret = 0;
end:
	if(sock >= 0)
		close(sock);
    return ret;
} */
/*
 * 描述：
 *    执行系统命令
 * 输入：
 *    p_system_cmd：系统命令字符串
 * 返回：
 *    成功状态：0，成功；其他，失败
 */
int comm_system(const char *p_system_cmd)
{
    int ret = 0;

    DB_DEBUG("[system]执行：%s。\n", p_system_cmd);
    ret = system(p_system_cmd);
    if(-1 == ret)
    {
        DB_ERR("执行system调用%s失败，error=%d。\n", p_system_cmd, ret);
        return -1;
    }
    else
    {
        if(WIFEXITED(ret))
        {
            ret = WEXITSTATUS(ret);
            if(0 == ret)
            {
                DB_DEBUG("执行system调用%s成功。\n", p_system_cmd);
                return 0;
            }
            else
            {
                DB_ERR("执行system调用%s失败，status=%d。\n", p_system_cmd, ret);
				return ret;
            }
        }
    }
    return -1;
} 

/*
 * 描述：运行外部程序
 */
int comm_exec_progrem(const char *path, char *const argv[])
{
	pid_t pid = 0;
	char *path_tmp = NULL;
	int i = 0;
	
	//参数检查
	if(path == NULL || argv == NULL || argv[0] == NULL || strlen(argv[0]) <= 0){
		DB_ERR("[exec]参数错误！");
		return -1;
	}
	path_tmp = strrchr(path, '/');
	if(path_tmp == NULL || strcmp(path_tmp + 1, argv[0]))
	{
		DB_ERR("[%s]必须以程序名结尾,且第一个参数[%s]必须为程序名！", path, argv[0]);
		return -1;
	}
	//
	i = 0;
	DB_INFO("progrem path:%s", path);
	while(argv[i] != NULL){
		if(strlen(argv[i]) <= 0 || i > 20){
			DB_ERR("第[%d]参数为空字符！", i);
			return -1;
		}
		DB_INFO("argv[%d]:%s", i, argv[i]);
		i ++;
	}	
	//
	pid = vfork();
	if(pid == 0){
		int fdTableSize = getdtablesize();
		for(i=3;i<fdTableSize;i++){
				close(i);
		}
		execv(path,argv);
		exit(0);
	}
	return 0;
}




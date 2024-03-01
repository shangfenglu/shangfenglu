/*
 * =====================================================================================
 *
 *       Filename:  common.h
 *
 *    Description:  公共处理头文件
 *
 *        Version:  1.0
 *        Created:  2023年7月1日 04时39分38秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  
 *        Company:  am
 *
 * =====================================================================================
 */

#ifndef __COMMON_H__
#define __COMMON_H__

#include <string.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <sys/ioctl.h>


#ifndef ETIMEDOUT
#define ETIMEDOUT 138
#endif

#ifndef safe_free
#define safe_free(x, f) if(x){f(x);x=NULL;}
#endif

#define BIT(x) (1 << (x))
#define SETBIT(addr, bit) (addr |= (1<<bit))
#define CLEARBIT(addr, bit) (addr &= ~(1<<bit))
#define CHECKBIT(addr, bit) (addr & (1<<bit))

enum{
	IPV4_LEN = 16,
	IPV6_LEN = 46
};

#ifndef IPSTR
#define IPSTR					"%u.%u.%u.%u"
#define IP2STR(a)				(((a) >> 24) & 0xff), (((a) >> 16) & 0xff), \
									(((a) >> 8) & 0xff), ((a) & 0xff)
#define NETIP2STR(a)			((a) & 0xff), (((a) >> 8) & 0xff), \
									(((a) >> 16) & 0xff), (((a) >> 24) & 0xff)
#endif

/* Helper casts */
#define SA_IN_P(p) (&((struct sockaddr_in *)(p))->sin_addr)
#define SA_IN_U8_P(p) ((uint8_t*)(&((struct sockaddr_in *)(p))->sin_addr))
#define SA_IN6_P(p) (&((struct sockaddr_in6 *)(p))->sin6_addr)
#define SA_IN6_U8_P(p) ((uint8_t*)(&((struct sockaddr_in6 *)(p))->sin6_addr))

#define SA_IN_PORT(p) (((struct sockaddr_in *)(p))->sin_port)
#define SA_IN6_PORT(p) (((struct sockaddr_in6 *)(p))->sin6_port)

#define SA_IN_P_GENERIC(addr, size) ((size==sizeof(struct sockaddr_in))?SA_IN_U8_P(addr):SA_IN6_U8_P(addr))
#define SA_IN_P_TYPE(addr, type) ((type==AF_INET)?SA_IN_U8_P(addr):SA_IN6_U8_P(addr))
#define SA_IN_SIZE(size) ((size==sizeof(struct sockaddr_in))?sizeof(struct in_addr):sizeof(struct in6_addr))
#define BASE64_ENCODE_RAW_LENGTH(length) ((((length) + 2)/3)*4)

#define MAX_WAIT_SECS 3

#define ERR_BAD_COMMAND -2
#define ERR_PEER_TERMINATED -11

#define MAX_IP_STR 46
#define MAX_ID_STR 20
#define MAX_DATE_STR 20
#define MAX_AGENT_NAME 64
#define SAFE_ID_SIZE (BASE64_ENCODE_RAW_LENGTH(20)+1)
#define MAX_CMD_LEN 256
#define MAX_CMD_RES_LEN 256
#define MAX_SUBOPTIONS 5

#define MAX_GROUPNAME_SIZE MAX_USERNAME_SIZE
#define MAX_GROUPS 32
#define PARAM_MEX_SIZE 32

#define SID_SIZE 	32
#define BASE64_SID_SIZE 45
#define CONTENT_TYPE_SIZE 256

#define COOKIE_HEADER "zcwsessionid="
#define COOKIE_HEADER_SIZE 13 /* sizeof  COOKIE_HEADER*/


#ifndef MIN
# define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

#ifndef HAVE_STRLCPY
size_t oc_strlcpy(char *dst, char const *src, size_t siz);
# define strlcpy oc_strlcpy
#endif

#define SAFE_FREE(p, f) do{ \
						if( p != NULL ) { \
							f(p); \
							p = NULL; \
						} \
					}while(0)
						
#define SAFE_FREE_DP(p,l) do{ \
						int i; \
						for(i = l - 1; i >= 0; i--) { \
							SAFE_FREE(p[i], free); \
						} \
					}while(0)
/* 目前定义了work到main */
typedef enum {
	FUNC_NONE = 0,
	FUNC_ADD,
	FUNC_DELETE
} func_rpc_proto;

/* 目前定义了work到main */
typedef enum {
	CMD_W2M_USER_STATE = 1,
	CMD_W2M_CONNECT
} cmd_rpc_proto;

				
/* IPC protocol commands */
typedef enum {
	AUTH_COOKIE_REP = 2,
	AUTH_COOKIE_REQ = 4,

	CMD_COSIGN_EQ_REQ,
	CMD_COSIGN_EQ_REP,
	/* from worker to sec-mod */
	CMD_SEC_AUTH_INIT = 120,
	CMD_SEC_AUTH_REPLY,

	/* from main to sec-mod and vice versa */
	MIN_SECM_CMD=239,
	CMD_SECM_SESSION_OPEN, /* sync: reply is CMD_SECM_SESSION_REPLY */
	CMD_SECM_SESSION_CLOSE, /* sync: reply is CMD_SECM_CLI_STATS */
	CMD_SECM_SESSION_REPLY,
	CMD_SECM_BAN_IP,
	CMD_SECM_BAN_IP_REPLY,

} cmd_request_t;

typedef struct subcfg_val_st {
	char *name;
	char *value;
} subcfg_val_st;

inline static
char c_isspace (int c)
{
  switch (c)
    {
    case ' ': case '\t': case '\n': case '\v': case '\f': case '\r':
      return 1;
    default:
      return 0;
    }
}


inline static
void safe_memset(void *data, int c, size_t size)
{
	volatile unsigned volatile_zero = 0;
	volatile char *vdata = (volatile char*)data;

	/* This is based on a nice trick for safe memset,
	 * sent by David Jacobson in the openssl-dev mailing list.
	 */

	if (size > 0)
		do {
			memset(data, c, size);
		} while(vdata[volatile_zero] != c);
}

char *comm_strrchr(const char *src, char c);
ssize_t force_read_timeout(int sockfd, void *buf, size_t len, unsigned sec);
char *Base64Encode(const char *buf, const long size, char *base64Char);
char *Base64Decode(const char *base64Char, const long base64CharSize, char *originChar, long originCharSize);
int EncodeBase64(const unsigned char* pSrc, char* pDst, int nSrcLen);
int DecodeBase64(const char* pSrc, unsigned char* pDst, int nSrcLen);
char *malloc_strdup(const char *p);
char *malloc_strndup(const char *p, int len);
void *get_brackets_string1(const char *str);
int cmd_request_to_str(unsigned _cmd, char *out, int len);
unsigned expand_brackets_string(const char *str, subcfg_val_st out[MAX_SUBOPTIONS]);
void free_expanded_brackets_string(subcfg_val_st out[MAX_SUBOPTIONS], unsigned size);
int c_strcasecmp (const char *s1, const char *s2);
//char *calc_safe_id(const uint8_t *data, unsigned size, char *output, unsigned output_size);
unsigned int net_ip_string2int(char *ip_addr);
const char *net_ip_int2string(unsigned int ip_addr, char *buff);
char *net_ip6_2string(struct in6_addr *in6);
inline unsigned int net_get_mask_by_ip_range(unsigned int start, unsigned int end);
int com_system(char *cmd_string);
int calc_num_of_1(int in);
int two_dimen_alloc( size_t row, int column, char ***buffer );
void timestamp2date(time_t* tm, char *date);
long date2timestamp(char *str_date);
time_t get_cur_timestamp();
int comm_get_ifindex_by_ifname(int fd, const char *devname);
unsigned int comm_get_ip_by_ifname(char *ifname);
int comm_set_tun_mtu(const char *name, unsigned mtu);
int firewall_port_opt(int port, int open);
int tcp_port_change_handle(int old_port, int new_port);
int com_port_idle_check(int port);
void comm_create_file(const char *path);
int comm_get_maskstr_by_ifname(const char *ifName, char *ipAddr, int addrLen);
int comm_get_ipstr_by_ifname(const char *ifName, char *ipAddr, int addrLen);


int comm_system(const char *p_system_cmd);
int comm_exec_progrem(const char *path, char *const argv[]);

#endif

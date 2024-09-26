#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/route.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <sys/time.h>
#include <linux/netlink.h>

ssize_t sock_readt(int fd,void* buf,size_t n,int timeout)
{
    //set socket timeout option
    if(timeout >= 0){
        struct timeval tv;
        tv.tv_sec  = timeout;
        tv.tv_usec = 0;
        setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(struct timeval));
    }

    return read(fd,buf,n);
}

ssize_t sock_readn(int fd,void* buf,size_t n)
{
    ssize_t nleft;
    ssize_t nread;
    char* ptr;

    ptr = (char*)buf;
    nleft = n;
    while(nleft > 0){
        if((nread = sock_readt(fd,ptr,nleft,0)) < 0){
            if(errno == EINTR){
                nread = 0;
                continue;
            }
            return -1;
        }else if(nread == 0){
            break;
        }
        
        nleft -= nread;
        ptr += nread;
    }

    return (n - nleft);
}

ssize_t socket_writen(int fd,const void* buf,size_t n)
{
    size_t nleft;
    ssize_t writen;
    const char* ptr = (char*)buf;

    nleft = n;
    while(nleft > 0){
        if((writen = write(fd,ptr,nleft)) <= 0){
            if(writen < 0 && errno == EINTR){
                writen = 0;
                continue;
            }
            return -1;
        }

        nleft -= writen;
        ptr += writen;
    }

    return n;
}

int sock_make_nonblock(int fd)
{
    int flags = fcntl(fd,F_GETFL);
    if(flags < 0){
        return -1;
    }

    flags |= O_NONBLOCK;
    return fcntl(fd,F_SETFL,flags);
}

int sock_fd_is_closed(int fd)
{
    int optval = 0;
    int optlen = 0;

    optlen = sizeof(optval);
    return getsockopt(fd, SOL_SOCKET, SO_TYPE, &optval, (socklen_t *)&optlen);
}

int sock_fd_connect(int sockfd,const char* hostname,unsigned short port)
{
    int ret = 0;
    char servname[10];
    struct addrinfo hints;
    struct addrinfo *res;
    struct addrinfo *ai;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    snprintf(servname, sizeof(servname), "%u", port);
    if(getaddrinfo(hostname, servname, &hints, &res) != 0){
        perror("getaddrinfo");
        return -1;
    }

    ret = -1;
    for(ai=res; ai!=NULL; ai=ai->ai_next){
        ret = connect(sockfd,ai->ai_addr,ai->ai_addrlen);
        if(ret != -1){
            break;
        }
    }
    freeifaddrs(res);

    //take EINPROGRESS as ETIMEOUT
    if((ret != 0) && (errno == EINPROGRESS)){
        errno = ETIMEDOUT;
    }

    return (ret != 0) ? -1 : 0;
}

int close_fd(int* fd)
{
    int ret = -1;

    if(fd && *fd > 0){
        if((ret = close(*fd)) == 0){
            *fd = -1;
        }else {
            perror("close");
        }
    }

    return ret;
}

int net_get_ethip(const char* ethname,uint32_t* ethip)
{
    int fd;
    int errcode;
    struct ifreq ifr;

    if(!ethname || !ethip){
        return -1;
    }
        
    errcode = -1;
    if((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
        return errcode;
    }

    memset(&ifr, 0, sizeof(struct ifreq));
    ifr.ifr_addr.sa_family = AF_INET;
    snprintf(ifr.ifr_name,IF_NAMESIZE,ethname);
    if(!ioctl(fd, SIOCGIFADDR, &ifr)){
        *ethip = ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr.s_addr;
        errcode = 0;
    }
    close(fd);
    return errcode;
}

//just for debug only
void net_dump_ethip(void)
{
    int i;
    unsigned int ip;
    cahr* ethname[] = {"eth0","eth1","eth2","eth3","eth4","eth5","eth6","eth7"};
    cahr ipnum[32];

    for(i = 0; i < 8; i++){
        printf("%s: ",ethname[i]);
        if(net_get_ethip(ethname[i],&ip) == 0){
            inet_itoa(ipnum,sizeof(ipnum),ip);
            printf("%s:%s (%u)\n",ethname[i],ipnum,ip);
        }
    }
    return ;
}

#undef ROUTE_ADD
#undef ROUTE_DEL
#define ROUTE_ADD  1
#define ROUTE_DEL  2

static inline void net_resolve(struct sockaddr_in* s_addr,uint32_t addr)
{
    s_addr->sin_family = AF_INET;
    s_addr->sin_addr.s_addr = htonl(addr);
    return ;
}

static int net_route(int cmd,uint32_t dst,uint32_t gw)
{
    int err;
    int skfd;
    char rt_buf[sizeof(struct rtentry)];
    struct rtentry* ret = (struct rtentry*)rt_buf;

    memset(ret,0,sizeof(struct rtentry));
    net_resolve((struct sockaddr_in*)&ret->rt_dst, dst);
    net_resolve((struct sockaddr_in*)&ret->rt_gateway, gw);
    net_resolve((struct sockaddr_in*)&ret->rt_genmask, 0xffffffff);
    rt->rt_flags = RTF_HOST;

    skfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(cmd == ROUTE_ADD){
        err = ioctl(skfd, SIOCADDRT, rt);
    }else {
        err = ioctl(skfd, SIOCDELRT, rt);
    }
    close(skfd);
    if(err){
        perror("ioctl");
    }
    return err;
}

int net_route_add(uint32_t dst,uint32_t gw)
{
    if(gw == 0 || ((dst >> 24) == 127)){
        return 0;
        
    }
    net_route(ROUTE_DEL,dst,0);
    return net_route(ROUTE_ADD,dst,gw);
}

int net_route_del(uint32_t dst)
{
    return net_route(ROUTE_DEL,dst,0);
}

#define BUFSIZE 8192
struct route_info{
    struct in_addr dstAddr;
    struct in_addr srcAddr;
    struct in_addr gateWay;
    char ifName[IF_NAMESIZE];
};

#ifndef DEBUG3
#define DEBUG3(fmt, ...) do {printf(fmt, ##__VA_ARGS__);} while(0) 
#endif

static int nl_readNlSock(int sockFd,char* bufPtr,int seqNum,int pId)
{
    struct nlmsghdr* nlHdr;
    int readLen = 0;
    int msgLen = 0;

    do{
        if (msgLen >= BUFSIZE){
            break;
        }

        //recieve respinst from the kernel
        if ((readLen = recv(sockFd,bufPtr,BUFSIZE - msgLen, 0)) < 0){
            perror("recv");
            return -1;
        }

        nlHdr = (struct nlmsghdr*)bufPtr;

        //check if the header is vaild
        if((NLMSG_OK(nlHdr,(unsigned int )readLen) == 0) || (nlHdr->nlmsg_type == NLMSG_DONE)){
            perror("Error in received packet");
            return -1;
        }

        //check if the its the last message
        if(nHdr->nlmsg_type == NLMSG_DONE){
            break;
        }else {
            bugPtr += readLen;
            msgLen += readLen;
        }

        //check if its a multi part message 
        if((nlHdr->nlmsg_flags & NLM_F_MULTI) == 0){
            break;
        }

    }while((nlHdr->nlmsg_seq != (unsigned int)seqNum) 
    || (nlHdr->nlmsg_pid != (unsigned int)pId));

    return msgLen;
}

//for printing ther routes
static void nl_printRoute(struct route_info* info)
{
    char tempBuf[512];
    
    //print destination address
    if(info->dstAddr.s_addr != 0){
        strcpy(tempBuf,inet_ntoa(info->dstAddr))
    }else{
        sprintf(tempBuf,"*.*.*.*\t");
    }
    DEBUG3("%s\t",tempBuf);

    //print Gatewat address
    if(info->gateWay.s_addr != 0){
        strcpy(tempBuf,(char*) inet_ntoa(info->gateWay));
    }else{
        sprintf(tempBuf,"*.*.*.*\t");
    }
    DEBUG3("%s\t",tempBuf);
    
    //print interface name
    DEBUG3("%s\n",info->ifName);

    //print source address
    if(info->srcAddr.s_addr != 0){
        strcpy(tempBuf,inet_ntoa(info->srcAddr));
    }else {
        sprintf(tempBuf,"*.*.*.*\t");
    }
    DEBUG3("%s\t",tempBuf);

    return ;
}

//for parsing the route info retruned
static void nl_parseRoute(struct nlmsghdr* nlHdr,struct route_info* rtInfo)
{
    int rtLen = 0;
    struct rtmsg* rtMsg;
    struct rtattr* rtAttr;

    rtMsg = (struct rtmsg*)NLMSG_DATA(nlHdr);
    if(rtMsg->rtm_family != AF_INET){
        return ;
    }

    rtAddr = (struct rtattr*)RTM_RTA(rtMsg);
    rtLen = RTM_PAYLOAD(nlHdr);
    fo(;RTA_OK(rtAttr,rtLen);rtAttr = RTA_NEXT(rtAttr,rtLen)){
        switch(rtAttr->rta_type){
            case RTA_OIF:
                if_indextoname(*(int*)RTA_DATA(rtAttr),rtInfo->ifName);
                break;
            case RTA_GATEWAY:
                rtInfo->gateWay.s_addr = *(int*)RTA_DATA(rtAttr);
                break;
            case RTA_PREFSRC:
                rtInfo->srcAddr.s_addr = *(int*)RTA_DATA(rtAttr);
                break;
            case RTA_DST:
                rtInfo->dstAddr.s_addr = *(int*)RTA_DATA(rtAttr);
                break;
        }
    }
    nl_printRoute(rtInfo);
    return ;
}

int netlink_get_iface_getway(char* ifname,char* gwip,int size)
{
    struct nlmsghdr* nlMsg;
    struct rtmsg* rtMsg;
    struct route_info rtInfo;
    char msgBuf[BUFSIZE];
    int sock = 0;
    int len = 0;
    int msgSeq = 0;

    if((sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE)) < 0){
        perror("socket");
        return -1;
    }
    
    memset(msgBuf,0,BUFSIZE);

    //point the header and the msg structure pointers into thee buffer
    nlMsg = (struct nlmsghdr*)msgBuf;
    rtMsg = (struct rtmsg*)NLMSG_DATA(mlMsg);
    rtMsg->rtm_family = 2;
    rtMsg->rtm_table = 100;

    //fill inthe nlmsg header
    nlMsg->nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg));
    nlMsg->nlmsg_type = RTM_GETROUTE;

    nlMsg->nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
    nlMsg->nlmsg_seq = msgSeq++;
    nlMsg->nlmsg_pid = getpid();

    //send the request
    if(send(sock,nlMsg,nlMsg->nlmsg_len,0) < 0){
        close(sock);
        printf("Write to socket failed");
        return -1;
    }

    //read the response
    if((len = nl_readNlSock(sock,msgBuf,msgSeq,getpid())) < 0){
        close(sock);
        printf("read from socket failed");
        return -1;
    }

    close(sock);

    //parse and print the response
    DEBUG3("Destination\tGateway\tInterface\n");
    for(;NLMSG_OK(nlMsg,(unsigned int)len);nlMsg = NLMSG_NEXT(nlMsg,len)){
        memset(&rtInfo,0,sizeof(struct route_info));
        nl_parseRoute(nlMsg,&rtInfo);
        if(strcmp(rtInfo.ifName,ifname) == 0 && rtInfo.gateWay.s_addr != 0){
            inet_ntop(AF_INET,(void*)&(rtInfo.gateWay.s_addr),gwip,size);
            break;
        }
    }

    return 0;
}

int sock_unix_listen(const char* hostname)
{
    int sockfd = -1;
    struct sockaddr_un addr;

    if(strlen(hostname) > sizeof(addr.sun_path)){
        return -1;
    }

    memset(&addr,0,sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path,hostname);

    sockfd = socket(AF_UNIX,SOCK_STREAM,0);
    if(sockfd < 0){
        perror("socket");
        return -1;
    }

    if(bind(sockfd,(const struct sockaddr*)&addr,(socklen_t)sizeof(addr)) < 0){
        perror("bind");
        close(sockfd);
        return -1;
    }

    if(listen(sockfd,10) < 0){
        perror("listen");
        close(sockfd);
        return -1;
    }

    return sockfd;
}

static int sock_nonlocal(struct addrinfo* ai)
{
    int sockfd;
    int option_value = 1;

    if((sockfd = socket(ai->ai_family,ai->ai_socktype,ai->ai_protocol)) < 0){
        perror("socket");
        return -1;
    }

    setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&option_value,sizeof(option_value));
    if(bind(sockfd,ai->ai_addr,ai->ai_addrlen) < 0){
        if(errno == EADDRNOTAVAIL){
            close(sockfd);
            return -1;
        }
    }

    close(sockfd);
    return 0;
}

static int sock_inet_listen(struct addrinfo* ai)
{
    int sockfd = -1;
    int option_value = 1;
    int transparent = 0;

    sockfd = socket(ai->ai_family,ai->ai_socktype,ai->ai_protocol);
    if(sockfd == -1){
        perror("socket");
        return -1;
    }

    if(sock_nonlocal(ai)){
        
    }
}

int sock_tcp_listen(const char* hostname,unsigned short port)
{
    int sockfd;
    int status;
    struct addrinfo hints;
    struct addrinfo *res;
    struct addrinfo* ai;
    char* host = NULL;
    char servname[8];

    //支持0.0.0.0的地址格式，当指定为0.0.0.0时，表示监听任意地址
    if(strncmp(hostname,"0.0.0.0",strlen("0.0.0.0")) != 0 &&
        strncmp(hostname,"*",strlen("*")) != 0){
            host = strdup(hostname);
    }

    memset(&hints,0,sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_family = AF_UNSPEC;
    hints.ai_flags |= AI_PASSIVE;

    snprintf(servname,sizeof(servname),"%d",port);
    status = getaddrinfo(host,servname,&hints,&res);
    if(host){
        free(host);
    }

    if(status != 0){
        perror("getaddrinfo");
        return -1;
    }

    sockfd = -1;
    if(sockfd < 0){
        for(ai = res; ai; ai = ai->ai_next){
            if(ai->ai_family == AF_INET){
                if((sockfd = sock_inet_listen(ai) > 0)){
                    break;
                }
            }
        }
    }

    freeaddrinfo(res);
    return (sockfd);
}

static int sock_tcp_listen_inet4(const char* hostname,unsigned short port)
{
    int fd;
    int sockfd;
    int status;
    char servname[10];
    struct addrinfo hints;
    struct addrinfo* res;
    struct addrinfo* ai;
    int transparent;
    int option_value;

    memset(&hints,0,sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    
}
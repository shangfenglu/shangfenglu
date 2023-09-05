#include "../include/ftp_proxy.h"
#include "../include/signal.h"

int main(int argc,char* argv[])
{
	if (argc != 2) {
		printf("please enter the server_ip\n");
		return;
	}

	printf("version is 6.8\n");
	
	/*初始化全局变量*/
	init_global_state(argv);

	/*信号处理*/
	init_signal();

	/*开启代理服务*/
	start_proxy_service();

	return 0;
}
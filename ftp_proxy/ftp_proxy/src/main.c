#include "ftp_proxy.h"
#include <stdio.h>

int main(int argc, char* argv[])
{
	if (2 != argc) {
		printf("please enter the server_ip\n");
		return -1;
	}
	printf("version is 9.9\n");

	/*初始化全局变量*/
	init_global_state(argv);

	/*信号处理*/
	init_signal();

	/*开启代理服务*/
	start_proxy_service();

	return 0;
}
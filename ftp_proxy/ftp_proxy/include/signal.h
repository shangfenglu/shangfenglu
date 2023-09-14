#ifndef __SIGNAL_
#define __SIGNAL_

#include <signal.h>
#define CLEAR_USER 35

//初始化信号
int init_signal();

//SIGINT信号处理
void sigint_handle(int n);

//SIGQUIT信号处理
void sigquit_handle(int n);

//子进程清理信号
void sigclear_handle(int n);

#endif
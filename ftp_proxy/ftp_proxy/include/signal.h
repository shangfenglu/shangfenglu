#ifndef __SIGNAL_
#define __SIGNAL_

//初始化信号
int init_signal();

//SIGINT信号处理
void sigint_handle(int n);

//SIGQUIT信号处理
void sigquit_handle(int n);



#endif
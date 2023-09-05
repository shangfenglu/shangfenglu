#include "../include/signal.h"
#include "../include/ftp_proxy.h"

#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>



int init_signal()
{
	int res = -1;
	res = signal(2, sigint_handle);
	res = signal(3, sigquit_handle);

	return 0;
}

void sigint_handle(int n)
{
	global_state.keep_on = 0;
}

void sigquit_handle(int n)
{
	global_state.keep_on = 0;
}

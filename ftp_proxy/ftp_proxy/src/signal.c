#include "signal.h"
#include "session.h"
#include "utils.h"

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <wait.h>

int init_signal()
{
	int res = -1;
	res = signal(2, sigint_handle);
	res = signal(3, sigquit_handle);
	res = signal(CLEAR_USER, sigclear_handle);
	return 0;
}

void sigint_handle(int n)
{
	if (global_state.father_pid == getpid()) {
		close(global_state.server_socket);
		global_state.keep_on = 0;
	}
	else
		global_state.child_keep_on = 0;
}

void sigquit_handle(int n)
{
	if (global_state.father_pid == getpid()) {
		close(global_state.server_socket);
		global_state.keep_on = 0;
	}
	else
		global_state.child_keep_on = 0;
}

void sigclear_handle(int n)
{
	int status = 0;
	pid_t child = wait(&status);
	del_child_pid(global_state.child_pids, 1024, child);
}

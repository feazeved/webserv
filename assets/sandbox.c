// Allowed functions: fork, waitpid, exit, alarm, sigaction, kill, printf, strsignal,
// errno, sigaddset, sigemptyset, sigfillset, sigdelset, sigismember
// --------------------------------------------------------------------------------------

// This function must test if the function f is a nice function or a bad function, you
// will return 1 if f is nice, 0 if f is bad or -1 in case of an error in your function.

// A function is considered bad if it is terminated or stopped by a signal (segfault, abort...),
// if it exit with any other exit code than 0 or if it times out.

// If verbose is true, you must write the appropriate message among the following:
// "Nice function!\n"
// "Bad function: exited with code <exit_code>\n"
// "Bad function: <signal description>\n"
// "Bad function: timed out after <timeout> seconds\n"

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>

static sig_atomic_t	s_timed_out = 0;

void fn_timeout(int value)
{
	(void) value;
	s_timed_out = 1;
}

static
int	s_kill(pid_t pid, size_t numTries)
{
	int	status;

	kill(pid, SIGKILL);
	while (numTries > 0 && waitpid(pid, &status, NULL) == -1)
		numTries--;
	return (numTries == 0);
}

int sandbox(void (*f)(void), unsigned int timeout, bool verbose)
{
	struct sigaction	sa;
	pid_t				pid;
	int					status;
	int					waitStatus;

	sa.sa_handler = fn_timeout;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGALRM, &sa, NULL);

	pid = fork();
	if (pid == -1)
		return (-1);
	if (pid == 0)
	{
		f();
		exit(0);
	}
	alarm(timeout);
	waitStatus = waitpid(pid, &status, WUNTRACED);
	alarm(0);
	if (waitStatus == -1 || WIFSTOPPED(status))
		s_kill(pid, 16);

	if (s_timed_out == 1)
	{
		if (verbose)
			printf("Bad function: timed out after %u seconds\n", timeout);
		return (0);
	}
	if (WIFEXITED(status))
	{
		if (WEXITSTATUS(status) == 0)
		{
			if (verbose)
				printf("Nice function!\n", WEXITSTATUS(status));
			return (1);
		}
		else
		{
			if (verbose)
				printf("Bad function: exited with code %d\n", WEXITSTATUS(status));
			return (0);
		}
	}
	if (WIFSIGNALED(status))
	{
		if (verbose)
			printf("Bad function: %s\n", strsignal(WTERMSIG(status)));
		return (0);		
	}
	if (WIFSTOPPED(status))
	{
		if (verbose)
			printf("Bad function: %s\n", strsignal(WSTOPSIG(status)));
		return (0);
	}
	return (-1);
}

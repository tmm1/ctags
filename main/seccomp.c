#include "seccomp-bpf.h"
#include <signal.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <sys/prctl.h>
#include <unistd.h>

#include <stdlib.h>
#include <sys/ucontext.h>
#include "config.h"

int install_syscall_filter(void)
{
	struct sock_filter filter[] = {
		VALIDATE_ARCHITECTURE,
		EXAMINE_SYSCALL,
		ALLOW_SYSCALL(rt_sigreturn),
#ifdef __NR_sigreturn
		ALLOW_SYSCALL(sigreturn),
#endif
		ALLOW_SYSCALL(exit_group),
		ALLOW_SYSCALL(exit),
		ALLOW_SYSCALL(read),
		ALLOW_SYSCALL(write),
		ALLOW_SYSCALL(fstat),
		ALLOW_SYSCALL(mmap),
		ALLOW_SYSCALL(rt_sigprocmask),
		ALLOW_SYSCALL(rt_sigaction),
		ALLOW_SYSCALL(nanosleep),
		ALLOW_SYSCALL(lseek),
		KILL_PROCESS,
	};
	struct sock_fprog prog = {
		.len = (unsigned short)(sizeof(filter)/sizeof(filter[0])),
		.filter = filter,
	};

	if (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0)) {
		perror("prctl(NO_NEW_PRIVS)");
		goto failed;
	}
	if (prctl(PR_SET_SECCOMP, SECCOMP_MODE_FILTER, &prog)) {
		perror("prctl(SECCOMP)");
		goto failed;
	}
	return 0;

failed:
	if (errno == EINVAL)
		fprintf(stderr, "SECCOMP_FILTER is not available. :(\n");
	return 1;
}

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>

int main(int argc, char *argv[])
{
	int sv[2];
	char buf[128];
	pid_t pid;

	if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv) == -1) {
		fprintf(stderr, "Create socket pair failed:%s\n", strerror(errno));
		exit(-1);
	}

	if ((pid = fork()) == 0) { // Child
		read(sv[0], buf, 128);
		fprintf(stdout, "Child: RECV: %s\n", buf);
		strcpy(buf, "from child");
		write(sv[0], buf, strlen(buf) + 1);
	} else if (pid > 0) { // parent
		write(sv[1], "from parent", strlen("from parent") + 1);
		read(sv[1], buf, 128);
		fprintf(stdout, "Parent: RECV: %s\n", buf);

		wait(NULL);
	}

	return 0;
}

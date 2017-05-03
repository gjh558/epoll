#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

int main(int argc, char *argv[])
{
	int fd;
	int res;
	struct sockaddr_un addr;
	char buf[] = "Hello World";
	int len;

	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, "./mysocket");

	fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (fd < 0) {
		fprintf(stderr, "Failed to create socket: %s\n", strerror(errno));
		exit(-1);
	}

	len =sizeof(addr);
	if (connect(fd, (struct sockaddr *)&addr, len) == -1) {
		fprintf(stderr, "Connect failed: %s\n", strerror(errno));
		exit(-1);
	}

	//fprintf(stdout, "Connected\n");

	res = send(fd, buf, strlen(buf) + 1, 0);
	if (res <= 0) {
		fprintf(stderr, "Send failed: %s\n", strerror(errno));
		exit(-1);
	}

	res = recv(fd, buf, sizeof(buf), 0);
	if (res <= 0) {
		fprintf(stderr, "Recv failed: %s\n", strerror(errno));
		exit(-1);
	}

	fprintf(stdout, "%s\n", buf);
	close(fd);

	return 0;
}

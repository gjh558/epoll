#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

int main(int argc, char *argv[])
{
	int res;
	struct sockaddr_un localaddr, remoteaddr;
	int len;
	int listenfd, connfd;
	char buf[1024];

	listenfd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (listenfd < 0) {
		fprintf(stderr, "Failed to create socket: %s\n", strerror(errno));
		exit(-1);
	}

	localaddr.sun_family = AF_UNIX;
	strcpy(localaddr.sun_path, "./mysocket");
	unlink(localaddr.sun_path);

	len = sizeof(localaddr);

	res = bind(listenfd, (struct sockaddr *)&localaddr, len);
	if (res != 0) {
		fprintf(stderr, "Failed to bind: %s\n", strerror(errno));
		exit(-1);
	}

	listen(listenfd, 5);

	connfd = accept(listenfd, (struct sockaddr *)&remoteaddr, &len);
	if (connfd < 0) {
		fprintf(stderr, "Failed to accept: %s\n", strerror(errno));
		exit(-1);
	}

	res = recv(connfd, buf, sizeof(buf), 0);
	if (res <= 0) {
		fprintf(stderr, "RECV: Error occur on the connection: %s\n", strerror(errno));
		exit(-1);
	}

	res = send(connfd, buf, res, 0);
	if (res <= 0) {
		fprintf(stderr, "SEND: Error occur on the connection: %s\n", strerror(errno));
		exit(-1);
	}

	close(connfd);

	close(listenfd);

	return 0;
}

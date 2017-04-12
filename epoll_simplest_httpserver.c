#include <stdio.h>  
#include <sys/stat.h>  
#include <sys/socket.h>  
#include <netinet/in.h>
#include <fcntl.h>  
#include <errno.h>  
#include <stddef.h>  
#include <string.h>
#include <sys/epoll.h>
#include <stdlib.h>

#define PORT 1989
#define MSG "HTTP/1.1 200 OK\n\nhello yo!\n"
#define LEN strlen(MSG)

void on_read(int fd, int len, char *buffer) {
	struct sockaddr_in addr;
	int length;
	char ipAddr[512];
	buffer[len] = '\0';
	getpeername(fd, (struct sockaddr *)&addr, &length);
	inet_ntop(AF_INET, &addr.sin_addr, ipAddr, sizeof(ipAddr));
	fprintf(stdout, "---->receive from %s, length %d:\n%s\n", ipAddr, len, buffer);

}

void on_write(int fd) {
	//send data
	int res = write(fd, MSG, LEN);
	if (res < 0) {
		fprintf(stderr, "send data failed: %s\n", strerror(errno));
		exit(-1);
	}
	fprintf(stdout, "---->sent to %d: length : %d\n%s\n", fd, LEN, MSG);
}

int main()
{
	int i = 0;
	int epollfd;
	struct epoll_event ev, events[20];

	struct sockaddr_in addr;
	struct sockaddr_in client_addr;
	int client_len;

	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0) {
		fprintf(stderr, "create socket failed: %s\n", strerror(errno));
		exit(-1);
	}

	int flag;
	setsockopt(sockfd,SOL_SOCKET ,SO_REUSEPORT,&flag,sizeof(int));
	//set sockfd nonblock
	fcntl(sockfd, F_SETFL, O_NONBLOCK);

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(PORT);

	if (bind(sockfd, (struct sockaddr *)&addr, sizeof(struct sockaddr)) == -1) {
		fprintf(stderr, "bind failed: %s\n", strerror(errno));
		exit(-1);
	}

	listen(sockfd, 10);

	epollfd = epoll_create(10);
	if (epollfd == -1) {
		fprintf(stderr, "create epoll failed: %s\n", strerror(errno));
		exit(-1);
	}

	ev.data.fd = sockfd;
	ev.events = EPOLLIN;// | EPOLLET;
	epoll_ctl(epollfd, EPOLL_CTL_ADD, sockfd, &ev);

	for(;;) {
		int nfd = epoll_wait(epollfd, events, 20, -1);
		for (i = 0; i < nfd; ++i) {
			if (events[i].data.fd == sockfd) { // New connection 
				int connfd = accept(sockfd, (struct sockaddr *) & client_addr, &client_len);
				if (connfd < 0) {
					fprintf(stderr, "Failed to accept: %s\n", strerror(errno));
					continue;
				}

				fcntl(connfd, F_SETFL, O_NONBLOCK);

				ev.data.fd = connfd;
				ev.events = EPOLLIN;// | EPOLLET;
				epoll_ctl(epollfd, EPOLL_CTL_ADD, connfd, &ev);
			} else if (events[i].events & EPOLLIN) { // There is data recved
				int fd = events[i].data.fd;
				char buffer[1024];
				int readed = read(fd, buffer, 1024, 0);
				if (errno == EAGAIN) {
					fprintf(stderr, "No data in socket buffer\n");
					continue;
				} else if (errno == ECONNRESET) {
					fprintf(stderr, "Recv RST\n");
					continue;
				} else if (errno == EINTR) {
					fprintf(stderr,  " interrupt by signal\n");
					continue;
				} else if (errno < 0) {
					fprintf(stderr, "Other error: %s\n", strerror(errno));
					ev.data.fd = fd;
					epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, &ev);
					continue;
				}

				if (readed == 0) {
					fprintf(stderr, "Recv FIN\n");
					ev.data.fd = fd;
					epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, &ev);
					continue;
				} else if (readed < 1024) {
					on_read(fd, readed, buffer);
					ev.data.fd = fd;
					ev.events = EPOLLOUT;// | EPOLLET;
					epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &ev);
				}
			} else if (events[i].events & EPOLLOUT) {
				int fd = events[i].data.fd;
				on_write(fd);
				epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, &ev);
				close(fd);
			}
		}
	}

	return 0;
}

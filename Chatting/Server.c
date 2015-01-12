/*
 * Server.c
 *
 *  Created on: 2014. 11. 23.
 *      Author: superhakgoman
 */

#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <string.h>
#include <stdio.h>

#define PORT 8080
#define POLLSIZE 20
#define MAXLINE 1024

struct user_data
{
	int fd;
	char name[10];
};

int user_fds[1024];

void send_msg(struct epoll_event ev, char *msg)
{
	int i;
	char buf[MAXLINE + 24];
	struct user_data* user;

	user = ev.data.ptr;
	for(i = 0; i < 1024; i++)
	{
		memset(buf, 0x00, MAXLINE+24);
		sprintf(buf, "%s %s", user->name, msg);
		if(1 == (user_fds[i]))
		{
			write(i, buf, MAXLINE + 24);
		}
	}
}

int main(int argc, char** argv){
	struct sockaddr_in addr, clientaddr;
	struct epoll_event ev;
	struct epoll_event* events;
	struct user_data* user;
	int listenfd;
	int clientfd;
	socklen_t addrlen, clilen;
	int readn, eventn, epollfd;
	char buf[MAXLINE];
	int i;

	char* idtable = "Dog\0Cat\0Pet\0Man\0Boy\0Guy\0Cap\0Tim\0";

	printf("Server started. Port Number : %d.\n"
			"Turn on the client programs to start chatting.\n", PORT);

	events = (struct epoll_event*)malloc(sizeof(struct epoll_event) * POLLSIZE);
	if((epollfd = epoll_create(100)) == -1)
	{
		return 1;
	}

	addrlen = sizeof(addr);
	if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		return 1;
	}

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(PORT);
	if(bind(listenfd, (struct sockaddr*)&addr, addrlen) == -1)
	{
		return 1;
	}
	listen(listenfd, 5);
	ev.events = EPOLLIN;
	ev.data.fd = listenfd;
	epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &ev);
	memset(user_fds, -1, sizeof(int)*1024);

	while(1)
	{
		int n = getchar();
		if(n == 0){
			break;
		}
		eventn = epoll_wait(epollfd, events, POLLSIZE, -1);
		if(eventn == -1)
		{
			return 1;
		}
		for(i = 0; i < eventn; i++)
		{
			if(events[i].data.fd == listenfd)
			{
				clilen = sizeof(struct sockaddr);
				clientfd = accept(listenfd, (struct sockaddr*)&clientaddr, &clilen);
				user_fds[clientfd] = 1;
				user = malloc(sizeof(user));
				user->fd = clientfd;
				sprintf(user->name, "Anonymous %s > ", idtable + (clientfd%8*4));

				ev.events = EPOLLIN;
				ev.data.ptr = user;

				epoll_ctl(epollfd, EPOLL_CTL_ADD, clientfd, &ev);
			}
			else
			{
				user = events[i].data.ptr;
				memset(buf, 0x00, MAXLINE);
				readn = read(user->fd, buf, MAXLINE);
				if(readn <= 0)
				{
					epoll_ctl(epollfd, EPOLL_CTL_DEL, user->fd, events);
					close(user->fd);
					user_fds[user->fd] = -1;
					free(user);
				}
				else
				{
					send_msg(events[i], buf);
				}
			}
		}
	}
}


/* main.c
 *
 * Copyright (c) 2020 Ali AslRousta <aslrousta@gmail.com>
 * See LICENSE file.
 */

#include <ev.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static int client_read(int fd) {
  char buf[1024];
  ssize_t received;
  int i;

  received = recv(fd, buf, 1024, 0);
  if (received <= 0) {
    if (received < 0)
      perror("couldn't read from client");
    return -1;
  }

  printf("client sent: ");
  for (i = 0; i < received; ++i)
    printf("%02x ", buf[i]);
  printf("\n");

  return 0;
}

static void do_client(struct ev_loop *loop, ev_io *w, int events) {
  if (events & EV_READ) {
    if (client_read(w->fd) < 0) {
      ev_io_stop(loop, w);
      close(w->fd);

      printf("client closed\n");
    }
  }
}

static void do_accept(struct ev_loop *loop, ev_io *w, int events) {
  struct ev_io *w_client;
  struct sockaddr_in remote_addr;
  socklen_t addrlen = sizeof(remote_addr);
  int clientfd;

  clientfd = accept(w->fd, (struct sockaddr *)&remote_addr, &addrlen);
  if (clientfd < 0) {
    perror("couldn't accept the client");
    return;
  }

  printf("client connected: %s\n", inet_ntoa(remote_addr.sin_addr));

  w_client = (struct ev_io *)malloc(sizeof(struct ev_io));
  ev_io_init(w_client, do_client, clientfd, EV_READ | EV_WRITE);
  ev_io_start(loop, w_client);
}

int main() {
  struct ev_loop *loop = EV_DEFAULT;
  struct sockaddr_in addr;
  struct ev_io w_accept;
  int serverfd;

  serverfd = socket(PF_INET, SOCK_STREAM, 0);
  if (serverfd < 0) {
    perror("couldn't create server socket");
    return 1;
  }

  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = 0;
  addr.sin_port = htons(23);

  if (bind(serverfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    perror("couldn't bind to :23");
    close(serverfd);
    return 1;
  }

  if (listen(serverfd, 8) < 0) {
    perror("couldn't listen on :23");
    close(serverfd);
    return 1;
  }

  ev_io_init(&w_accept, do_accept, serverfd, EV_READ);
  ev_io_start(loop, &w_accept);

  ev_run(loop, 0);

  close(serverfd);
  return 0;
}

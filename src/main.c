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

#include "session.h"

#define BUFSIZE 1024

struct client {
  ev_io w;
  struct session session;
  struct in_addr addr;
  char rbuf[BUFSIZE];
  char wbuf[BUFSIZE];
  int rpos;
  int wpos;
};

static int client_read(struct client *c) {
  ssize_t received;

  received = recv(c->w.fd, c->rbuf + c->rpos, BUFSIZE - c->rpos, 0);
  if (received <= 0) {
    if (received < 0)
      perror("recv failed");
    return -1;
  }

  c->rpos += received;
  return 0;
}

static int client_write(struct client *c) {
  ssize_t sent;

  c->wpos += session_flush(&c->session, c->wbuf + c->wpos, BUFSIZE - c->wpos);
  if (!c->wpos)
    return 0;

  sent = send(c->w.fd, c->wbuf, c->wpos, 0);
  if (sent < 0) {
    perror("send failed");
    return -1;
  }

  if (sent < c->wpos)
    memmove(c->wbuf, c->wbuf + sent, c->wpos - sent);
  c->wpos -= sent;
  return 0;
}

static void do_client(struct ev_loop *loop, ev_io *w, int events) {
  struct client *client = (struct client *)w;
  int proced;

  if (events & EV_READ) {
    if (client_read(client) < 0)
      goto close;

    proced = session_proc(&client->session, client->rbuf, client->rpos);
    if (proced < 0)
      goto close;

    if (proced < client->rpos)
      memmove(client->rbuf, client->rbuf + proced, client->rpos - proced);
    client->rpos -= proced;
  }

  if ((events & EV_WRITE) && client_write(client) < 0)
    goto close;

  return;

close:
  ev_io_stop(loop, &client->w);
  close(client->w.fd);
  free(client);

  printf("closed: %s\n", inet_ntoa(client->addr));
}

static void do_accept(struct ev_loop *loop, ev_io *w, int events) {
  struct client *client;
  struct sockaddr_in remote_addr;
  socklen_t addrlen = sizeof(remote_addr);
  int clientfd;

  clientfd = accept(w->fd, (struct sockaddr *)&remote_addr, &addrlen);
  if (clientfd < 0) {
    perror("accept failed");
    return;
  }

  client = (struct client *)malloc(sizeof(struct client));
  client->addr = remote_addr.sin_addr;
  client->rpos = 0;
  client->wpos = 0;

  session_init(&client->session);
  ev_io_init(&client->w, do_client, clientfd, EV_READ | EV_WRITE);
  ev_io_start(loop, &client->w);

  printf("connected: %s\n", inet_ntoa(remote_addr.sin_addr));
}

static int run() {
  struct ev_loop *loop = EV_DEFAULT;
  struct sockaddr_in addr;
  struct ev_io w_accept;
  int fd;

  fd = socket(PF_INET, SOCK_STREAM, 0);
  if (fd < 0) {
    perror("socket failed");
    return 1;
  }

  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = 0;
  addr.sin_port = htons(23);

  if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    perror("bind failed");
    close(fd);
    return 1;
  }

  if (listen(fd, 8) < 0) {
    perror("listen failed");
    close(fd);
    return 1;
  }

  ev_io_init(&w_accept, do_accept, fd, EV_READ);
  ev_io_start(loop, &w_accept);
  ev_run(loop, 0);

  close(fd);
  return 0;
}

int main() { return run(); }

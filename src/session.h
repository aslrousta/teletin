/* session.h
 *
 * Copyright (c) 2020 Ali AslRousta <aslrousta@gmail.com>
 * See LICENSE file.
 */

#ifndef TELETIN_SESSION__INCLUDED
#define TELETIN_SESSION__INCLUDED

struct session {
  char name[100];
  char email[200];
};

void session_init(struct session *s);
int session_proc(struct session *s, const char *buf, int len);
int session_flush(struct session *s, const char *buf, int len);

#endif /* TELETIN_SESSION__INCLUDED */

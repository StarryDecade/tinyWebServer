#pragma once

#include "log.h"
#include <arpa/inet.h>

class util_timer;
struct client_data {
	sockaddr_in address;
	int sockfd;
	util_timer *timer;
};

class util_timer {
public:
	util_timer() : prev(NULL), next(NULL) {}

public:
	time_t expire;
	void (*cb_func)(client_data *);
	client_data *user_data;
	util_timer *prev;
	util_timer *next;
};

class sort_timer_lst {
public:
	sort_timer_lst() : head(NULL), tail(NULL){}
	~sort_timer_lst();
	void add_timer(util_timer *timer);
	void adjust_timer(util_timer *timer);
	void del_timer(util_timer *timer);
	void tick();

private:
	void add_timer(util_timer *timer, util_timer *lst_head);

private:
	util_timer *head;
	util_timer *tail;
};

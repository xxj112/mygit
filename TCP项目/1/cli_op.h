#ifndef _CLI_OP_H_
#define _CLI_OP_H_
#include "share.h"
void pr_msg(msg now);
void send_msg(msg now,int fd);
void hand_msg(int fd,msg now);
void pr_time(void);

#endif
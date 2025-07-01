#ifndef _SER_OP_H_
#define _SER__H_
#include "share.h"
#include "mysql.h"
void pr_msg(msg now);
void send_msg(msg now,int fd);
void hand_msg(int fd,msg now);
void register_user(int fd,char * name, char * password);
void login_user(int fd, char * account, char * password);
void delete_user(int fd,char * account);
void send_one_user(int fd,msg now);
void lgout_user(int fd, char * account);
void  send_all_user(int fd, msg now);
#endif
#ifndef _SER_OP_H_
#define _SER__H_
#include "share.h"
#include "mysql.h"
void pr_msg(msg now);
void send_msg(msg now,int fd);

void file_w(msg now);

void register_user(int fd, const char *password, const char *name);
void login_user(int fd, const char *account, const char *password);
void logout_user(int fd, const char *account);
void delete_user(int fd, const char *account);

void send_one_user(int fd, const char *from, const char *to, const char *msg);
void send_all_user(int fd, const char *from, const char *msg);

void add_friend(int fd, const char *a, const char *b);
void delete_friend(int fd, const char *a, const char *b);

void create_qun(int fd, const char *account, const char *qun_name);
void join_qun(int fd, const char *account, int qun_id);
void quit_qun(int fd, const char *account, int qun_id);
void dismiss_qun(int fd, const char *account, int qun_id);

void query_friend_list(int fd, const char *account);
void query_qun_list(int fd, const char *account);


#endif  
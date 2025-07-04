#ifndef _SER_OP_H_
#define _SER__H_
#include "share.h"
#include "mysql.h"
void pr_msg(msg now);
void send_msg(msg now,int fd);

void log_request(msg now);
void log_response(msg now); 

void register_user(int fd, const char *password, const char *name);
void login_user(int fd, const char *account, const char *password);
void logout_user(int fd, const char *account);
void delete_user(int fd, const char *account);

void send_one_user(int fd, const char *from, const char *name, const char *to, const char *data);

void add_friend(int fd, const char *a, const char *b);
void delete_friend(int fd, const char *a, const char *b);

void create_qun(int fd, const char *account, const char *qun_name);
void join_qun(int fd, const char *account, int qun_id);
void quit_qun(int fd, const char *account, int qun_id);
void dismiss_qun(int fd, const char *account, int qun_id);

void query_friend_list(int fd, const char *account);
void query_qun_list(int fd, const char *account);

void send_qun_user(int fd, const char *from, const char *name, const char *data, int qun_id);
void delete_qun_user(int fd, const char *a, const char *b, int qun_id);
void set_qun_permission(int fd, const char *a, const char *b, int permission, int qun_id);
void jin_qun_user(int fd, const char *a, const char *b, int qun_id);
void jie_qun_user(int fd, const char *a, const char *b, int qun_id);

#endif  
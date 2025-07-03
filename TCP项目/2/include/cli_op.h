#ifndef _CLI_OP_H_
#define _CLI_OP_H_
#include "share.h"
void pr_msg(msg now);
void send_msg(msg now);
void pr_time(void);
// void hand_msg(int fd,msg now);
void msg_handle(msg rec_msg);
void one_menu(void); //一级菜单
void two_menu(void); //二级菜单

// rec 都是处理接受信息的函数
void Register(void);
void rec_register(const char *account, const char *data);
void Login(void);
void rec_login(const char *name, const char *data);
void Lgout(void);
void rec_lgout(const char *data);
void Delete_account(void);
void rec_delete_account(const char *data);
void Send_single(void);
void rec_send_single(const char *account, const char *name, const char *data);
void Add_user(void);
void rec_add_user(const char *data);
void Delete_user(void);
void rec_delete_user(const char *data);
void Find_user_list(void);
void rec_find_user_list(const char *account, const char *name, const char *data);

void Create_qun(void);
void rec_create_qun(const char *data);
void Delete_qun(void);
void rec_delete_qun(const char *data);   
void Find_qun_list(void);
void rec_find_qun_list(const char *account, const char *name, const char *data);

void Add_qun(void);
void rec_add_qun(const char *data);
void Exit_qun(void);
void rec_exit_qun(const char *data);

void Send_qun(void);
void rec_send_qun(const char *account, const char *name, const char *data, const char *qun);

// void threee_menu(void); //群聊界面


#endif
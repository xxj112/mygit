#ifndef  _MYSQL_H_
#define _MYSQL_H_
/*
初始化 连接 关闭
增加 用户信息 账号 密码 昵称 
删除用户信息 
更改用户信息 fd
查询用户信息 返回用户 账号 密码 昵称 fd 
 */
#include "share.h"
#include <mysql/mysql.h>
extern MYSQL *mydb ;
extern char accounts[100][16]; 
void mysql_init_xxj(void);
void mysql_close_xxj(void);

void mysql_insert_user(sql_user user); // 添加用户信息
void mysql_delete_user(const char *account); //根据账号删除
void mysql_update_user_status(const char *account, int new_fd); //根据账号 修改fd
sql_user mysql_query_users(const char *account); // 根据账号 用户信息
void mysql_update_user_qun(const char *account, int new_qun); //根据账号 修改qun
int get_accounts_qun(void);//返回群账号的个数
#endif // ! 
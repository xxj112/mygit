#ifndef  _MYSQL_H_
#define _MYSQL_H_
/*
// 0 表示成功 -1表示操作失败
*/
#include "share.h"
#include <mysql/mysql.h>

typedef struct {
    char account[17];   // 用户账号（16字节 + 1结尾符）
    char password[17];  // 密码
    char name[17];      // 昵称
    int fd;             // 在线状态（0=离线，1=在线等）
} sql_user;

typedef struct {
    int qun_id;         // 群ID，自增
    char qun_name[17];  // 群名称
    char qunzhu[17];    // 群主账号
} sql_qun;

typedef struct {
    int qun_id;             // 群ID
    char member[17];        // 群成员账号
    int permission;         // 权限（0=成员，1=管理员，2=群主）
    int mute;               // 禁言状态 0=正常 1=禁言
} sql_qun_member;

typedef struct {
    char a[17];   // 用户A账号
    char b[17];   // 用户B账号
} sql_friend;

extern MYSQL *mydb;
//连接和关闭
int sql_init(void);
void sql_close(void);

// 用户
int sql_insert_user(const char *account, const char *password, const char *name);
int sql_update_user_status(const char *account, int fd);
int sql_delete_user(const char *account);
sql_user sql_get_user_by_account(const char *account);
// 好友
int sql_add_friend(const char *a, const char *b);
int sql_delete_friend(const char *a, const char *b);
int sql_is_friend_exist(const char *a, const char *b);
int sql_query_friend_list(int fd, const char *account);
int sql_is_friend(const char *a, const char *b);
// 群
int sql_create_qun(const char *qun_name, const char *qunzhu);
int sql_join_qun(int qun_id, const char *account);
int sql_dismiss_qun(int qun_id, const char *qunzhu);
int sql_quit_qun(int qun_id, const char *account);
int sql_query_qun_list(int fd, const char *account);
int sql_query_qun_members(int qun_id);
int sql_query_created_quns(const char *account); 
int sql_get_mute_status(int qun_id, const char *account);
int sql_set_mute(int qun_id, const char *account, int mute_flag);
int sql_is_qun_user(const char *account, int qun_id);
void sql_send_qun_members(int qun_id, const char *account, const char *name, const char *data);
#endif 


#include "mysql.h"
MYSQL *mydb = NULL;
void mysql_init_xxj(void)
{
    mydb = mysql_init(NULL);
    if (!mysql_real_connect(mydb, "localhost", "root", "1", "TCP_DB", 0, NULL, 0)) 
    {
        fprintf(stderr, "数据库连接失败: %s\n", mysql_error(mydb));
        return ;
    }
   // printf("%s");
    printf("数据库连接成功\n");
}
void mysql_close_xxj(void)
{
    mysql_close(mydb);
}
void mysql_insert_user(sql_user user) // 添加用户信息
{
    char buf[128] = {0};
    sprintf(buf, "insert into user value('%s','%s','%s',%d,%d);",\
    user.account, user.password, user.selfname, user.fd, user.qun);
    printf("%s\n",buf);
    int ret = mysql_query(mydb,buf);
    if(ret != 0)
    {
        printf("添加用户信息失败\n");
        mysql_close_xxj();
        return ;
    }
    printf("添加用户信息成功\n");
}
void mysql_update_user_status(const char *account, int new_fd) //根据账号 修改fd
{
    char buf[128] = {0};
    snprintf(buf, sizeof(buf), "UPDATE user SET fd=%d WHERE 账号='%s';", new_fd, account);
    int ret = mysql_query(mydb, buf);
    if (ret != 0) {
        fprintf(stderr, "更新用户状态失败: %s\n", mysql_error(mydb));
        return;
    }
    printf("账号'%s'的用户fd字段更新为%d成功\n", account, (int)new_fd);
}
void mysql_delete_user(const char *account) //根据账号删除
{
    char buf[64] = {0};
    snprintf(buf, sizeof(buf), "delete from user where 账号='%s';", account);
    int ret = mysql_query(mydb, buf);
    if (ret != 0) 
    {
        fprintf(stderr, "删除用户失败: %s\n", mysql_error(mydb));
        return;
    }
    printf("账号为'%s'的用户删除成功\n", account);
}
sql_user mysql_query_users(const char *account) // 根据账号 用户信息
{
    MYSQL_RES *res = NULL;
    MYSQL_ROW row;
    //sql_user now = {0}; //要返回的信息
    char query[256] = {0};
    sprintf(query, "SELECT * FROM user WHERE 账号 = '%s';", account);

    // 执行查询
    if(mysql_query(mydb, query) != 0)
    {
        printf("查询用户失败: %s\n", mysql_error(mydb));
        sql_user empty = {0};
        return empty;
    }

    res = mysql_store_result(mydb);
    if(res == NULL)
    {
        printf("获取查询结果失败: %s\n", mysql_error(mydb));
        sql_user empty = {0};
        return empty;
    }

    // 获取一行数据
    if ((row = mysql_fetch_row(res)) != NULL)
    {
        sql_user user;
        strcpy(user.account,   row[0] ? row[0] : "");
        strcpy(user.password,  row[1] ? row[1] : "");
        strcpy(user.selfname,  row[2] ? row[2] : "");
        user.fd  = atoi(row[3]);
        user.qun = atoi(row[4]);

        mysql_free_result(res);
        return user;
    }
    else
    {
        printf("未找到该账号：%s\n", account);
        sql_user empty = {0};
        mysql_free_result(res);
        return empty;
    }

  //  return bow;
}
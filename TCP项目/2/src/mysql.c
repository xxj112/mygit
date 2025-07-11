#include "mysql.h"
MYSQL *mydb = NULL;
// 0 表示成功 -1表示操作失败
int sql_init(void)
{
    mydb = mysql_init(NULL);
    if (!mysql_real_connect(mydb, "localhost", "root", "1", "chatdb", 0, NULL, 0)) 
    {
        fprintf(stderr, "数据库连接失败: %s\n", mysql_error(mydb));
        return -1;
    }
   // printf("%s");
    printf("数据库连接成功\n");
    return 0;
}       
void sql_close(void)
{
    mysql_close(mydb);
}
//添加新用户
int sql_insert_user(const char *account, const char *password, const char *name) {
    char sql[256];
    sprintf(sql, "INSERT INTO users(account, password, name, fd) VALUES('%s', '%s', '%s', 0)", account, password, name);
    int ret = mysql_query(mydb, sql);
    if (ret != 0) {
        fprintf(stderr, "插入用户失败: %s\n", mysql_error(mydb));
        return -1;
    }
    return 0;
}
//更新用户套接字，套接字为 0 表示不在线， 1 在线
int sql_update_user_status(const char *account, int fd) {
    char sql[128];
    sprintf(sql, "UPDATE users SET fd = %d WHERE account = '%s'", fd, account);
    int ret = mysql_query(mydb, sql);
    if (ret != 0) {
        fprintf(stderr, "更新用户状态失败: %s\n", mysql_error(mydb));
        return -1;
    }
    return 0;
}
/*删除用户 
先判断是否是群主
先把他创建的群解散
再删除好友列表
最后再删除用户
*/
int sql_delete_user(const char *account) {
    char sql[256];
    MYSQL_RES *res;
    MYSQL_ROW row;

    // 1. 查询该用户创建的群
    sprintf(sql, "SELECT qun_id FROM qun WHERE qunzhu = '%s'", account);
    if (mysql_query(mydb, sql) != 0) {
        fprintf(stderr, "查询用户创建的群失败: %s\n", mysql_error(mydb));
        return -1;
    }

    res = mysql_store_result(mydb);
    if (!res) {
        fprintf(stderr, "获取结果失败: %s\n", mysql_error(mydb));
        return -1;
    }

    // 2. 遍历每个群，解散它
    while ((row = mysql_fetch_row(res))) {
        int qun_id = atoi(row[0]);

        // 删除该群的群成员记录
        sprintf(sql, "DELETE FROM qun_list WHERE qun_id = %d", qun_id);
        if (mysql_query(mydb, sql) != 0) {
            fprintf(stderr, "删除群成员失败 (群ID=%d): %s\n", qun_id, mysql_error(mydb));
            mysql_free_result(res);
            return -1;
        }

        // 删除群本身
        sprintf(sql, "DELETE FROM qun WHERE qun_id = %d", qun_id);
        if (mysql_query(mydb, sql) != 0) {
            fprintf(stderr, "删除群失败 (群ID=%d): %s\n", qun_id, mysql_error(mydb));
            mysql_free_result(res);
            return -1;
        }
    }
    mysql_free_result(res);

    // 3. 删除该用户相关的好友关系
    sprintf(sql, "DELETE FROM user_friend WHERE a='%s' OR b='%s'", account, account);
    if (mysql_query(mydb, sql) != 0) {
        fprintf(stderr, "删除好友关系失败: %s\n", mysql_error(mydb));
        return -1;
    }

    // 4. 删除该用户参与的群成员信息
    sprintf(sql, "DELETE FROM qun_list WHERE member='%s'", account);
    if (mysql_query(mydb, sql) != 0) {
        fprintf(stderr, "删除群参与信息失败: %s\n", mysql_error(mydb));
        return -1;
    }

    // 5. 删除该用户账户本身
    sprintf(sql, "DELETE FROM users WHERE account='%s'", account);
    if (mysql_query(mydb, sql) != 0) {
        fprintf(stderr, "删除用户失败: %s\n", mysql_error(mydb));
        return -1;
    }

    return 0;
}
// 判断是否为好友关系（即是否存在 a -> b 这一条记录）
// 如果是好友，返回 0；否则返回 1；如果查询失败返回 -1
int sql_is_friend(const char *a, const char *b) {
    char sql[256];
    MYSQL_RES *res;
    MYSQL_ROW row;

    // 检查 a -> b 是否存在
    sprintf(sql, "SELECT * FROM user_friend WHERE a='%s' AND b='%s'", a, b);
    if (mysql_query(mydb, sql) != 0) {
        fprintf(stderr, "查询好友关系失败 (%s -> %s): %s\n", a, b, mysql_error(mydb));
        return -1;
    }

    res = mysql_store_result(mydb);
    if (res == NULL) {
        fprintf(stderr, "获取查询结果失败: %s\n", mysql_error(mydb));
        return -1;
    }

    int is_friend = (mysql_num_rows(res) > 0) ? 0 : 1;

    mysql_free_result(res);
    return is_friend;
}

//添加好友关系
int sql_add_friend(const char *a, const char *b) {
    char sql[256];

    // 插入 a -> b
    sprintf(sql, "INSERT INTO user_friend VALUES('%s', '%s')", a, b);
    if (mysql_query(mydb, sql) != 0) {
        fprintf(stderr, "添加好友失败 (%s -> %s): %s\n", a, b, mysql_error(mydb));
        return -1;
    }

    // 插入 b -> a
    sprintf(sql, "INSERT INTO user_friend VALUES('%s', '%s')", b, a);
    if (mysql_query(mydb, sql) != 0) {
        fprintf(stderr, "添加好友失败 (%s -> %s): %s\n", b, a, mysql_error(mydb));

        // 回滚前面插入的 a -> b（可选）
        char rollback_sql[256];
        sprintf(rollback_sql, "DELETE FROM user_friend WHERE a='%s' AND b='%s'", a, b);
        mysql_query(mydb, rollback_sql);

        return -1;
    }

    return 0;
}
//返回用户信息
sql_user sql_get_user_by_account(const char *account)
{
    sql_user user = {0};
    memset(&user, 0, sizeof(user));  // 初始化为 0，默认 account 是空字符串

    if (!account || strlen(account) == 0)
        return user;

    char sql[128];
    snprintf(sql, sizeof(sql),
             "SELECT account, password, name, fd FROM users WHERE account = '%s'", account);

    if (mysql_query(mydb, sql) != 0) {
        fprintf(stderr, "查询用户失败: %s\n", mysql_error(mydb));
        return user;
    }

    MYSQL_RES *res = mysql_store_result(mydb);
    if (!res) {
        fprintf(stderr, "获取查询结果失败: %s\n", mysql_error(mydb));
        return user;
    }

    MYSQL_ROW row = mysql_fetch_row(res);
    if (row) {
        strncpy(user.account, row[0], 16);
        strncpy(user.password, row[1], 16);
        strncpy(user.name, row[2], 16);
        user.fd = row[3] ? atoi(row[3]) : 0;
    } else {
        // 用户不存在
        fprintf(stderr, "用户 %s 不存在\n", account);
    }

    mysql_free_result(res);
    return user;
}

//删除好友关系
int sql_delete_friend(const char *a, const char *b) {
    char sql[256];
    sprintf(sql,
        "DELETE FROM user_friend WHERE (a='%s' AND b='%s') OR (a='%s' AND b='%s')",
        a, b, b, a);
    int ret = mysql_query(mydb, sql);
    if (ret != 0) {
        fprintf(stderr, "删除好友失败: %s\n", mysql_error(mydb));
        return -1;
    }
    return 0;
}
//判断是否是好友
int sql_is_friend_exist(const char *a, const char *b) {
    char sql[256];
    sprintf(sql, "SELECT * FROM user_friend WHERE a='%s' AND b='%s'", a, b);
    if (mysql_query(mydb, sql) != 0) {
        fprintf(stderr, "查询好友关系失败: %s\n", mysql_error(mydb));
        return -1;
    }

    MYSQL_RES *res = mysql_store_result(mydb);
    if (!res) {
        fprintf(stderr, "获取好友查询结果失败: %s\n", mysql_error(mydb));
        return -1;
    }

    int exists = (mysql_num_rows(res) > 0); // 如果结果不为空，说明存在
    mysql_free_result(res);
    return exists;
}
//查询好友列表
int sql_query_friend_list(int fd, const char *account) {
    char sql[256];
    // 注意把 user 改成正确的表名 users
    sprintf(sql,
        "SELECT users.account, users.name "
        "FROM user_friend JOIN users ON user_friend.b = users.account "
        "WHERE user_friend.a='%s'", account);

    if (mysql_query(mydb, sql) != 0) {
        fprintf(stderr, "查询好友列表失败: %s\n", mysql_error(mydb));
        return -1;
    }

    MYSQL_RES *res = mysql_store_result(mydb);
    if (!res) {
        fprintf(stderr, "获取好友列表失败: %s\n", mysql_error(mydb));
        return -1;
    }

    MYSQL_ROW row;
    msg friends = {0};
    friends.msgtype = MSG_FIU;

    printf("【%s 的好友列表】:\n", account);
    int w = 0;
    while ((row = mysql_fetch_row(res))) {
        printf("好友账号: %s，昵称: %s\n", row[0], row[1]);

        // 填充结构体
        strcpy(friends.account, row[0]);
        strcpy(friends.selfname, row[1]);

        // 用 msgdata 传递好友编号（可选）
        snprintf(friends.msgdata, sizeof(friends.msgdata), "%d", ++w);

        // 发送给客户端
        send_msg(friends, fd);
    }

    mysql_free_result(res);
    return 0;
}

// 创建群
int sql_create_qun(const char *qun_name, const char *qunzhu) {
    char sql[256];

    // 1. 插入群表
    sprintf(sql, "INSERT INTO qun(qun_name, qunzhu) VALUES('%s', '%s')", qun_name, qunzhu);
    if (mysql_query(mydb, sql) != 0) {
        fprintf(stderr, "创建群失败: %s\n", mysql_error(mydb));
        return -1;
    }

    // 2. 获取刚刚插入的群ID
    int qun_id = mysql_insert_id(mydb);

    // 3. 把群主也加入 qun_list，权限为2
    sprintf(sql, "INSERT INTO qun_list(qun_id, member, permission) VALUES(%d, '%s', 2)", qun_id, qunzhu);
    if (mysql_query(mydb, sql) != 0) {
        fprintf(stderr, "插入群成员失败: %s\n", mysql_error(mydb));

        // 删除刚插入的群
        sprintf(sql, "DELETE FROM qun WHERE id = %d", qun_id);
        if (mysql_query(mydb, sql) != 0) {
            fprintf(stderr, "回滚群失败: %s\n", mysql_error(mydb));
        }

        return -1;
    }

    return 0;
}
// 加入群
int sql_join_qun(int qun_id, const char *account) {
    char sql[256];

    // 插入为普通成员，permission = 0
    sprintf(sql, "INSERT INTO qun_list(qun_id, member, permission) VALUES(%d, '%s', 0)", qun_id, account);
    if (mysql_query(mydb, sql) != 0) {
        fprintf(stderr, "加入群失败: %s\n", mysql_error(mydb));
        return -1;
    }

    return 0;
}
//退出群
int sql_quit_qun(int qun_id, const char *account) {
    char sql[256];

    // 查询权限，判断是不是群主
    sprintf(sql, "SELECT permission FROM qun_list WHERE qun_id = %d AND member = '%s'", qun_id, account);
    if (mysql_query(mydb, sql) != 0) {
        fprintf(stderr, "查询成员权限失败: %s\n", mysql_error(mydb));
        return -1;
    }

    MYSQL_RES *res = mysql_store_result(mydb);
    if (!res) {
        fprintf(stderr, "获取权限结果失败: %s\n", mysql_error(mydb));
        return -1;
    }

    MYSQL_ROW row = mysql_fetch_row(res);
    if (!row) {
        fprintf(stderr, "用户不在群中，无法退出\n");
        mysql_free_result(res);
        return -1;
    }

    int perm = atoi(row[0]);
    mysql_free_result(res);

    // 如果是群主则解散群
    if (perm == 2) {
        return sql_dismiss_qun(qun_id, account);
    }

    // 否则正常删除该成员
    sprintf(sql, "DELETE FROM qun_list WHERE qun_id = %d AND member = '%s'", qun_id, account);
    if (mysql_query(mydb, sql) != 0) {
        fprintf(stderr, "退出群失败: %s\n", mysql_error(mydb));
        return -1;
    }

    return 0;
}
// 注销聊天群
int sql_dismiss_qun(int qun_id, const char *qunzhu) {
    char sql[256];

    // 确认是否是群主
    sprintf(sql, "SELECT qunzhu FROM qun WHERE qun_id = %d", qun_id);
    if (mysql_query(mydb, sql) != 0) {
        fprintf(stderr, "查询群主失败: %s\n", mysql_error(mydb));
        return -1;
    }

    MYSQL_RES *res = mysql_store_result(mydb);
    if (!res) {
        fprintf(stderr, "获取群主信息失败: %s\n", mysql_error(mydb));
        return -1;
    }

    MYSQL_ROW row = mysql_fetch_row(res);
    if (!row || strcmp(row[0], qunzhu) != 0) {
        fprintf(stderr, "不是群主，无法解散群\n");
        mysql_free_result(res);
        return -1;
    }
    mysql_free_result(res);

    // 删除所有成员
    sprintf(sql, "DELETE FROM qun_list WHERE qun_id = %d", qun_id);
    if (mysql_query(mydb, sql) != 0) {
        fprintf(stderr, "删除群成员失败: %s\n", mysql_error(mydb));
        return -1;
    }

    // 删除群本身
    sprintf(sql, "DELETE FROM qun WHERE qun_id = %d", qun_id);
    if (mysql_query(mydb, sql) != 0) {
        fprintf(stderr, "删除群失败: %s\n", mysql_error(mydb));
        return -1;
    }

    return 0;
}

// 查询加入的所有群
int sql_query_qun_list(int fd, const char *account) {
    char sql[256];
    sprintf(sql,
        "SELECT q.qun_id, q.qun_name, l.permission, l.mute "
        "FROM qun_list l "
        "JOIN qun q ON l.qun_id = q.qun_id "
        "WHERE l.member = '%s'", account);

    if (mysql_query(mydb, sql) != 0) {
        fprintf(stderr, "查询群列表失败: %s\n", mysql_error(mydb));
        return -1;
    }

    MYSQL_RES *res = mysql_store_result(mydb);
    if (!res) {
        fprintf(stderr, "获取群列表失败: %s\n", mysql_error(mydb));
        return -1;
    }

    MYSQL_ROW row;
    msg qun_msg = {0};
    qun_msg.msgtype = MSG_FIQ;  // 群列表消息类型（你自己定义）

    int index = 1;
    while ((row = mysql_fetch_row(res))) {
        const char *qun_id     = row[0];
        const char *qun_name   = row[1];
        const char *permission = row[2];
        const char *mute       = row[3];

        printf("%d. 群ID: %s, 群名: %s, 权限: %s\n",
               index, qun_id, qun_name, permission);

        // 群ID和群名
        strncpy(qun_msg.account, qun_id, sizeof(qun_msg.account) - 1);
        strncpy(qun_msg.selfname, qun_name, sizeof(qun_msg.selfname) - 1);

        // msgdata: 权限#禁言#编号，例如："2#0#1"
        snprintf(qun_msg.msgdata, sizeof(qun_msg.msgdata), "权限%s状态%s--第%d个群", permission, mute, index);

        send_msg(qun_msg, fd);
        index++;
    }

    mysql_free_result(res);
    return 0;
}



//查询群成员
int sql_query_qun_members(int qun_id) {
    char sql[256];
    sprintf(sql, 
        "SELECT u.account, u.name, l.permission FROM qun_list l "
        "JOIN users u ON l.member = u.account "
        "WHERE l.qun_id = %d", qun_id);

    if (mysql_query(mydb, sql) != 0) {
        fprintf(stderr, "查询群成员失败: %s\n", mysql_error(mydb));
        return -1;
    }

    MYSQL_RES *res = mysql_store_result(mydb);
    if (!res) {
        fprintf(stderr, "获取群成员失败: %s\n", mysql_error(mydb));
        return -1;
    }

    MYSQL_ROW row;
    printf("【群 %d 成员列表】:\n", qun_id);
    while ((row = mysql_fetch_row(res))) {
        printf("账号: %s, 昵称: %s, 权限: %s\n", row[0], row[1], row[2]);
    }

    mysql_free_result(res);
    return 0;
}
//查询创造的群
int sql_query_created_quns(const char *account) {
    char sql[256];
    sprintf(sql, "SELECT qun_id, qun_name FROM qun WHERE qunzhu = '%s'", account);

    if (mysql_query(mydb, sql) != 0) {
        fprintf(stderr, "查询创建的群失败: %s\n", mysql_error(mydb));
        return -1;
    }

    MYSQL_RES *res = mysql_store_result(mydb);
    if (!res) {
        fprintf(stderr, "获取创建的群失败: %s\n", mysql_error(mydb));
        return -1;
    }

    MYSQL_ROW row;
    printf("【%s 创建的群】:\n", account);
    while ((row = mysql_fetch_row(res))) {
        printf("群ID: %s, 群名: %s\n", row[0], row[1]);
    }

    mysql_free_result(res);
    return 0;
}
//查询权限
int sql_query_permission_quns(const char *account, int qun_id)
{

    // 构造 SQL 查询语句
    char sql[256];
    snprintf(sql, sizeof(sql),
             "SELECT permission FROM qun_list WHERE qun_id=%d AND member='%s'",
             qun_id, account);

    // 执行 SQL 查询
    if (mysql_query(mydb, sql) != 0) {
        fprintf(stderr, "MySQL 查询出错: %s\n", mysql_error(mydb));
        return -1;
    }

    // 获取查询结果
    MYSQL_RES *res = mysql_store_result(mydb);
    if (!res) {
        fprintf(stderr, "MySQL 结果存储出错: %s\n", mysql_error(mydb));
        return -1;
    }

    // 获取一行结果（应该只有一行）
    MYSQL_ROW row = mysql_fetch_row(res);
    int permission = -1;

    if (row && row[0]) {
        permission = atoi(row[0]);  // 将结果转换为整数
    }

    mysql_free_result(res);  // 释放结果集资源
    return permission;       // 返回权限值或-1
}
//设置权限
int sql_set_permission_quns(const char *account,int permission,  int qun_id)
{
    // 构造 SQL 更新语句
    char sql[256];
    snprintf(sql, sizeof(sql),
             "UPDATE qun_list SET permission=%d WHERE qun_id=%d AND member='%s'",
             permission, qun_id, account);

    // 执行 SQL
    if (mysql_query(mydb, sql) != 0) {
        fprintf(stderr, "MySQL 修改权限出错: %s\n", mysql_error(mydb));
        return -1;
    }

    // 检查是否有行被修改（如无则说明找不到该成员）
    if (mysql_affected_rows(mydb) == 0) {
        return -1;  // 没有更新任何行
    }

    return 0;  // 成功
}
//设置禁言状态，只是对应群的禁言状态
int sql_set_mute(int qun_id, const char *account, int mute_flag) {
    char sql[256];
    sprintf(sql, "UPDATE qun_list SET mute = %d WHERE qun_id = %d AND member = '%s'", mute_flag, qun_id, account);

    if (mysql_query(mydb, sql) != 0) {
        fprintf(stderr, "设置禁言状态失败: %s\n", mysql_error(mydb));
        return -1;
    }
    return 0;
}
//查询对应群的禁言状态
int sql_get_mute_status(int qun_id, const char *account) {
    char sql[256];
    sprintf(sql, "SELECT mute FROM qun_list WHERE qun_id = %d AND member = '%s'", qun_id, account);

    if (mysql_query(mydb, sql) != 0) {
        fprintf(stderr, "查询禁言状态失败: %s\n", mysql_error(mydb));
        return -1;
    }

    MYSQL_RES *res = mysql_store_result(mydb);
    if (!res) {
        fprintf(stderr, "获取禁言状态结果失败: %s\n", mysql_error(mydb));
        return -1;
    }

    MYSQL_ROW row = mysql_fetch_row(res);
    if (!row) {
        fprintf(stderr, "该成员不在群中\n");
        mysql_free_result(res);
        return -1;
    }

    int status = atoi(row[0]);
    mysql_free_result(res);
    return status;
}
//是否是群成员
int sql_is_qun_user(const char *account, int qun_id)
{
    char sql[256];
    sprintf(sql, "SELECT 1 FROM qun_list WHERE qun_id = %d AND member = '%s' LIMIT 1", qun_id, account);

    if (mysql_query(mydb, sql) != 0) {
        fprintf(stderr, "判断群成员失败: %s\n", mysql_error(mydb));
        return -1;  // 数据库错误
    }

    MYSQL_RES *res = mysql_store_result(mydb);
    if (!res) {
        fprintf(stderr, "获取查询结果失败: %s\n", mysql_error(mydb));
        return -1;
    }

    int is_member = (mysql_num_rows(res) > 0) ? 0 : 1;
    mysql_free_result(res);

    return is_member;
}
//群发 
void sql_send_qun_members(int qun_id, const char *account, const char *name, const char *data)
{
    char sql[256];
    char qun_name[16] = {0}; 

    // Step 1: 查询群名
    sprintf(sql, "SELECT qun_name FROM qun WHERE qun_id = %d", qun_id);
    if (mysql_query(mydb, sql) != 0) {
        fprintf(stderr, "查询群名失败: %s\n", mysql_error(mydb));
        return;
    }

    MYSQL_RES *res_qun = mysql_store_result(mydb);
    if (!res_qun || mysql_num_rows(res_qun) == 0) {
        fprintf(stderr, "群ID %d 不存在。\n", qun_id);
        mysql_free_result(res_qun);
        return;
    }

    MYSQL_ROW row_qun = mysql_fetch_row(res_qun);
    strncpy(qun_name, row_qun[0], sizeof(qun_name) - 1);
    mysql_free_result(res_qun);

    // Step 2: 查询群成员
    sprintf(sql, "SELECT member FROM qun_list WHERE qun_id = %d", qun_id);
    if (mysql_query(mydb, sql) != 0) {
        fprintf(stderr, "查询群成员失败: %s\n", mysql_error(mydb));
        return;
    }

    MYSQL_RES *res = mysql_store_result(mydb);
    if (!res) {
        fprintf(stderr, "获取群成员失败: %s\n", mysql_error(mydb));
        return;
    }

    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res))) {
        const char *target_account = row[0];

        // 获取用户是否在线
        sql_user u = sql_get_user_by_account(target_account);
        if (u.fd == 0) continue;

        // 构造消息
        msg m = {0};
        m.msgtype = MSG_ALL; // 群聊类型
        strncpy(m.account, account, sizeof(m.account) - 1);      // 发送者账号
        strncpy(m.selfname, name, sizeof(m.selfname) - 1);       // 发送者昵称
        strncpy(m.msgdata, data, sizeof(m.msgdata) - 1);         // 消息内容
        snprintf(m.other, sizeof(m.other), "%d%s", qun_id, qun_name); //可能越界

        // 发送
        send_msg(m, u.fd);
        printf("已群发消息给：%s（fd=%d）\n", u.account, u.fd);
    }

    mysql_free_result(res);
}
#include "ser_op.h"
#include <time.h>
//封装函数原则，尽量不要用全局变量， 只用参数就能完成功能， 服务端是这样的， 但是客户端每处理好
//发送信息 
void send_msg(msg now,int fd) 
{
    write(fd, &now, sizeof(msg));
    usleep(100000);
}
//打印信息
void pr_msg(msg now)
{
    printf("msgtype = %d, account = %s, password = %s, name = %s, data = %s, other = %s\n",\
    now.msgtype, now.account, now.password, now.selfname, now.msgdata, now.other);
}
// 服务器处理消息的函数
void hand_msg(int fd,msg now)
{
    //服务器每次收到消息，都追加到文本文件里面
    file_w(now); 
    switch (now.msgtype)
    {
        case MSG_REG: // 注册
            register_user(fd, now.password, now.selfname);
            break;
        case MSG_LOG: // 登录
            login_user(fd, now.account, now.password);
            break;
        case MSG_OUT: // 注销（退出登录）
            logout_user(fd, now.account);
            break;
        case MSG_DEL: // 删除账户
            delete_user(fd, now.account);
            break;
        case MSG_ONE: // 私聊
            send_one_user(fd, now.account, now.selfname, now.other, now.msgdata);
            break;
        case MSG_CRQ:   //建群
            create_qun(fd, now.account, now.msgdata);
            break;
        case MSG_INQ: // 加入群（msgdata 传群ID）
            join_qun(fd, now.account, atoi(now.msgdata));
            break;
        case MSG_QEQ: // 退出群
            quit_qun(fd, now.account, atoi(now.msgdata));
            break;
        case MSG_DEQ: // 解散群（自己必须是群主）根据群id
            dismiss_qun(fd, now.account, atoi(now.msgdata));
            break;
        case MSG_FIQ: // 查询加入的群列表
            query_qun_list(fd, now.account);
            break;
        case MSG_INU: // 添加好友（other为对方账号）
            add_friend(fd, now.account, now.other);
            break;
        case MSG_DEU: // 删除好友
            delete_friend(fd, now.account, now.other);
            break;
        case MSG_FIU: // 查询好友列表
            query_friend_list(fd, now.account);

        case MSG_ALL: // 群聊
            send_qun_user(fd, now.account, now.selfname, now.msgdata, atoi(now.password));
            break;
        case MSG_TQU: // 踢人
            delete_qun_user(fd, now.account, now.other, atoi(now.password));
            break;    
        case MSG_SQM: // 设置群权限
            set_qun_permission(fd, now.account, now.other, atoi(now.msgdata), atoi(now.password));
            break;

        case MSG_JIN: // 禁言
            jin_qun_user(fd, now.account, now.other, atoi(now.password));
            break;    
        case MSG_JIE: // 解言
            jie_qun_user(fd, now.account, now.other, atoi(now.password));
            break;

        default:
            printf("收到未知消息类型: %d\n", now.msgtype);
            break;
    }
}
//注册
void register_user(int fd, const char *password, const char *name)
{
    msg now = {0}; //返回的消息
    now.msgtype = MSG_REG; 
   // 获取时间戳生成账号
    int flag = 1;
    long  num = time(NULL); //生成账号
    if(num == -1 ) // 获取失败
    {
        printf("time获取账号名失败\n");
        strcpy(now.msgdata, "注册失败");
        flag = 0;
    }
    if(flag)//生成账号成功
    {
        //更新返回的信息
        sprintf(now.account, "%ld", num);
        //更新数据库
    //    int sql_insert_user(const char *account, const char *password, const char *name) 
        int ret = sql_insert_user(now.account, password, name);
        if(ret != 0)
        {
            strcpy(now.msgdata, "注册失败");
        }
        else
            strcpy(now.msgdata, "注册成功");
    }
    send_msg(now,fd); //返回消息
}
// 登陆账号
void login_user(int fd, const char *account, const char *password)
{
    // 县查询账号是否存在，
    //判断密码是否正确
    //判断账号是否登陆
    //更新数据库
    msg now = {0};
    now.msgtype = MSG_LOG; 
    sql_user user = sql_get_user_by_account(account);
    strcpy(now.selfname, user.name);
    if(strcmp(account, user.account) != 0)
    {
        strcpy(now.msgdata, "账号不存在");
    }
    else 
    {
        if(strcmp(password, user.password) != 0)
        {
            strcpy(now.msgdata, "密码错误");
        }
        else
        {
            if(user.fd) // 已经在线了
            {
                strcpy(now.msgdata, "已经在线");
            }
            else
            {
                //登陆成功
            //    strcpy(now.msgdata, "登陆成功");
                //更新用户套接字，套接字为 0 表示不在线， 1 在线
                //int sql_update_user_status(const char *account, int fd)
                int ret = sql_update_user_status(account, fd);
                if(ret != 0)
                {
                    strcpy(now.msgdata, "登陆失败");
                }
                else
                    strcpy(now.msgdata, "登陆成功");
                }
        }
    }
    send_msg(now,fd); //返回消息
}
// 退出登陆
void logout_user(int fd, const char *account)
{
    msg now = {0};
    now.msgtype = MSG_OUT; 
    int ret = sql_update_user_status(account, 0);
    if(ret != 0)
    {
        strcpy(now.msgdata, "退出失败");
    }
    else
    {
        strcpy(now.msgdata, "退出成功");
    }
    send_msg(now,fd); //返回消息
}
// 注销账号
void delete_user(int fd, const char *account)//密码已经客户端判断过了,且能登陆账号一定存在
{
    msg now = {0};
    now.msgtype = MSG_DEL; 
    //int sql_delete_user(const char *account)
    int ret = sql_delete_user(account);
    if(ret != 0)
    {
        strcpy(now.msgdata, "注销失败");
    }
    else
    {
        strcpy(now.msgdata, "注销成功");
    }
    pr_msg(now);
    send_msg(now,fd); //返回消息
}
// 添加好友 a 添加 b
void add_friend(int fd, const char *a, const char *b)
{
    msg now = {0};
    now.msgtype = MSG_INU; 
    // CREATE TABLE user_friend (
    // a CHAR(16) NOT NULL,                   -- 用户A账号
    // b CHAR(16) NOT NULL,                   -- 用户B账号
    // 先判断是否有这个人
    sql_user user = sql_get_user_by_account(b);
    if(strcmp(user.account, b) != 0) //用户不存在
    {
        strcpy(now.msgdata, "账号不存在");
    }
    else //用户存在
    {
        //是否是好友
        int ret = sql_is_friend(a, b);
        if(ret == 0)
        {
            strcpy(now.msgdata, "原来就是好友");
        }
        else
        {
            //更新数据库
            ret = sql_add_friend(a,b);
            if(ret != 0)
            {
                strcpy(now.msgdata, "添加失败");
            }
            else
            {
                strcpy(now.msgdata, "添加成功");
            }
        }
    }
    send_msg(now,fd); //返回消息
}
//删除好友
void delete_friend(int fd, const char *a, const char *b)
{
    msg now = {0};
    now.msgtype = MSG_INU; 
    // 先判断是否有这个人
    sql_user user = sql_get_user_by_account(b);
    if(strcmp(user.account, b) != 0) //用户不存在
    {
        strcpy(now.msgdata, "账号不存在");
    }
    else //用户存在
    {
        //判断是否为好友
        //int sql_is_friend(const char *a, const char *b)
        int ret = sql_is_friend(a, b);
        if(ret != 0)
        {
            strcpy(now.msgdata, "不是好友");
        }
        else //是好友
        {
            //更新数据库
            ret = sql_delete_friend(a, b);
            if(ret != 0)
            {
                strcpy(now.msgdata, "删除失败");
            }
            else
            {
                strcpy(now.msgdata, "删除成功");
            }
        }
    }
    send_msg(now,fd); //返回消息
}
//返回好友列表
void query_friend_list(int fd, const char *account)
{
    //结构限制，只能一个一个发送
    msg now = {0};
    now.msgtype = MSG_FIU; 
    int ret = sql_query_friend_list(fd, account);
    if(ret != 0)
    {
        strcpy(now.msgdata, "查询失败");
    }
    else
    {
        strcpy(now.msgdata, "查询成功");
    }
    send_msg(now,fd); //返回消息
}
// 私聊 ：建立在好友功能之上，且好友得在线
void send_one_user(int fd, const char *from,const char *name, const char *to, const char *data)
{
    //用户是否存在
    //用户是否在线
    //用户是否是好友
    //发送消息
    msg now = {0};
    now.msgtype = MSG_ONE; 
    strcpy(now.account, from);
    sql_user user = sql_get_user_by_account(to);
    if(strcmp(to, user.account) != 0)
    {
        strcpy(now.msgdata, "用户不存在");
    }
    else 
    {
        if(user.fd == 0)
        {
            strcpy(now.msgdata, "用户不在线");
        }
        else
        {
            int ret = sql_is_friend(from, to);
            if(ret != 0)
            {
                strcpy(now.msgdata, "不是好友");
            }
            else
            {
                //发送给接收方
                msg fff = {0};
                fff.msgtype = MSG_ONE; 
                strcpy(fff.msgdata, data);
                strcpy(fff.account, from);
                strcpy(fff.selfname, name);
                send_msg(fff,user.fd);
                strcpy(now.msgdata, "发送成功");
            }
        }
    }
    //返回给发送方
    send_msg(now,fd); //返回消息
}

//建群 群名字和群主都可以重复 但是有一个关键字（可以看成群账号） 一定不会重复，自动生存
void create_qun(int fd, const char *account, const char *qun_name)
{
    msg now = {0};
    now.msgtype = MSG_CRQ; 
    //int sql_create_qun(const char *qun_name, const char *qunzhu) 
    int ret = sql_create_qun(qun_name, account);
    if(ret != 0)
    {
        strcpy(now.msgdata, "建群失败");
    }
    else
    {
        strcpy(now.msgdata, "建群成功");
    }
    send_msg(now,fd); //返回消息
}
void dismiss_qun(int fd, const char *account, int qun_id)
{
    msg now = {0};
    now.msgtype = MSG_DEQ; 
    //int sql_dismiss_qun(int qun_id, const char *qunzhu) 
    int ret = sql_dismiss_qun(qun_id, account);
    if(ret != 0)
    {
        strcpy(now.msgdata, "解散群失败");
    }
    else
    {
        strcpy(now.msgdata, "解散群成功");
    }
    send_msg(now,fd); //返回消息
}
void query_qun_list(int fd, const char *account)
{
    //结构限制，只能一个一个发送
    msg now = {0};
    now.msgtype = MSG_FIQ; 
    int ret = sql_query_qun_list(fd, account);
    if(ret != 0)
    {
        strcpy(now.msgdata, "查询失败");
    }
    else
    {
        strcpy(now.msgdata, "查询成功");
    }
    send_msg(now,fd); //返回消息
}
//加群
void join_qun(int fd, const char *account, int qun_id)
{
    //int sql_join_qun(int qun_id, const char *account)
    msg now = {0};
    now.msgtype = MSG_INQ; 
    int ret = sql_join_qun(qun_id, account);
    if(ret != 0)
    {
        strcpy(now.msgdata, "加群失败");
    }
    else
    {
        strcpy(now.msgdata, "加群成功");
    }
    send_msg(now,fd); //返回消息
}
//退群
void quit_qun(int fd, const char *account, int qun_id)
{
    //int sql_quit_qun(int qun_id, const char *account)
    msg now = {0};
    now.msgtype = MSG_QEQ; 
    int ret = sql_quit_qun(qun_id, account);
    if(ret != 0)
    {
        strcpy(now.msgdata, "退群失败");
    }
    else
    {
        strcpy(now.msgdata, "退群成功");
    }
    send_msg(now,fd); //返回消息
}
//群聊
void send_qun_user(int fd, const char *from, const char *name, const char *data, int qun_id)
{
    //这里不判断是否存在群了，反正只有群成员可以发送
    msg now = {0};
    now.msgtype = MSG_ALL; 
    //是否在群里
    int ret = sql_is_qun_user(from, qun_id);
    if(ret != 0)
    {
        strcpy(now.msgdata, "不是群成员");
    }
    else
    {
        strcpy(now.msgdata, "发送成功");
        sql_send_qun_members(qun_id, from, name, data);
        //发送所有在线成员，选获取在线成员表，再发送
    }
    send_msg(now,fd); //返回消息
}
void set_qun_permission(int fd, const char *a, const char *b, int permission, int qun_id)
{
    msg now = {0};
    now.msgtype = MSG_SQM; 
    //是否在群里
    //
    int ret = sql_is_qun_user(from, qun_id);
    if(ret != 0)
    {
        strcpy(now.msgdata, "不是群成员");
    }
    else
    {
        strcpy(now.msgdata, "发送成功");
        sql_send_qun_members(qun_id, from, name, data);
        //发送所有在线成员，选获取在线成员表，再发送
    }
    send_msg(now,fd); //返回消息
}

void delete_qun_user(int fd, const char *a, const char *b, int qun_id)
{
    msg now = {0};
    now.msgtype = MSG_TQU; 


    send_msg(now,fd); //返回消息
}

void jin_qun_user(int fd, const char *a, const char *b, int qun_id)
{
    msg now = {0};
    now.msgtype = MSG_JIN; 
     send_msg(now,fd); //返回消息
}
void jie_qun_user(int fd, const char *a, const char *b, int qun_id)
{
    msg now = {0};
    now.msgtype = MSG_JIE; 
    send_msg(now,fd); //返回消息
}

// 将消息写入日志文件
// 这里还没有对消息详细处理，有时间可以把消息分分类
// 还可以增加消息删除，和查看
void file_w(msg now)
{
    FILE *fp = fopen("chat_log.txt", "a"); // 以追加模式打开
    if (fp == NULL) {
        perror("打开日志文件失败\n");
        return;
    }
    // 获取当前时间
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    // 写入时间
    fprintf(fp, "时间：%04d年%02d月%02d日 %02d时%02d分%02d秒\n",
            tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
            tm->tm_hour, tm->tm_min, tm->tm_sec);
    // 写入聊天内容
    fprintf(fp, "类型：[%d] 内容：[%s] 对 [%s] 说：%s\n\n", now.msgtype, now.account, now.other, now.msgdata);

    fclose(fp);
}

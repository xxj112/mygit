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
    log_request(now);

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
        case MSG_JIE: // 解言
            jie_qun_user(fd, now.account, now.other, atoi(now.password));
            break;
        case MSG_JIN: // 禁言
            jin_qun_user(fd, now.account, now.other, atoi(now.password));
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
    log_response(now);
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
    log_response(now);
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
    log_response(now);
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
    log_response(now);
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
    log_response(now);
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
    log_response(now);
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
    log_response(now);
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
    log_response(now);
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
    log_response(now);
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
    log_response(now);
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
    log_response(now);
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
    log_response(now);
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
    log_response(now);
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

        //是否禁言int sql_get_mute_status(int qun_id, const char *account)
        ret = sql_get_mute_status(qun_id, from);
        if(ret == 0)
        {
            sql_send_qun_members(qun_id, from, name, data);
            strcpy(now.msgdata, "发送成功");
        }
        else
        {
            strcpy(now.msgdata, "处于禁言状态");
        }
    }
    send_msg(now,fd); //返回消息
    log_response(now);
}
//权限设置 a 设置 b qun_id权限permission
void set_qun_permission(int fd, const char *a, const char *b, int permission, int qun_id)
{
    msg now = {0};
    now.msgtype = MSG_SQM; 
    //是否在群里int sql_is_qun_user(const char *account, int qun_id)
    int rea = sql_is_qun_user(a, qun_id);
    int reb = sql_is_qun_user(b, qun_id);
    if(rea != 0 || reb != 0)
    {
        strcpy(now.msgdata, "不是群成员");
    }
    else
    {
        // 权限判断int sql_query_permission_quns(const char *account, int qun_id)
        rea = sql_query_permission_quns(a, qun_id);
        reb = sql_query_permission_quns(b, qun_id);
        if(rea == -1 || reb == -1)
        {
            strcpy(now.msgdata, "查询失败");
           /// return ;
        }
        else
        {
            if(rea > reb && permission <= rea && permission < 2)
            {
                //设置群权限int sql_set_permission_quns(const char *account,int permission,  int qun_id)
                reb = sql_set_permission_quns(b, permission, qun_id);
                if(reb == 0)
                {
                    strcpy(now.msgdata, "设置成功");
                }
                else
                {
                    strcpy(now.msgdata, "设置失败");
                }
            }
            else
            {
                strcpy(now.msgdata, "权限不够");
            }
        }
    }
    send_msg(now,fd); //返回消息
    log_response(now);
}

void delete_qun_user(int fd, const char *a, const char *b, int qun_id)
{
    msg now = {0};
    now.msgtype = MSG_TQU; 
    int rea = sql_is_qun_user(a, qun_id);
    int reb = sql_is_qun_user(b, qun_id);
    if(rea != 0 || reb != 0)
    {
        strcpy(now.msgdata, "不是群成员");
    }
    else
    {
        // 权限判断int sql_query_permission_quns(const char *account, int qun_id)
        rea = sql_query_permission_quns(a, qun_id);
        reb = sql_query_permission_quns(b, qun_id);
        if(rea == -1 || reb == -1)
        {
            strcpy(now.msgdata, "查询失败");
        }
        else
        {
            if(rea > reb )
            {
                //踢人int sql_quit_qun(int qun_id, const char *account) 
                reb = sql_quit_qun(qun_id, b);
                if(reb == 0)
                {
                    strcpy(now.msgdata, "踢人成功");
                }
                else
                {
                    strcpy(now.msgdata, "踢人失败");
                }
            }
            else
            {
                strcpy(now.msgdata, "权限不够");
            }
        }
    }

    send_msg(now,fd); //返回消息
    log_response(now);
}

void jin_qun_user(int fd, const char *a, const char *b, int qun_id)
{
    msg now = {0};
    now.msgtype = MSG_JIN; 

    int rea = sql_is_qun_user(a, qun_id);
    int reb = sql_is_qun_user(b, qun_id);
    if(rea != 0 || reb != 0)
    {
        strcpy(now.msgdata, "不是群成员");
    }
    else
    {
        // 权限判断int sql_query_permission_quns(const char *account, int qun_id)
        rea = sql_query_permission_quns(a, qun_id);
        reb = sql_query_permission_quns(b, qun_id);
        if(rea == -1 || reb == -1)
        {
            strcpy(now.msgdata, "查询失败");
        }
        else
        {
            if(rea > reb )
            {
                //设置int sql_set_mute(int qun_id, const char *account, int mute_flag)
                reb = sql_set_mute(qun_id, b, 1);
                if(reb == 0)
                {
                    strcpy(now.msgdata, "设置成功");
                }
                else
                {
                    strcpy(now.msgdata, "设置失败");
                }
            }
            else
            {
                strcpy(now.msgdata, "权限不够");
            }
        }
    }


    send_msg(now,fd); //返回消息
    log_response(now);
}
void jie_qun_user(int fd, const char *a, const char *b, int qun_id)
{
    msg now = {0};
    now.msgtype = MSG_JIE; 
    int rea = sql_is_qun_user(a, qun_id);
    int reb = sql_is_qun_user(b, qun_id);
    if(rea != 0 || reb != 0)
    {
        strcpy(now.msgdata, "不是群成员");
    }
    else
    {
        // 权限判断int sql_query_permission_quns(const char *account, int qun_id)
        rea = sql_query_permission_quns(a, qun_id);
        reb = sql_query_permission_quns(b, qun_id);
        if(rea == -1 || reb == -1)
        {
            strcpy(now.msgdata, "查询失败");
        }
        else
        {
            if(rea > reb )
            {
                //设置int sql_set_mute(int qun_id, const char *account, int mute_flag)
                reb = sql_set_mute(qun_id, b, 0);
                if(reb == 0)
                {
                    strcpy(now.msgdata, "设置成功");
                }
                else
                {
                    strcpy(now.msgdata, "设置失败");
                }
            }
            else
            {
                strcpy(now.msgdata, "权限不够");
            }
        }
    }
    send_msg(now,fd); //返回消息
    log_response(now);
}

// 将消息写入日志文件
// 还可以增加消息删除，和查看
//收到结果
void log_request(msg now)
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

   switch (now.msgtype) {
    case MSG_REG:
        fprintf(fp, "[注册] 账号: %s 姓名: %s 密码: %s\n", now.account, now.selfname, now.password);
        break;
    case MSG_LOG:
        fprintf(fp, "[登录] 账号: %s 密码: %s\n", now.account, now.password);
        break;
    case MSG_OUT:
        fprintf(fp, "[退出] 账号: %s\n", now.account);
        break;
    case MSG_DEL:
        fprintf(fp, "[注销] 账号: %s\n", now.account);
        break;
    case MSG_ONE:
        fprintf(fp, "[私聊] 发送者: %s (%s) . 接收者: %s 消息: %s\n", now.account, now.selfname, now.other, now.msgdata);
        break;
    case MSG_ALL:
        fprintf(fp, "[群聊] 群成员: %s (%s) . 群消息: %s\n", now.account, now.selfname, now.msgdata);
        break;
    case MSG_INU:
        fprintf(fp, "[加好友] %s . %s\n", now.account, now.other);
        break;
    case MSG_DEU:
        fprintf(fp, "[删除好友] %s . %s\n", now.account, now.other);
        break;
    case MSG_CRQ:
        fprintf(fp, "[建群] 群主: %s 群名: %s\n", now.account, now.msgdata);
        break;
    case MSG_INQ:
        fprintf(fp, "[加群] 账号: %s 群ID: %s\n", now.account, now.other);
        break;
    case MSG_QEQ:
        fprintf(fp, "[退群] 账号: %s 群ID: %s\n", now.account, now.other);
        break;
    case MSG_DEQ:
        fprintf(fp, "[解散群] 群主: %s 群ID: %s\n", now.account, now.other);
        break;
    case MSG_FIU:
        fprintf(fp, "[查询好友列表] 账号: %s\n", now.account);
        break;
    case MSG_FIQ:
        fprintf(fp, "[查询群列表] 账号: %s\n", now.account);
        break;
    case MSG_TQU:
        fprintf(fp, "[踢人] 管理员: %s 踢出: %s 群ID: %s\n", now.account, now.other, now.msgdata);
        break;
    case MSG_SQM:
        fprintf(fp, "[设置群权限] 操作者: %s 目标: %s 权限: %s 群ID: %s\n", now.account, now.other, now.password, now.msgdata);
        break;
    case MSG_JIN:
        fprintf(fp, "[禁言] 操作者: %s 目标: %s 群ID: %s\n", now.account, now.other, now.msgdata);
        break;
    case MSG_JIE:
        fprintf(fp, "[解禁] 操作者: %s 目标: %s 群ID: %s\n", now.account, now.other, now.msgdata);
        break;
    default:
        fprintf(fp, "[未知消息] 类型: %d\n", now.msgtype);
        break;
    }

    fclose(fp);
}

//返回结果
void log_response(msg now) {
    FILE *fp = fopen("chat_log.txt", "a"); // 以追加模式打开
    if (fp == NULL) {
        perror("打开日志文件失败\n");
        return;
    }

    switch (now.msgtype) {
    case MSG_REG:
        fprintf(fp, "[注册结果] 返回信息: %s\n", now.msgdata);
        break;
    case MSG_LOG:
        fprintf(fp, "[登录结果] 返回信息: %s\n", now.msgdata);
        break;
    case MSG_OUT:
        fprintf(fp, "[退出登录结果] 返回信息: %s\n", now.msgdata);
        break;
    case MSG_DEL:
        fprintf(fp, "[注销结果] 返回信息: %s\n", now.msgdata);
        break;
    case MSG_ONE:
        fprintf(fp, "[私聊结果] 返回信息: %s\n", now.msgdata);
        break;
    case MSG_ALL:
        fprintf(fp, "[群聊结果] 返回信息: %s\n", now.msgdata);
        break;
    case MSG_INU:
        fprintf(fp, "[添加好友结果] 返回信息: %s\n", now.msgdata);
        break;
    case MSG_DEU:
        fprintf(fp, "[删除好友结果] 返回信息: %s\n", now.msgdata);
        break;
    case MSG_FIU:
        fprintf(fp, "[查询好友列表结果] 返回信息: %s\n", now.msgdata);
        break;
    case MSG_CRQ:
        fprintf(fp, "[建群结果] 返回信息: %s\n", now.msgdata);
        break;
    case MSG_INQ:
        fprintf(fp, "[加群结果] 返回信息: %s\n", now.msgdata);
        break;
    case MSG_QEQ:
        fprintf(fp, "[退群结果] 返回信息: %s\n", now.msgdata);
        break;
    case MSG_DEQ:
        fprintf(fp, "[解散群结果] 返回信息: %s\n", now.msgdata);
        break;
    case MSG_FIQ:
        fprintf(fp, "[查询群列表结果] 返回信息: %s\n", now.msgdata);
        break;
    case MSG_TQU:
        fprintf(fp, "[踢人结果] 返回信息: %s\n", now.msgdata);
        break;
    case MSG_SQM:
        fprintf(fp, "[设置群权限结果] 返回信息: %s\n", now.msgdata);
        break;
    case MSG_JIE:
        fprintf(fp, "[解除禁言结果] 返回信息: %s\n", now.msgdata);
        break;
    case MSG_JIN:
        fprintf(fp, "[禁言结果] 返回信息: %s\n", now.msgdata);
        break;
    default:
        fprintf(fp, "[未知类型结果] 返回信息: %s\n", now.msgdata);
        break;
    }

     fclose(fp);
}

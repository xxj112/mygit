#include "ser_op.h"

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
    switch(now.msgtype)
    {
        case MSG_REG: //注册
            register_user(fd, now.selfname, now.password);
            break;
        case MSG_LOG: //登陆
            login_user(fd, now.account, now.password);
            break;
        case MSG_OUT: //退出
            lgout_user(fd, now.account);
            break;
        case MSG_DEL: //删除
            delete_user(fd, now.account);
            break;
        case MSG_ALL: //群发
            send_all_user(fd, now);
            break;
        case MSG_ONE: //私聊
            send_one_user(fd, now);
            break;
        default:
            break;    
    }
}
void send_all_user(int fd, msg now)
{
    printf("send_all_user\n");
    pr_msg(now);
    printf("1\n");
    if(strcmp(now.other, "end") == 0) // 退出群聊
    {

    printf("2\n");
        mysql_update_user_qun(now.account, 0);
    }
    else if(strcmp(now.other, "start") == 0) // 进入群聊
    {

     printf("3\n");
        mysql_update_user_qun(now.account, 1);
    }
    else  //发送消息
    {

    printf("4\n");
        int cnt = get_accounts_qun(); //获取所有群用户
        printf("cnt = %d", cnt);
        for(int i = 0; i < cnt; i++)
        {
            sql_user w = mysql_query_users(accounts[i]); 
            printf("Account = %s\n", accounts[i]);
            if(w.fd == fd) continue; // 自己跳过
            if(w.fd != 0) //在线 ，其实在群里一定在线，防止中途退出
            {
                send_msg(now,w.fd); //发送信息
            }
        }
    }

    printf("5\n");
}
void register_user(int fd,char * name, char * password)
{
    msg now = {0};
    int flag = 1;
    long  num = time(NULL); //生成账号
    if(num == -1 ) // 获取失败
    {
    //    printf("获取账号名失败\n");
        strcpy(now.msgdata, "获取账号名失败\n");
        flag = 0;
    }
    if(flag)//生成账号
    {
        //更新返回的数据
     //   char str[16];
        now.msgtype = MSG_REG;
        sprintf(now.account, "%ld", num);
        strcpy(now.password,password);
        strcpy(now.selfname,name);
        strcpy(now.msgdata, "注册成功\n");
        //更新数据库
        sql_user user = {0};
        strcpy(user.account,now.account);
        strcpy(user.password,now.password);
        strcpy(user.selfname,now.selfname);
        mysql_insert_user(user);
    }
    send_msg(now,fd); //返回消息
}
//登陆的时候，要更新 数据库对应的套接字
void login_user(int fd, char * account, char * password)
{
    sql_user now = mysql_query_users(account);
    msg rec = {0};
    if(strcmp(account, now.account) != 0) //账号不存在
    {
        strcpy(rec.msgdata, "账号不存在");
    }
    else 
    {
        if(strcmp(password, now.password) != 0) // 密码不对
        {
            strcpy(rec.msgdata, "密码错误");
        }
        else
        {   
            strcpy(rec.msgdata, "success");
            mysql_update_user_status(account, fd); //修改套接字
        }
    }
    strcpy(rec.selfname, now.selfname);
    send_msg(rec,fd); //返回消息
  
}
void delete_user(int fd,char * account) //密码已经客户端判断过了,且能登陆账号一定存在
{
    msg rec = {0};
    mysql_delete_user(account);
    sql_user now = mysql_query_users(account);
    if(now.account[0] == '\0')
    strcpy(rec.msgdata, "注销成功");
    else strcpy(rec.msgdata, "注销失败");
    send_msg(rec,fd);
}
//void send_all_user(fd);
void send_one_user(int fd, msg now)
{
    // 查找目标用户
    sql_user target = mysql_query_users(now.other); // 根据账号 用户信息
    if(strcmp(target.account, now.other) != 0)
    {
        //用户不存在
        strcpy(now.msgdata, "用户不存在");
    }
    else if(target.fd == 0)
    {
        strcpy(now.msgdata, "用户不在线");
    }
    else
    {
        send_msg(now, target.fd);
        strcpy(now.msgdata, "发送成功");
    }
    send_msg(now, fd);
}
void lgout_user(int fd, char * account)//退出 把fd赋值为0，不是不在线
{
   mysql_update_user_status(account, 0);
}
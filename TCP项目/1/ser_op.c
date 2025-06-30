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
        case MSG_DEL: //删除
            delete_user(fd, now.account);
            break;
        case MSG_ALL: //群发
         //   send_all_user(fd, );
            break;
        default:
            break;    
    }
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
        char str[16];
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

}
void delete_user(int fd,char * account)
{

}
//void send_all_user(fd);


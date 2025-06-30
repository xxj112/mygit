#include "cli_op.h"
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
    //if()
}

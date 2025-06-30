#ifndef  _SHARE_H_
#define _SHARE_H_

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h> 
#include <netinet/in.h>      
#include <pthread.h>
#include <time.h>
#define SERVER_IP "192.168.138.60"
// 端口号用短整形 2字节  0~65535 
// 不建议使用10000以下的有肯呢个别占用
#define SERVER_RORT 8848
typedef struct node{
    char account[16]; //自身的账号; 
    char password[16];//自身的密码; 
    char selfname[16]; //自身的姓名；
    int fd;
    int qun;
}sql_user;

typedef struct user_msg{
    int msgtype; //消息类型;
    char account[16]; //自身的账号; 
    char password[16];//自身的密码; 
    char selfname[16]; //自身的姓名；
    char msgdata[64]; //发送的消息; 
    char other[16]; //别人的账号; 
}msg;
enum{MSG_REG=1,MSG_LOG,MSG_DEL,MSG_ONE,MSG_ALL}; //消息类型：注册、登陆、注销、私聊和群发；
#endif // ! _SHARE_H_


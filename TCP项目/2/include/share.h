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
#include <pthread.h>
#include <sys/epoll.h>

#define SERVER_IP "192.168.0.9"
// 端口号用短整形 2字节  0~65535 
// 不建议使用10000以下的有肯呢个别占用
#define SERVER_RORT 8848
typedef struct user_msg{
    int msgtype; //消息类型;    
    char account[16]; //自身的账号; 
    char password[16];//自身的密码; 
    char selfname[16]; //自身的姓名；
    char msgdata[64]; //发送的消息; 
    char other[16]; //别人的账号; 
}msg;
//消息类型：
//   注册       登陆    注销     私聊    群发     退出      加群     退群    删除群   加好友    删除好友 创建群
enum{MSG_REG=1,MSG_LOG,MSG_DEL,MSG_ONE,MSG_ALL,MSG_OUT,MSG_INQ,MSG_QEQ,MSG_DEQ,MSG_INU,MSG_DEU,MSG_CRQ
//查询好友 查询群     踢人     设置群权限 禁言     解言
,MSG_FIU, MSG_FIQ, MSG_TQU, MSG_SQM, MSG_JIN, MSG_JIE}; 
#endif // ! _SHARE_H_


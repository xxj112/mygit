#include "share.h"
#include "cli_op.h"
msg rec_msg = {0}; //实时接受的消息
msg reg_msg = {0};      //注册专用信息
msg sen_msg = {0};      //发送信息

int LG = 0; //标记是否登陆 1：登陆 0：未登录
char my_account[16];
char my_password[16];
char my_name[16];
int skt;
void *myfunc(void *arg);
int main(void)
{
    //1、创建通信套接字
    skt = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in myaddr = {0};
    myaddr.sin_addr.s_addr = inet_addr(SERVER_IP);//IP
    myaddr.sin_family = AF_INET;//IP类型
    myaddr.sin_port = htons(SERVER_RORT);//端口号
    //2、连接服务器
    int ret = connect(skt, (struct sockaddr *)&myaddr, sizeof(myaddr));
    if(ret < 0)
    {
        printf("connect eroor\n");
        return 0;
    }
    printf("connect success\n");
    pthread_t pd = 0;
    pthread_create(&pd, NULL, myfunc, NULL);
    while(1)
    {
        if(LG) //登陆后菜单二
        {
            two_menu();
        }
        else
        {
            one_menu();
        }
    }
    //关闭线程，并等待推出
    pthread_cancel(pd);
    pthread_join(pd, NULL);
    close(skt);
    return 0;
}

void *myfunc(void *arg) //线程负责接受消息
{
    //消息种类 ： 1 私聊 2 群聊 3 注册 
    //注意注册信息要单独存储 防止刷新 
    //线程 读
    while(1)
    {
        if(read(skt,&rec_msg, sizeof(msg)) == 0)
            break;
        msg_handle(rec_msg);
    }
}

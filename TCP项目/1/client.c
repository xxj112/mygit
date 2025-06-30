#include "share.h"
msg rec_msg = {0}; //实时接受的消息
msg reg_msg = {0};      //注册专用信息
msg sen_msg = {0};      //发送信息

int LG = 0; //标记是否登陆 1：登陆 0：未登录
char my_account[16];
char my_password[16];
char my_name[16];
int skt;
void *myfunc(void *arg);
void Register(void);
void Login(void);    
void one_menu(void); //一级菜单
void two_menu(void); //二级菜单
void send_all(void); //群发
void Delete_account(void); // 注销账号
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
void one_menu(void) 
{
    int num = 0;
    printf("1. 注册\n2. 登录\n");
    scanf("%d",&num);
    switch (num) 
    {
        case 1: //注册
            Register();
            break;
        case 2: //登陆
            Login();
            break;
        default:
            printf("Default case\n");
    }

}
void two_menu(void)
{
    int num = 2;
    printf("登陆成功");
    printf("%s %s %s\n",my_account,my_name,my_password);
    while(1)
    {
        printf("1. 私聊\n2. 群发\n3.退出\n4. 注销\n");
        scanf("%d",&num);
        switch (num) 
        {
            case 1: //私聊
//                one_send();
            //    Register();
              //  printf("Case 1\n");
                break;
            case 2: //群发
                send_all();
                break;
            case 3: //退出
                LG = 0;
                return ;
                break;
            case 4: //注销
                Delete_account();
                return ;
                break;
            default:
                printf("Default case\n");
        }
    }
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
        if(rec_msg.msgtype == MSG_REG) //更新注册内容防止，消息覆盖
        {
            reg_msg = rec_msg;
        }
        else
        printf("收到内容：%s\n",rec_msg.msgdata);
    }
}
void Login(void)
{
    printf("请输入账户：");
    scanf("%s",my_account);
    printf("请输入密码：");
    scanf("%s",my_password);
    strcpy(reg_msg.password,my_password);
    strcpy(reg_msg.account,my_account);
    reg_msg.msgtype = MSG_LOG;
    send_msg(reg_msg);
    if(strcmp(rec_msg.msgdata,"success") != 0) //登陆失败
    {
     //   printf("注册失败\n");
        printf("任意键返回\n");
        getchar();
        getchar();
        return ;
    }
    // 登陆成功
    LG = 1;
    strcpy(my_name,rec_msg.selfname);
}
void Register(void)
{
    printf("请输入用户名：");
    scanf("%s",my_name);
    printf("请输入密码：");
    scanf("%s",my_password);
    printf("请再次输入密码：");
    scanf("%s",reg_msg.password);
    if(strcmp(reg_msg.password,my_password) != 0)
    {
        printf("两次密码不一样\n");
        return ;
    }
    strcpy(reg_msg.selfname,my_name);
    reg_msg.msgtype = MSG_REG;
    reg_msg.account[0] = '\0';
    send_msg(reg_msg,skt);
    if(reg_msg.account[0] == '\0')
    {
        printf("%s\n",reg_msg.msgdata);
     //   printf("注册失败\n");
        printf("任意键返回\n");
        getchar();
        getchar();
        return ;
    }
    strcpy(my_account,reg_msg.account);
    printf("我的账号是：%s\n",my_account);
    printf("任意键返回\n");
    getchar();
    getchar();
}
void Delete_account(void)
{
    strcpy(sen_msg.account, my_account);
    strcpy(sen_msg.password, my_password);
   // memset(sen_msg.msgdata, 0, sizeof(sen_msg.msgdata));
   // strcpy(sen_msg., );
    sen_msg.msgtype = MSG_DEL;
    send_msg(sen_msg);
    if(strcmp(rec_msg.msgdata, "注销成功") == 0)
    {
        printf("注销成功\n");
        LG = 0;
    }
    else 
    {
        printf("注销失败\n");
    }
}
void send_all(void)
{
    printf("请输入发送的消息：");
    scanf("%s",sen_msg.msgdata);
    sen_msg.msgtype = MSG_ALL;
    strcpy(sen_msg.account, my_account);
    strcpy(sen_msg.password, my_password);
    send_msg(sen_msg);
}

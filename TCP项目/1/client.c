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
void Register(void);
void Login(void);    
void one_menu(void); //一级菜单
void two_menu(void); //二级菜单
void threee_menu(void); //群聊界面
void send_all(void); //群发
void send_all_start(void); //进入群聊
void send_all_end(void); // 退出群聊
void Delete_account(void); // 注销账号
void send_single(void); // 单独发送
void lgout(void); //退出
void msg_handle(msg now); //处理接受的消息

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
        // case 2: //登陆
        //     Login();
        //     break;
        default:
            printf("Default case\n");
    }

}
void two_menu(void)
{
    int num = 2;
    while(1)
    {
        printf("1. 私聊\n2. 群发\n3. 退出\n4. 注销\n");
        scanf("%d",&num);
        switch (num) 
        {
            case 1: //私聊
                send_single();
                break;
            case 2: //群发
                threee_menu();
                break;
            case 3: //退出
                lgout();
                LG = 0;
                return ;
            case 4: //注销
                Delete_account();
                return;
            default:
                printf("Default case\n");
        }
    }
}
void threee_menu(void) //群聊界面
{
    int num = 2;
    send_all_start();
    while(1)
    {
        printf("1. 发送\n2. 退出\n");
        scanf("%d",&num);
        switch (num) 
        {
            case 1: //
                send_all();
                break;
            case 2: //
                send_all_end();
                return;
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
        msg_handle(rec_msg);
    }
}
void msg_handle(msg now)
{
    pr_time();
    switch(now.msgtype)
    {
        case MSG_REG: //注册信息单独存储，防止刷新
            reg_msg = now;
            break;
        // case MSG_LOG: //登陆
            
        //     break;
        // case MSG_OUT: //退出
            
        //     break;
        // case MSG_DEL: //删除
            
        //     break;
        case MSG_ALL: //群发
            printf("收到群聊的消息%s：%s\n", now.selfname, now.msgdata);
            break;
        case MSG_ONE: //私聊
            printf("收到%s的消息：%s\n", now.selfname, now.msgdata);
            break;
        default:
            printf("收到内容：%s\n", now.msgdata);
            break;    
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
    send_msg(reg_msg,skt);
    if(strcmp(rec_msg.msgdata,"success") != 0) //登陆失败
    {
   
        printf("%s\n",rec_msg.msgdata);
        printf("任意键返回\n");
        getchar();
        getchar();
        return ;
    }
    // 登陆成功
    LG = 1;
    
    strcpy(my_name,rec_msg.selfname);
    printf("登陆成功");
    printf("%s %s %s\n",my_account,my_name,my_password);
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
    printf("请输入密码:");
    char password[16] = {0};
    scanf("%s",password);
    if(strcmp(my_password, password) != 0) // 密码不对
    {
        printf("密码错误\n");
        return ;
    }
    strcpy(sen_msg.account, my_account);
    strcpy(sen_msg.password, my_password);
    sen_msg.msgtype = MSG_DEL;
    send_msg(sen_msg,skt);
    if(strcmp(rec_msg.msgdata, "注销成功") == 0)
    {
        printf("注销成功\n");
        LG = 0; //注销后自动退出
    }
    else 
    {
        printf("%s\n",rec_msg.msgdata);
//        printf("注销失败\n");
    }
}
void send_all(void)
{
    printf("请输入发送的消息：");
    scanf("%s",sen_msg.msgdata);
    sen_msg.msgtype = MSG_ALL;
    strcpy(sen_msg.other, "send");
    strcpy(sen_msg.account, my_account);
    strcpy(sen_msg.password, my_password);
    strcpy(sen_msg.selfname, my_name);
    send_msg(sen_msg,skt);
}
void send_all_start(void)
{
    strcpy(sen_msg.other, "start");
    sen_msg.msgtype = MSG_ALL;
    strcpy(sen_msg.account, my_account);
    send_msg(sen_msg,skt);
}
void send_all_end(void)
{
    strcpy(sen_msg.other, "end");
    sen_msg.msgtype = MSG_ALL;
    strcpy(sen_msg.account, my_account);
    send_msg(sen_msg,skt);
}
void send_single(void)
{
    printf("请输入目标用户：");
    scanf("%s",sen_msg.other);
    printf("请输入发送的消息：");
    scanf("%s",sen_msg.msgdata);
    sen_msg.msgtype = MSG_ONE;
    strcpy(sen_msg.account, my_account);
    strcpy(sen_msg.password, my_password);
    strcpy(sen_msg.selfname, my_name);
    send_msg(sen_msg,skt);
}
void lgout(void) //退出更改 fd状态
{
    strcpy(sen_msg.account, my_account);
    sen_msg.msgtype = MSG_OUT;
    send_msg(sen_msg,skt);
}


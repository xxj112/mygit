#include "cli_op.h"

extern msg rec_msg ; //实时接受的消息
extern msg reg_msg ;      //注册专用信息
extern msg sen_msg;      //发送信息

extern int LG; //标记是否登陆 1：登陆 0：未登录
extern char my_account[16];
extern char my_password[16];
extern char my_name[16];
extern int skt;


//发送信息 
void send_msg(msg now) 
{
    write(skt, &now, sizeof(msg));
    usleep(100000);
}
//打印信息
void pr_msg(msg now)
{
    printf("msgtype = %d, account = %s, password = %s, name = %s, data = %s, other = %s\n",\
    now.msgtype, now.account, now.password, now.selfname, now.msgdata, now.other);
}
// 打印时间
void pr_time(void)
{
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);

    printf("%04d年%02d月%02d日 %02d时%02d分%02d秒\n",
           tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
           tm->tm_hour, tm->tm_min, tm->tm_sec);
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
    int num = 0;
    while(LG)
    {
        printf("1. 私聊 2. 群发 3. 退出 4. 注销\n");
        printf("5. 加好友 6. 删除好友 7. 查询好友列表\n");
        printf("8. 创建群 9. 删除群 10. 查询加入的群\n");
        printf("11. 加群 12. 退群 13. 设置群权限 \n");
        printf("14. 禁言 15. 解禁 16. 踢人 \n");
        scanf("%d",&num);
        switch (num) 
        {
            case 1: //私聊
                Send_single();
                break;
            case 2: //群发
                Send_qun();
                break;
            case 3: //退出
                Lgout(); 
                break;
            case 4: //注销
                Delete_account();
                break;
            case 5: //加好友
                Add_user();
                break;
            case 6: //删除好友
                Delete_user();
                break;
            case 7: //好友列表
                Find_user_list();
                break;
             case 8: //创建群
                Create_qun();
                break;
            case 9: //删除群
                Delete_qun();
                break;
            case 10: //查询加入的群
                Find_qun_list();
                break;    
            case 11: //加群
                Add_qun();
                break;
            case 12: //退群
                Exit_qun();
                break;   
            case 13: //设置群权限
                Set_permission();
                break;    
            case 14: //禁言
                Set_jin_qun();
                break;
            case 15: //解禁
                Set_jie_qun();
                break; 
            case 16: //踢人
                delete_qun_user();
                break; 
            default:
                printf("Default case\n");
        }
    }
}
void threee_menu(void) //群聊界面
{
    int num = 2;
    while(1)
    {
        printf("1. 发送\n2. 退出\n");
        scanf("%d",&num);
        switch (num) 
        {
            case 1: //
               // send_all();
                break;
            case 2: //
             //   send_all_end();
                return;
            default:
                printf("Default case\n");
        }
    }
}
void msg_handle(msg now)
{
    pr_time();
    switch(now.msgtype)
    {
        case MSG_REG: //注册信息单独存储，防止刷新
            rec_register(now.account, now.msgdata);
          //  printf("收到内容：%s\n", now.msgdata);
            break;
        case MSG_LOG: //登陆
            rec_login(now.selfname, now.msgdata);
            break;
        case MSG_OUT: //退出
            rec_lgout(now.msgdata);
            break;
        case MSG_DEL: //删除
            rec_delete_account(now.msgdata);
            break;
        case MSG_ALL: //群发
            rec_send_qun(now.account, now.selfname, now.msgdata, now.other);
            break;
        case MSG_ONE: //私聊
            rec_send_single(now.account, now.selfname, now.msgdata);
            break;
        case MSG_INU: //
            rec_add_user(now.msgdata);
            break;
        case MSG_DEU:
            rec_delete_user(now.msgdata);
            break;
        case MSG_FIU:
            rec_find_user_list(now.account, now.selfname ,now.msgdata);
            break;
         case MSG_CRQ: //
            rec_create_qun(now.msgdata);
            break;
        case MSG_DEQ:
            rec_delete_qun(now.msgdata);
            break;
        case MSG_FIQ:
            rec_find_qun_list(now.account, now.selfname ,now.msgdata);
            break;
        case MSG_INQ:
            rec_add_qun(now.msgdata);
            break;
        case MSG_QEQ:
            rec_exit_qun(now.msgdata);
            break;
        case MSG_SQM: //这四个操作，都之返回信息
           // rec_delete_qun(now.msgdata);
          //  break;
        case MSG_JIN:
          
         //   break;
        case MSG_JIE:
            
          //  break;
        case MSG_TQU:
            rec_set_permission(now.msgdata);
            break;
        default:
            printf("收到内容：%s\n", now.msgdata);
            break;    
    }

}
void Register(void)
{
    printf("请输入用户名：");
    scanf("%s",my_name);
    printf("请输入密码：");
    scanf("%s",my_password);
    printf("请再次输入密码：");
    scanf("%s",sen_msg.password);
    if(strcmp(sen_msg.password,my_password) != 0)
    {
        printf("两次密码不一样\n");
        return ;
    }
    strcpy(sen_msg.selfname,my_name);
    sen_msg.msgtype = MSG_REG;
    send_msg(sen_msg);
    printf("任意键返回\n");
    getchar();
    getchar();
}
void rec_register(const char *account, const char *data)
{
    if(strcmp(data, "注册成功") == 0)
    {
        //更新账号并打印
        strcpy(my_account, account);
        printf("账号:%s\n", my_account);
    }
    printf("收到内容：%s\n", data);
}
void Login(void)
{
    printf("请输入账户：");
    scanf("%s",my_account);
    printf("请输入密码：");
    scanf("%s",my_password);

    strcpy(sen_msg.password,my_password);
    strcpy(sen_msg.account,my_account);
    sen_msg.msgtype = MSG_LOG;
    send_msg(sen_msg);
    printf("任意键返回\n");
    getchar();
    getchar();
    
}
void rec_login(const char *name, const char *data)
{
    if(strcmp(data, "登陆成功") == 0)
    {
        //更新账号并打印
        strcpy(my_name, name);
        printf("昵称:%s\n", my_name);
        LG = 1;
    }
    printf("收到内容：%s\n", data);
}
void Lgout()
{
    strcpy(sen_msg.account, my_account);
    sen_msg.msgtype = MSG_OUT;
    send_msg(sen_msg);
    printf("任意键返回\n");
    getchar();
    getchar();

}
void rec_lgout(const char *data)
{
    if(strcmp(data, "退出成功") == 0)
    {
        LG = 0;
    }
    printf("收到内容：%s\n", data);
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
    send_msg(sen_msg);
    printf("任意键返回\n");
    getchar();
    getchar();
}
void rec_delete_account(const char *data)
{
   // printf("1111\n");
    if(strcmp(data, "注销成功") == 0)
    {
        LG = 0;
    }
    printf("收到内容：%s\n", data);
}
void Send_single()
{
    printf("请输入目标用户：");
    scanf("%s",sen_msg.other);
    printf("请输入发送的消息：");
    scanf("%s",sen_msg.msgdata);
    sen_msg.msgtype = MSG_ONE;
    strcpy(sen_msg.account, my_account);
    strcpy(sen_msg.selfname, my_name);
    send_msg(sen_msg);
    printf("任意键返回\n");
    getchar();
    getchar();
}
void rec_send_single(const char *account, const char *name, const char *data)
{
    if(strcmp(account, my_account) == 0)//发送方
    {
        printf("收到信息：%s\n", data);
    }
    else//接受方
    {
        printf("收到%s %s的私聊信息：%s\n", account, name, data);
    }
}

void Add_user(void)
{
    printf("请输入要添加的好友账号：");
    scanf("%s",sen_msg.other);
    sen_msg.msgtype = MSG_INU;
    strcpy(sen_msg.account, my_account);
    strcpy(sen_msg.selfname, my_name);
    send_msg(sen_msg);
    printf("任意键返回\n");
    getchar();
    getchar();
}
void rec_add_user(const char *data)
{
    printf("收到内容：%s\n", data);
}
void Delete_user(void)
{
    printf("请输入要删除的好友账号：");
    scanf("%s",sen_msg.other);
    sen_msg.msgtype = MSG_DEU;
    strcpy(sen_msg.account, my_account);
    strcpy(sen_msg.selfname, my_name);
    send_msg(sen_msg);
    printf("任意键返回\n");
    getchar();
    getchar();
}
void rec_delete_user(const char *data)
{
    printf("收到内容：%s\n", data);
}
void Find_user_list(void)
{
    sen_msg.msgtype = MSG_FIU;
    strcpy(sen_msg.account, my_account);
    send_msg(sen_msg);
    printf("任意键返回\n");
    getchar();
    getchar();
}
void rec_find_user_list(const char *account, const char *name, const char *data)
{
    if(strcmp(data, "查询成功") == 0 || strcmp(data, "查询失败") == 0) // 结束语
    {
       printf("收到内容：%s\n", data);
    }
    else
    {
        printf("第%s个好友----账号: %s ---- 昵称: %s\n", data, account, name);
    }
    
}

void Create_qun(void)
{
    printf("请输入群名称：");
    scanf("%s",sen_msg.msgdata);
    sen_msg.msgtype = MSG_CRQ;
    strcpy(sen_msg.account, my_account);
    send_msg(sen_msg);
    printf("任意键返回\n");
    getchar();
    getchar();
}
void rec_create_qun(const char *data)
{
    printf("收到内容：%s\n", data);
}
void Delete_qun(void)
{
    printf("请输入删除群的ID：");
    scanf("%s",sen_msg.msgdata);
    sen_msg.msgtype = MSG_DEQ;
    strcpy(sen_msg.account, my_account);
    send_msg(sen_msg);
    printf("任意键返回\n");
    getchar();
    getchar();
}
void rec_delete_qun(const char *data)
{
    printf("收到内容：%s\n", data);
}    
void Find_qun_list(void)
{
    sen_msg.msgtype = MSG_FIQ;
    strcpy(sen_msg.account, my_account);
    send_msg(sen_msg);
    printf("任意键返回\n");
    getchar();
    getchar();
}
void rec_find_qun_list(const char *account, const char *name, const char *data)
{
    if(strcmp(data, "查询成功") == 0 || strcmp(data, "查询失败") == 0) // 结束语
    {
       printf("收到内容：%s\n", data);
    }
    else
    {
        printf("群ID：%s 群名:%s %s\n", account, name, data);
    }
}

void Add_qun(void)
{
    printf("请输入要加入群的ID：");
    scanf("%s",sen_msg.msgdata);
    sen_msg.msgtype = MSG_INQ;
    strcpy(sen_msg.account, my_account);
    send_msg(sen_msg);
    printf("任意键返回\n");
    getchar();
    getchar();
}
void rec_add_qun(const char *data)
{
    printf("收到内容：%s\n", data);
}
void Exit_qun(void)
{
    printf("请输入要退出群的ID：");
    scanf("%s",sen_msg.msgdata);
    sen_msg.msgtype = MSG_QEQ;
    strcpy(sen_msg.account, my_account);
    send_msg(sen_msg);
    printf("任意键返回\n");
    getchar();
    getchar();
}
void rec_exit_qun(const char *data)
{
    printf("收到内容：%s\n", data);
}


void Send_qun(void)
{
    printf("请输入发送的群的ID：");
    scanf("%s",sen_msg.password);
    printf("请输入发送信息：");
    scanf("%s",sen_msg.msgdata);
    sen_msg.msgtype = MSG_ALL;
    strcpy(sen_msg.account, my_account);
    strcpy(sen_msg.selfname, my_name);
    send_msg(sen_msg);
    printf("任意键返回\n");
    getchar();
    getchar();
}
void rec_send_qun(const char *account, const char *name, const char *data, const char *qun)
{
    printf("接收到群 %s %s %s: %s ",qun, account, name, data);
}
void Set_permission(void)
{
    printf("请输入发送的群的ID：");
    scanf("%s",sen_msg.password);
    printf("请输入要更改权限用户的帐号：");
    scanf("%s",sen_msg.other);
    printf("请输入要更改的权限：");
    scanf("%s",sen_msg.msgdata);

    sen_msg.msgtype = MSG_SQM;
    strcpy(sen_msg.account, my_account);
  //  strcpy(sen_msg.selfname, my_name);
    send_msg(sen_msg);
    printf("任意键返回\n");
    getchar();
    getchar();
}
void rec_set_permission(const char *data)
{
    printf("返回信息：%s",data);
}
void Set_jin_qun(void)
{
    printf("请输入操作的群的ID：");
    scanf("%s",sen_msg.password);
    printf("请输入要禁言成员的帐号：");
    scanf("%s",sen_msg.other);
    sen_msg.msgtype = MSG_JIN;
    strcpy(sen_msg.account, my_account);
    send_msg(sen_msg);
    printf("任意键返回\n");
    getchar();
    getchar();
}
void Set_jie_qun(void)
{
    printf("请输入操作的群的ID：");
    scanf("%s",sen_msg.password);
    printf("请输入要解禁成员的帐号：");
    scanf("%s",sen_msg.other);
    sen_msg.msgtype = MSG_JIE;
    strcpy(sen_msg.account, my_account);
    send_msg(sen_msg);
    printf("任意键返回\n");
    getchar();
    getchar();
}
void delete_qun_user(void)
{
    printf("请输入操作的群的ID：");
    scanf("%s",sen_msg.password);
    printf("请输入要删除成员的帐号：");
    scanf("%s",sen_msg.other);
    sen_msg.msgtype = MSG_TQU;
    strcpy(sen_msg.account, my_account);
    send_msg(sen_msg);
    printf("任意键返回\n");
    getchar();
    getchar();
}
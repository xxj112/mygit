#include "share.h"
#define BACKLOG 1024      //监听最大数量
// msg rec_msg = {0}; // 读取的信息
// msg sen_msg = {0}; // 发送的信息
int sktall[BACKLOG];    //所有的套接字
int sktcnt;             //记录当前的套接字个数
int epfd;
int bk[BACKLOG + 10] = {0}; //标记正在执行线程的套接字 1 表示执行  

void *my_func(void *arg) //现在是接受到并且返回
{
    int fd = (int) arg;
    msg rec_msg = {0}; // 读取的信息
    int ret = read(fd, &rec_msg, sizeof(rec_msg));
    if(ret == 0)
    {
        printf("客户端%d下线了\n",fd);
        //将客户端从套接字删除
        epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
        for(int i = 1; i < sktcnt; i++)
        {
            if(sktall[i] == fd)
            {
                sktall[i] = sktall[sktcnt - 1];
                sktcnt--;
                break;
            }
        }
        bk[fd] = 0; //恢复标记
        return NULL;
    }
   
    pr_msg(rec_msg);
    hand_msg(fd,rec_msg);
    //pr_msg(rec_msg);
   // strcpy(rec_msg.msgdata,"ww");
  //  send_msg(rec_msg,fd);
    bk[fd] = 0; //恢复标记
}
int main(void)
{
    // 连接数据库
    mysql_init_xxj();
    // 1、创建服务器套接字
    int skt = socket(AF_INET, SOCK_STREAM, 0);
    // 2、绑定自身IP
    // 定义IP结构体 填写服务器的信息
    struct sockaddr_in myaddr = {0}, client_addr = {0};
    myaddr.sin_addr.s_addr = inet_addr(SERVER_IP);
    myaddr.sin_family = AF_INET;
    myaddr.sin_port = htons(SERVER_RORT);
    int opt = 1;
    setsockopt(skt, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    //setsockopt(skt, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));
    
    int ret = bind(skt, (struct sockaddr *)&myaddr, sizeof(myaddr));
    if(ret < 0)
    {
        printf("服务器绑定IP失败\n");
        return 0;
    }    printf("服务器绑定IP成功\n");

    listen(skt, BACKLOG);
    struct sockaddr cliaddr = {0};
    socklen_t len = sizeof(cliaddr);
    int clifd = 0;
    //1、创建句柄
    epfd = epoll_create(1);
    //2、把服务器套接字加入到句柄
    struct epoll_event myepoll = {0};   
    myepoll.data.fd = skt;
    myepoll.events = EPOLLIN; 
    epoll_ctl(epfd, EPOLL_CTL_ADD, skt, &myepoll);
    struct epoll_event events[BACKLOG] = {0};
    sktcnt++;
    while(1)
    {
        //3、监听套接字
        ret = epoll_wait(epfd, events, sktcnt, -1);
        if(ret <= 0)
        {
            printf("监听失败\n");
            continue;
        }
        else 
        {
            for(int i = 0; i < ret; i++)
            {
                if(events[i].data.fd == skt) 
                {
                    //服务器异动
                    clifd = accept(skt, (struct sockaddr *)&client_addr, &len); //等待客户端连接
                    if(clifd <= 0)
                    {
                        printf("客户端上线失败\n");
                        continue;
                    }
                    printf("客户端%d上线\n",clifd);
                    //客户端上线，要把套接字加入到epoll的句柄中
                    myepoll.data.fd = clifd;
                    myepoll.events = EPOLLIN; 
                    epoll_ctl(epfd, EPOLL_CTL_ADD, clifd, &myepoll);
                    sktall[sktcnt++] = clifd; 
                
                }
                else 
                {
                    //客户端异动
                    if(bk[events[i].data.fd] == 1) continue;
                    bk[events[i].data.fd] = 1;
                    pthread_t pd = 0;
                    pthread_create(&pd, NULL, my_func, (void *)events[i].data.fd);
                    printf("%ld\n",pd);
                    pthread_detach(pd);  // 🔥 自动释放资源，避免僵尸线程
                }
            }    
        } 
    }
  //  mysql_close_xxj();
    return 0;
}

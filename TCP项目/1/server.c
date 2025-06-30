#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h> 
#include <netinet/in.h>      
#include <pthread.h>
#include <sys/epoll.h>

#define SERVER_IP "192.168.138.60"
// 端口号用短整形 2字节  0~65535 
// 不建议使用10000以下的有肯呢个别占用
#define SERVER_RORT 8848
#define BACKLOG 1024      //监听最大数量
int sktall[BACKLOG];    //所有的套接字
int sktcnt;             //记录当前的套接字个数
int epfd;
int bk[BACKLOG + 10] = {0}; //标记正在执行线程的套接字 1 表示执行   
void *my_func(void *arg)
{
    int fd = (int) arg;
    char buf[64] = {0};
    int ret = read(fd, buf, sizeof(buf));
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
        bk[fd] = 0;
        return NULL;
    }
    for(int i = 1; i < sktcnt; i++)
    {
        if(sktall[i] == fd) continue;
        write(sktall[i], buf, sizeof(buf));
    }
    bk[fd] = 0;
}
int main(void)
{
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
      //  printf("0\n");
        ret = epoll_wait(epfd, events, sktcnt, -1);
      //  printf("1\n");
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

    return 0;
}

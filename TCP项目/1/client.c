#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h> 
#include <netinet/in.h>      
#include <pthread.h>
#define SERVER_IP "192.168.138.60"
// 端口号用短整形 2字节  0~65535 
// 不建议使用10000以下的有肯呢个别占用
#define SERVER_RORT 8848
int skt;
void *myfunc(void *arg)
{
    char buf[64] = {0};
    while(1)
    {
        getchar();   
        printf("请输入：");
        scanf("%s",buf);
        write(skt, buf, sizeof(buf));
        getchar();
      //  usleep(100000);
    }


}
int main(void)
{
    // 1、创建服务器套接字
    skt = socket(AF_INET, SOCK_STREAM, 0);
    //
    struct sockaddr_in myaddr = {0};
    myaddr.sin_addr.s_addr = inet_addr(SERVER_IP);//IP
    myaddr.sin_family = AF_INET;//IP类型
    myaddr.sin_port = htons(SERVER_RORT);//端口号
    int ret = connect(skt, (struct sockaddr *)&myaddr, sizeof(myaddr));
    if(ret < 0)
    {
        printf("连接服务器失败\n");
        return 0;
    }
    printf("连接服务器成功\n");
    pthread_t pd = 0;
    pthread_create(&pd, NULL, myfunc, NULL);
    char rbuf[64] = {0};
    while(1)
    {
        if(read(skt, rbuf, sizeof(rbuf)) == 0)
        {
            printf("服务器结束\n");
            break;
        }
        printf("收到内容：%s\n",rbuf);
    }

    return 0;
}
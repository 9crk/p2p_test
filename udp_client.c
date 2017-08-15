#include<stdio.h>
#include <unistd.h>
#include<stdlib.h>
#include <string.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


int main(int argc,char* argv[])
{
    int len,n,sockfd = 0;    
    char buff[100];
    char target_ip[20];
    int target_port;
    struct sockaddr_in addr;
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(-1 == sockfd)
    {
        perror("socket");
        return -1;
    }
    printf("sockfd = %d\n", sockfd);
    len = sizeof(addr);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(argv[2]));
    addr.sin_addr.s_addr = inet_addr(argv[1]);
    if (addr.sin_addr.s_addr == INADDR_NONE)
    {
        printf("Incorrect ip address!");
        close(sockfd);
        exit(1);
    }
    //1.向服务器发起连接，观察服务器端本机的公网IP和port
    n = sendto(sockfd, buff, strlen(buff), 0, (struct sockaddr *)&addr, sizeof(addr));
    if (n < 0)
    {
        perror("sendto");
        close(sockfd);
        return 0;
    }
    n = recvfrom(sockfd, buff, 100, 0, (struct sockaddr *)&addr, &len);
    if (n>0)
    {
        buff[n] = 0;
        printf("received:");
        puts(buff);
    }
    memset(target_ip,0,20);
    printf("input the IP:\n");
    scanf("%s",target_ip);   
    printf("input the port:\n");
    scanf("%d",&target_port);
    //2.输入目标IP和端口，去连接另一个客户端的NAT
    addr.sin_family = AF_INET;
    addr.sin_port = htons(target_port);
    addr.sin_addr.s_addr = inet_addr(target_ip);
    printf("target:%s:%d\n",target_ip,target_port);
    while(1){
	    scanf("%s",buff);
        n = sendto(sockfd, buff, strlen(buff), 0, (struct sockaddr *)&addr, sizeof(addr));
        printf("sent %d\n",n);
        n = recvfrom(sockfd, buff, 100, 0, (struct sockaddr *)&addr, &len);
        printf("recv %d\n",n);
    }
    close(sockfd);
}

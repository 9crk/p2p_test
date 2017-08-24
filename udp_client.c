#include<stdio.h>
#include <unistd.h>
#include<stdlib.h>
#include <string.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>


int reg(char*ip,int port,int uuid,char*targetIp,int*targetPort)
{
    int i,socket_fd,ret,udp_socket,len;
    char buff[100];
    struct sockaddr_in addr;
    signal(SIGPIPE,SIG_IGN);
    socket_fd = socket(AF_INET,SOCK_STREAM,0);
    if(socket_fd == -1){printf("socket err\n");exit(0);}
    bzero(&addr,sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(port);
    addr.sin_addr.s_addr    = inet_addr(ip);
    
    if(connect(socket_fd,(struct sockaddr *)(&addr),sizeof(struct sockaddr))==-1){
        printf("conn err\n");close(socket_fd);return -1;
    }
    sprintf(buff,"<handle>%d</handle>",uuid);
    ret = send(socket_fd,buff,strlen(buff),0);
    if(ret < 0){
            printf("send err\n");
            close(socket_fd);
            return -1;
    }
    sleep(1);
    udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if(-1 == udp_socket){
        perror("socket");
        return -1;
    }
    printf("udp_socket = %d\n", udp_socket);
    len = sizeof(addr);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip);
    if (addr.sin_addr.s_addr == INADDR_NONE){
        printf("Incorrect ip address!");
        close(udp_socket);
        exit(1);
    }
    //1.向服务器发起连接，观察服务器端本机的公网IP和port
    sprintf(buff,"<handle>%d</handle>",uuid);
    ret = sendto(udp_socket, buff, strlen(buff), 0, (struct sockaddr *)&addr, sizeof(addr));
    if (ret < 0){
        perror("sendto");
        close(udp_socket);
        return 0;
    }


    memset(buff,0,100);
    ret = recv(socket_fd,buff,100,0);
    if(ret < 0){
        printf("recv err\n");
        close(socket_fd);
        return -1;
    }
    sscanf(buff,"%[^:]:%d",targetIp,&(*targetPort));
    close(socket_fd);

    //发一点东西,建立连接
    addr.sin_family = AF_INET;
    addr.sin_port = htons(*targetPort);
    addr.sin_addr.s_addr = inet_addr(targetIp);
    len = sizeof(addr);
    for(i=0;i<10;i++){
        sendto(udp_socket, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26, 0, (struct sockaddr *)&addr, sizeof(addr));
    }    
    return udp_socket;
}
void recv_thread(void*arg)
{
    char buff[1000];
    struct sockaddr_in addr;
    int len,n;
    int sockfd = *(int*)arg;
    len = sizeof(addr);
    while(1){
        memset(buff,0,1000);
        n = recvfrom(sockfd, buff, 100, 0, (struct sockaddr *)&addr, &len);
        printf("<<<<<<<<<<<<<:%s\n",buff);
    }
}
int main(int argc,char* argv[])
{
    int len,n,sockfd = 0;    
    char buff[100];

    char target_ip[20];
    int target_port;
    struct sockaddr_in addr;
    
    memset(target_ip,0,20);
    sockfd = reg(argv[1],atoi(argv[2]),atoi(argv[3]),target_ip,&target_port);
    
    //2.输入目标IP和端口，去连接另一个客户端的NAT
    addr.sin_family = AF_INET;
    addr.sin_port = htons(target_port);
    addr.sin_addr.s_addr = inet_addr(target_ip);
    len = sizeof(addr);
    printf("target:%s:%d handle=%d\n",target_ip,target_port,sockfd);

    pthread_t pid;
    pthread_create(&pid,NULL,recv_thread,&sockfd);
    while(1){
        scanf("%s",buff);
        n = sendto(sockfd, buff, strlen(buff), 0, (struct sockaddr *)&addr, sizeof(addr));
        printf(">>>>>>>>>>>>>:%s\n",buff);
    }
    close(sockfd);
}

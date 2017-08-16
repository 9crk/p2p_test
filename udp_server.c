#include<stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

//这个线程的目的：保存N个ID-IP:port对应表-----------------------------------------------
#define UDP_PORT    3333
#define MAX_UDP_LIST    1000
typedef struct id_ip_info{
    int id;
    char ip[20];
    int port;
}id_ip_info;
id_ip_info udplist[MAX_UDP_LIST];
void udp_server()
{
    int i,n,ihandle,len,sockfd;
    char buff[100];
    struct sockaddr_in addr,clientAddr;
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(-1 == sockfd){
        perror("socket");
        return;
    }
    len = sizeof(addr);
    memset(udplist,0,sizeof(id_ip_info)*MAX_UDP_LIST);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(UDP_PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);    
    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0){
        perror("bind");
        exit(1);
    }
    while(1){
        ihandle = 0;
        n = recvfrom(sockfd, buff, 100, 0, (struct sockaddr *)&clientAddr, &len);
        sscanf(buff,"<handle>%d</handle>",&ihandle);
        if(ihandle==0)continue;
        for(i=0;i<MAX_UDP_LIST;i++)if(udplist[i].id == 0){
            udplist[i].id = ihandle;
            sprintf(udplist[i].ip,"%s",inet_ntoa(clientAddr.sin_addr));
            udplist[i].port = ntohs(clientAddr.sin_port);
            break;
        }
        if(i==MAX_UDP_LIST/2){
            memcpy(&udplist,&udplist[MAX_UDP_LIST/2],sizeof(id_ip_info)*MAX_UDP_LIST/2);
            memset(&udplist[MAX_UDP_LIST/2],0,sizeof(id_ip_info)*MAX_UDP_LIST/2);
        }
        for(i=0;i<MAX_UDP_LIST;i++){
            if(udplist[i].id != 0){
                printf("<%d,%s:%d>\n",udplist[i].id,udplist[i].ip,udplist[i].port);
            }
        }
    }
    close(sockfd);
}
//这个线程用来做信令的
#define TCP_PORT    3333
void tcp_server()
{
    int ret,flag = 1,len,recvSize;
    int socket_fd,conn_fd;
    char buff[100];
    fd_set rSet;
    struct timeval tv;
    struct sockaddr_in  severAddr,clientAddr;
    signal(SIGPIPE,SIG_IGN);
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(socket_fd < 0){
        printf("socket err!\n");
        return;
    }
    severAddr.sin_family = AF_INET;
    severAddr.sin_port = htons(TCP_PORT);
    severAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    if(-1 == setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int))){
        printf("REUSE IO ERROR\n");
        exit(0);
    }
    ret = bind(socket_fd, (struct sockaddr*)&severAddr, sizeof(struct sockaddr));
    if(ret < 0){
        printf("bind sock err!\n");
        close(socket_fd);
        return;
    }
    ret = listen(socket_fd, 20);
    if(ret < 0){
        printf("listen sock err!\n");
        close(socket_fd);
        return;
    }
    len = sizeof(clientAddr);    
    while(1){
        //accept返回客户端套接字描述符    
        conn_fd = accept(socket_fd,(struct sockaddr *)&clientAddr,&len);  //注，此处为了获取返回值使用 指针做参数    
        
    }
}

int main(int argc,char* argv[])
{
    pthread_t pid1,pid2;
    pthread_create(&pid1,NULL,udp_server,NULL);
    pthread_create(&pid2,NULL,tcp_server,NULL);
    while(1){
        sleep(1);
    }
}

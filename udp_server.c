#include<stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>
/*
TCP 
1.accept connection + remember time + if timeout&no_data tick out
2.recv data ->get ID
3.recv UDP ->get-IP_Port ->remember
                                               ->send to each other ->close both
*/



//这个线程的目的：保存N个ID-IP:port对应表-----------------------------------------------
#define UDP_PORT    3333
#define MAX_CONN    1000
typedef struct id_ip_info{
    int id;
    int socket;
    char ip1[20];
    int port1;
    int skt1;
    char ip2[20];
    int port2;
    int skt2;
}id_ip_info;
id_ip_info clientlist[MAX_CONN];
void debug()
{
    int i;
    for(i=0;i<MAX_CONN;i++){
        if(clientlist[i].id != 0){
            printf("--------\n%d-%s:%d-%s:%d\n",clientlist[i].id,clientlist[i].ip1,clientlist[i].port1,clientlist[i].ip2,clientlist[i].port2);
        }
    }
}
static int updateConn(char* ip,int port,int handle)
{
    int i;
    char buff[100];
    for(i=0;i<MAX_CONN;i++){
        if(clientlist[i].id == handle)break;
    }
    if(i != MAX_CONN){//already exist
        if(strcmp(clientlist[i].ip1,"ip") == 0){
            clientlist[i].port1 = port;
            debug();
        }else{
            clientlist[i].port2 = port;
            sprintf(buff,"<ip>%s</ip><port>%d</port>",clientlist[i].ip2,clientlist[i].port2);
            send(clientlist[i].skt1,buff,strlen(buff),0);
            sprintf(buff,"<ip>%s</ip><port>%d</port>",clientlist[i].ip1,clientlist[i].port1);
            send(clientlist[i].skt2,buff,strlen(buff),0);
            close(clientlist[i].skt1);
            close(clientlist[i].skt2);
            debug();
            memset(&clientlist[i],0,sizeof(id_ip_info));
            printf("close one\n");
        }
    }
    return -1;
}
static int createOrUpdateConn(char* ip,int handle,int socket)
{
    int i;
    for(i=0;i<MAX_CONN;i++){
        if(clientlist[i].id == handle)break;
    }
    if(i!= MAX_CONN){
        strcpy(clientlist[i].ip2,ip);
        clientlist[i].skt2 = socket;
        debug();
        return 0;
    }
    for(i=0;i<MAX_CONN;i++){
        if(clientlist[i].id == 0)break;
    }
    if(i == MAX_CONN)return -1;
    clientlist[i].id = handle;
    strcpy(clientlist[i].ip1,ip);
    clientlist[i].skt1 = socket;
    debug();
    return 0;
}


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
    memset(clientlist,0,sizeof(id_ip_info)*MAX_CONN);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(UDP_PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0){
        perror("bind");
        exit(1);
    }
    printf("udp server ok\n");
    while(1){
        ihandle = 0;
        n = recvfrom(sockfd, buff, 100, 0, (struct sockaddr *)&clientAddr, &len);
        sscanf(buff,"<handle>%d</handle>",&ihandle);
        printf("port=%d recv:%s\n",ntohs(clientAddr.sin_port),buff);
        if(ihandle==0)continue;
        updateConn(inet_ntoa(clientAddr.sin_addr),ntohs(clientAddr.sin_port),ihandle);
    }
    close(sockfd);
}
//这个线程用来做信令的
#define TCP_PORT    3333
typedef struct conn_ip_info{
int conn_fd;
char ip[20];
}conn_ip_info;
conn_ip_info waitList[MAX_CONN];

int create_skt()
{
    int ret,i;
    int flag = 1;
    int socket_fd;
    struct sockaddr_in  severAddr;
    signal(SIGPIPE,SIG_IGN);
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(socket_fd < 0){
        printf("socket err!\n");
        return -1;
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
        return -1;
    }
    ret = listen(socket_fd, 10);
    if(ret < 0){
        printf("listen sock err!\n");
        close(socket_fd);
        return -1;
    }
   memset(&waitList,0,sizeof(conn_ip_info)*MAX_CONN);
   return socket_fd;
}

void accept_server(void*skt)
{
    int conn_fd,i,socket_fd,len;
    struct sockaddr_in  clientAddr;
    len = sizeof(clientAddr);
    socket_fd = *(int*)skt; 
    while(1){
        conn_fd = accept(socket_fd,(struct sockaddr *)&clientAddr,&len);  //注，此处为了获取返回值使用 指针做参数 
        if(conn_fd > 0){
            for(i=0;i<MAX_CONN;i++)if(waitList[i].conn_fd == 0){
                waitList[i].conn_fd = conn_fd;
                strcpy(waitList[i].ip,inet_ntoa(clientAddr.sin_addr));
                break;
            }
        }
    }
}
void tcp_server()
{
    int maxFd,ret,cnt,i,ihandle;
    struct timeval tv;
    fd_set set;
    int recvSize;
    char buff[100];
    while(1){
        maxFd = -1;
        cnt = 0;
        tv.tv_sec = 0;
        tv.tv_usec = 100*1000;
        FD_ZERO(&set);
        for(i=0;i<MAX_CONN;i++){
            if(waitList[i].conn_fd != 0){
                cnt++;
                FD_SET(waitList[i].conn_fd, &set);
                if(waitList[i].conn_fd > maxFd)maxFd = waitList[i].conn_fd;
            }
        }
        //printf("------------total:%d\n",cnt);
        ret = select(maxFd+1,&set,NULL,NULL,&tv);
        if(ret > 0){
            for(i=0;i<MAX_CONN;i++){
                if(waitList[i].conn_fd != 0){
                    if(FD_ISSET(waitList[i].conn_fd,&set)){
                        recvSize = recv(waitList[i].conn_fd, buff, 100, 0);
                        printf("recv = %d\n",recvSize);
                        if(recvSize>0){
                            sscanf(buff,"<handle>%d</handle>",&ihandle);
                            createOrUpdateConn(waitList[i].ip,ihandle,waitList[i].conn_fd);
                        }
                    }
                }
            }
        }
    }
}

int main(int argc,char* argv[])
{
    pthread_t pid1,pid2,pid3;
    int skt = create_skt();
    pthread_create(&pid1,NULL,udp_server,NULL);
    pthread_create(&pid2,NULL,accept_server,&skt);
    pthread_create(&pid3,NULL,tcp_server,NULL);
    
    while(1){
        sleep(1);
    }
}

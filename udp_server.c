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
    struct sockaddr_in clientAddr;
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(-1 == sockfd)
    {
        perror("socket");
        return -1;
    }
    printf("sockfd = %d\n", sockfd);
    len = sizeof(addr);

    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(argv[1]));
    addr.sin_addr.s_addr = htonl(INADDR_ANY);    
    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("bind");
        exit(1);
    }
    while(1){
        n = recvfrom(sockfd, buff, 100, 0, (struct sockaddr *)&clientAddr, &len);
        printf("recv %d bytes from %s:%u\n",n,inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
        sprintf(buff,"this is server\n");
        n = sendto(sockfd, buff, strlen(buff), 0, (struct sockaddr *)&clientAddr, sizeof(clientAddr));
        printf("sent %d\n",n);
    }
    close(sockfd);
}

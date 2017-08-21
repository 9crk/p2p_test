#include<stdio.h>
int main()
{
	char ss[] = "192.23:344";
	char sss[20];
	int ssss;
	sscanf(ss,"%[^:]:%d",sss,&ssss);
	printf("sss=%s ssss=%d\n",sss,ssss);
}

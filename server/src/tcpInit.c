#include "head.h"
// socket建立tcp连接
int tcpInit(const char* ip,const char* port, int* psockfd){
    // 建立tcp通信：socket bind listen
    *psockfd = socket(AF_INET,SOCK_STREAM,0);//psockfd传入传出参数
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;  //ipv4
    addr.sin_port = htons(atoi(port));
    addr.sin_addr.s_addr = inet_addr(ip);
    int reuse = 1;
    int ret = setsockopt(*psockfd,SOL_SOCKET,SO_REUSEADDR,&reuse,sizeof(reuse));
    ERROR_CHECK(ret,-1,"tcpInit setsockopt");
    ret = bind(*psockfd,(struct sockaddr *)&addr,sizeof(addr));
    ERROR_CHECK(ret,-1,"tcpInit bind");
    listen(*psockfd,10);
    return 0;
}

#include "head.h"
int main(int argc,char* argv[]){ // conf的路径 
    ARGS_CHECK(argc,2);  
    pid_t pid;
    char buf[1024] = {0};
    char ip[20] = "";  // xxx.xxx.xxx.xxx
    char port[10] = "";

    // 从配置文件中拿出 ip 和 port
    char * line, *ptr;
    int fd = open(argv[1],O_RDONLY);
    ERROR_CHECK(fd,-1,"open");
    readline(fd,buf,sizeof(buf));  // 读取文件的一行
    line = buf;
    ptr = strchr(line, '=');
    strcpy(ip,ptr+1);   // 获取ip

    bzero(buf,sizeof(buf));
    bzero(line,sizeof(line));

    readline(fd,buf,sizeof(buf)); // 读取文件的第二行
    line = buf;
    ptr = strchr(line, '=');  
    strcpy(port,ptr+1); // 获取port
    // printf("ip = %s,port = %s\n",ip,port);

    // socket 建立tcp连接
    socket_t fd_server;
    socket_t fd_client;
    tcpInit(ip, port,&fd_server);

    //进程池中进程的数量固定为3(即 workerNum == 3)是最合适的(后期还会延伸可以动态修改进程池中worker进程的数量)
    int workerNum = 3;
    // 创建一个workerdata的数组，用来保存所有子进程的信息(确定是子进程?)
    workerdata_t *workerdataArr = (workerdata_t*)calloc(3,sizeof(workerdata_t));
    //创建workerNum个子进程
    makeWorker(workerdataArr,3);  // workerNum == 3

    // 接收客户端的sockAddr
    //建立监听集合
    int epfd = epollCtor();
    //sockfd 管理新客户端的连接
    epollAdd(epfd,fd_server);
    //管理每个父子进程的socket
    for(int i = 0; i<workerNum;i++){
        epollAdd(epfd,workerdataArr[i].pipefd);
    }
    int listenSize = workerNum + 1;
    //就绪集合
    struct epoll_event* readyset = (struct epoll_event*)calloc(listenSize,sizeof(struct epoll_event));
    while(1){
        int readynum = epoll_wait(epfd,readyset,listenSize,-1);
        for(int idx = 0; idx < readynum;idx++){
            if(readyset[idx].data.fd == fd_server){
                //有客户端connect
                printf("1 client connect, accept is ready!\n");
                fd_client = accept(fd_server,NULL,NULL);  // 父进程的netfd这里是fd_client要交给子进程
                // 选择一个空闲的子进程
                for(int i = 0; i < workerNum; i++){
                    if(workerdataArr[i].status == FREE){
                        // pid 和 ppid的打印是方便调试用的
                        printf("No %d worker gets his job, pid = %d\n",
                                i,workerdataArr[i].pid);  //workerdataArr数组里面确实存的是子进程的信息
                        sendfd(workerdataArr[i].pipefd,fd_client); //sendfd函数用于池子里面父子通信
                        workerdataArr[i].status =BUSY;     
                        break;             
                    }
                }
                close(fd_client); //关闭父进程，不影响子进程
            }
            else{
                //有子进程完成任务
                printf("1 worker finished his job\n");
                int i;
                for(i = 0; i< workerNum;i++){
                    if(workerdataArr[i].pipefd == readyset[idx].data.fd){
                        pid_t pid;
                        read(workerdataArr[i].pipefd,&pid,sizeof(pid));
                        printf("No %d worker finished his job, pid =%d\n",i,pid);
                        workerdataArr[i].status = FREE;
                        break;
                    }
                }
            }
        }
    }    

}
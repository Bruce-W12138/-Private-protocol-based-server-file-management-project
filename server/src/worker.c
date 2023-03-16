#include "head.h"
int makeWorker(workerdata_t *workerdataArr,int workerNum){
    //创建子进程，每一次fork都会创建一个子进程
    //创建workerNum个子进程，将信息存到workerdataArr
    pid_t pid;
    int pipefds[2]; 
    for(int i = 0;i < workerNum;i++){
        //先socketpair 再fork, socketpair是一种特别的管道，它是架设在父子进程间
        // domain参数写AF_LOCAL而不是AF_INET;因为是池内本地通信
        socketpair(AF_LOCAL,SOCK_STREAM,0,pipefds); 
        // pipefds在上面函数调用完成后，pipefds[0]和pipefds[1]就变成了管道的两端但不区分读写
        pid = fork();
        if(pid == 0){
            //worker进程的内容(worker奴隶进程是子进程)，子进程是真正工作的进程 
            close(pipefds[0]);
            eventLoop(pipefds[1]);
        }
        else{
            //父进程，父进程的工作是需要保存子进程的状态
            close(pipefds[1]);
            workerdataArr[i].pid = pid; 
            workerdataArr[i].status = FREE;
            workerdataArr[i].pipefd = pipefds[0];
            // printf("worker %d, pid = %d, pipefd = %d\n", i, pid,pipefds[0]);  
        }     
    }
    return 0;
}

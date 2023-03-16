#ifndef __FTP_H__
#define __FTP_H__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/wait.h>
#define BUF_SIZE 8*1024
#define ERR_EXIT(msg) do { \
						perror(msg);\
						exit(1);\
						}while(0)
#define ARGS_CHECK(argc, num)       \
    {                               \
        if (argc != num)            \
        {                           \
            printf("error args\n"); \
            return -1;              \
        }                           \
    }
#define ERROR_CHECK(ret, retval, func_name) \
    {                                       \
        if (ret == retval)                  \
        {                                   \
            perror(func_name);              \
            return -1;                      \
        }                                   \
    }
#define THREAD_ERR_CHECK(ret, func_name)                                \
    {                                                                   \
        if (ret != 0)                                                   \
        {                                                               \
            printf("%s failed,%d %s\n", func_name, ret, strerror(ret)); \
            return -1;                                                  \
        }                                                               \
    }
#define THREAD_ERROR_CHECK(ret, info)                        \
    {                                                        \
        if (ret != 0)                                        \
        {                                                    \
            fprintf(stderr, "%s:%s\n", info, strerror(ret)); \
        }                                                    \
    }
enum{
    FREE,
    BUSY
};
typedef struct workerdata_s{
    //保存某个worker进程的信息
    pid_t pid;
    int status; //记录某个进程是空闲还是忙碌
    int pipefd; //父进程接收子进程消息的socket的fd
}workerdata_t;
typedef int socket_t ;
ssize_t readline(int fd, char *buf, ssize_t maxlen);
int makeWorker(workerdata_t *workerdataArr,int workerNum);
int eventLoop(socket_t pipefd);
int tcpInit(const char* ip,const char* port, int* psockfd);
int epollCtor();
int epollAdd(int epfd, int fd);
int epollDel(int epfd, int fd);
int sendfd(int pipefd, int fdtosend);
int recvfd(int pipefd, int *pfdtorecv);
void trim_space(char* src);
int recvn(socket_t fd_recv, char* recv_buf, int len );
int sendn(socket_t fd_send, char* send_buf, int len );
int readn(int fd_read, char* read_buf, int len );
int writen(int fd_write, char* write_buf, int len );
int upload(socket_t fd_up, char* file_name);
int download(socket_t fd_down, char* file_name);
#endif



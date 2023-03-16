#include<func.h>
int epollCtor(){
    int epfd = epoll_create(10);
    ERROR_CHECK(epfd,-1,"epoll_create");
    return epfd;
}
int epollAdd(int epfd, int fd){
    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = fd;
    int ret = epoll_ctl(epfd,EPOLL_CTL_ADD,fd,&event);
    ERROR_CHECK(ret,-1,"epoll_ctl add");
    return 0;
}
int epollDel(int epfd, int fd){
    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = fd;
    int ret = epoll_ctl(epfd,EPOLL_CTL_DEL,fd,&event);
    ERROR_CHECK(ret,-1,"epoll_ctl del");
    return 0;
}
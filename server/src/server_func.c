#include "head.h"
//读取fd的一行内容
ssize_t readline(int fd, char *buf, ssize_t maxlen)
{
	ssize_t count = 0;
	memset(buf, '\0', sizeof(buf));
	
	char *ptr = buf;
	char tmp = 0;
    while(1){
        read(fd,&tmp,sizeof(char));
        if(tmp == '\n' || tmp == '\0'){
            break;
        }
        (*ptr) = tmp;
		ptr ++;
		count ++;
		if(count >= maxlen){
			printf("Out of buffer!\n");
			return -1;
        }
    }
    return count;
}
// 进程池内部的发送函数
int sendfd(int pipefd, int fdtosend){
    // pipefd 是socketpair创建出来的socket的一端
    // fdtosend 是想要传递的文件对象的文件描述符
    struct msghdr hdr;
    bzero(&hdr,sizeof(hdr)); //第一步 一定要初始化！！！！！
    // 消息正文部分 正文部分一定要写
    char buf1[] = "hello";
    char buf2[] = "world";
    struct iovec iov[2];
    iov[0].iov_base = buf1;
    iov[0].iov_len = 5;
    iov[1].iov_base = buf2;
    iov[1].iov_len = 5;
    hdr.msg_iov = iov;
    hdr.msg_iovlen = 2;
    // 控制信息部分 data部分要存什么内容？文件描述符 是一个整数
    struct cmsghdr *pcmsghdr;//变长结构体，只能在堆上申请空间
    // CMSG_LEN已知data的长度，获取整个结构体的长度
    pcmsghdr = (struct cmsghdr *)calloc(1,CMSG_LEN(sizeof(int)));
    pcmsghdr->cmsg_len = CMSG_LEN(sizeof(int));
    pcmsghdr->cmsg_level = SOL_SOCKET;
    pcmsghdr->cmsg_type = SCM_RIGHTS;//说明了传递一个文件对象
    //把文件描述符填入data区域
    // 先获取data区域的首地址，然后强转成int*，然后解引用并赋值
    *(int *)CMSG_DATA(pcmsghdr) = fdtosend;
    // 设置hdr的内容
    hdr.msg_control = pcmsghdr;
    hdr.msg_controllen = CMSG_LEN(sizeof(int));
    int ret = sendmsg(pipefd,&hdr,0);//把消息正文连同控制信息 通过 pipefd发送
    ERROR_CHECK(ret,-1,"sendmsg");
    return 0;
}
// 进程池内部的接收函数
int recvfd(int pipefd, int *pfdtorecv){
    struct msghdr hdr;
    bzero(&hdr,sizeof(hdr));//第一步 一定要初始化！！！！！
    // 收消息的流程和发消息是一样的，只是缺少了buf和data
    // 用来接收消息正文
    char buf1[6] = {0};
    char buf2[6] = {0};
    struct iovec iov[2];
    iov[0].iov_base = buf1;
    iov[0].iov_len = 5; //这个长度必须非0
    iov[1].iov_base = buf2;
    iov[1].iov_len = 5;
    hdr.msg_iov = iov;
    hdr.msg_iovlen = 2;
    // 控制信息部分 data部分要存什么内容？文件描述符 是一个整数
    struct cmsghdr *pcmsghdr;//变长结构体，只能在堆上申请空间
    // // CMSG_LEN已知data的长度，获取整个结构体的长度
    pcmsghdr = (struct cmsghdr *)calloc(1,CMSG_LEN(sizeof(int)));
    pcmsghdr->cmsg_len = CMSG_LEN(sizeof(int));
    pcmsghdr->cmsg_level = SOL_SOCKET;
    pcmsghdr->cmsg_type = SCM_RIGHTS;//说明了传递一个文件对象
    // // 设置hdr的内容
    hdr.msg_control = pcmsghdr;
    hdr.msg_controllen = CMSG_LEN(sizeof(int));
    int ret = recvmsg(pipefd,&hdr,0);
    ERROR_CHECK(ret,-1, "recvmsg");
    *pfdtorecv = *(int *)CMSG_DATA(pcmsghdr);
    return 0;
}
// 写入
int writen(int fd_write, char* write_buf, int len)
{
	int sum = 0 ;
	int nwrite ;
	while(sum < len)
	{
		nwrite = write(fd_write, write_buf + sum, len - sum);
		sum += nwrite ;
	}
	return sum ;

}
// 文件上传
int upload(socket_t fd_up, char* file_name)
{
	int fd_file = open(file_name, O_RDONLY);
	if(fd_file == -1)
	{
		return -1 ;
	}
	char *read_buf = (char*)malloc(8 * 1024);
	bzero(read_buf, 8 * 1024);
	int nread ;
	while(1)
	{
		nread = readn(fd_file, read_buf, 8192);
		if(nread < 8192)
		{
			send(fd_up, &nread, sizeof(int), 0);
			sendn(fd_up, read_buf, nread);
			break ;
		}else
		{
			
			send(fd_up, &nread, sizeof(int), 0);
			sendn(fd_up, read_buf, nread);
		}
	}
	int flag = 0 ;
	send(fd_up, &flag, sizeof(flag), 0);
	close(fd_file);
	return 0 ;
}
//接收文件
int download(socket_t fd_down, char* file_name)
{
	int fd_file = open(file_name, O_WRONLY|O_CREAT,0666 );
	if(fd_file == -1)
	{
		return -1 ;
	}
	char* write_buf = (char*)malloc(8192);
	bzero(write_buf, 8192);
	int nwrite ;
	while(1)
	{
		recv(fd_down, &nwrite, sizeof(int), 0);
		if(nwrite == 0)
		{
			break ;
		}
		recvn(fd_down, write_buf, nwrite);
		writen(fd_file, write_buf, nwrite);
	}
	close(fd_file);
	return 0 ;
}	
// 发送 send_buf的内容给对端
int sendn(socket_t fd_send, char* send_buf, int len)
{
	int sum = 0 ;
	int nsend ;
	while(sum < len)
	{
		nsend = send(fd_send, send_buf + sum, len - sum, 0);
		sum += nsend ;
	}
	return sum ;
}
// 接收对端的内容，用recv_buf存起来
int recvn(socket_t fd_recv,char* recv_buf, int len)
{
	int sum = 0 ;
	int nrecv ;
	while(sum < len)
	{
		nrecv = recv(fd_recv, &recv_buf[ sum], len - sum, 0);
		sum += nrecv ;
	}
	recv_buf[sum] = '\0';
	return sum ;
}
int readn(int fd_read, char* read_buf, int len)
{
	int sum = 0 ;
	int nread ;
	while(sum < len)
	{
		nread = read(fd_read, &read_buf[ sum], len - sum);
		if(nread == 0)
		{
			break ;
		}
		sum += nread ;
	}
	read_buf[sum] = '\0';
	return sum ;		
}
// 创造一个目录
int create_dir(char* dir_name, char* path){
	// 文件夹与path拼接在一起
	char* ptr = path;
	while(*ptr){
		ptr++;
	}
	(*ptr) = '/';
	strcat(path,dir_name);
	printf("%s\n",path);
	int ret = mkdir(path,S_IRUSR);
	if(ret == 0){
		return 0;
	}
	return 1;
}




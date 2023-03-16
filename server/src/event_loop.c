#include "head.h"
// worker进程工作完后，告诉父进程，然后恢复空闲，空闲函数
void workerFree(socket_t pipefd, socket_t fd_client){
    close(fd_client);
    //子进程告知父进程，子进程任务完成
    pid_t pid = getpid();
    write(pipefd,&pid,sizeof(pid));
}
int eventLoop(socket_t pipefd){
    // 子进程的工作是什么，封装到里面，类似handle.c
    // 与源代码不同的点在于，源代码直接传文件描述符，现在我们传的是管道，现在我们创建一个fd_client去接收它
    //接收任务
    socket_t fd_client;
    recvfd(pipefd,&fd_client);  //通过管道传给fd_client接收
    char assist_buf[BUF_SIZE] = {0};
    int cmd_len = 0 ;
	int recv_ret ;
    while(1){
        recv_ret = recv(fd_client, &cmd_len, sizeof(int),0);//收到客户端的火车头,客户端发送指令
        // printf("cmd_len = %d, recv_ret = %d\n",cmd_len,recv_ret);
        if(cmd_len == 0 || recv_ret == 0)
        {
            printf("client exit !\n");  
            workerFree(pipefd,fd_client); //结束该进程
            break;
        }
        bzero(assist_buf, sizeof(assist_buf));
        recvn(fd_client, assist_buf, cmd_len);//收火车车厢
        if(strcmp("quit",assist_buf) == 0){
            printf("client exit !\n");  
            workerFree(pipefd,fd_client);
            break;
        }

        /*下面就是处理一堆来自客户端的请求*/
		if(strncmp("cd", assist_buf, 2) == 0)
		{
			do_cd(assist_buf,fd_client);
		}
        else if(strcmp("ls", assist_buf) == 0)
		{
			do_ls(assist_buf,fd_client);
		}
        else if( strncmp("puts ", assist_buf, 5)== 0)
		{
			do_puts(assist_buf,fd_client);
		}
        else if( strncmp("gets", assist_buf, 4)== 0)
		{
			do_gets(assist_buf,fd_client);
		}
        else if( strncmp("remove ", assist_buf, 7)== 0)
		{
			do_remove(assist_buf,fd_client);
		}
        else if(strcmp("pwd", assist_buf) == 0) 
		{
			do_pwd(assist_buf,fd_client);
		}
        else if(strncmp("mkdir",assist_buf, 5) == 0)
        {
            do_mkdir(assist_buf,fd_client);
        }
        else if(strncmp("touch",assist_buf,5) == 0)
        {
            do_touch(assist_buf,fd_client);
        }
        else if(strncmp("rmdir",assist_buf,5) == 0)
        {
            do_rmdir(assist_buf,fd_client);
        }
        else 
		{
			do_error(assist_buf,fd_client);
			continue ;
		}




        // close(fd_client);
        // //子进程告知父进程，子进程任务完成
        // pid_t pid = getpid();
        // write(pipefd,&pid,sizeof(pid));
    }
}
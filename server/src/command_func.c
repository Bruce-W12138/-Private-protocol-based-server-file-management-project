#include "head.h"
char* file_type(mode_t md)
{
	if(S_ISREG(md))
	{
		return "-";	
	}else if(S_ISDIR(md))
	{
		return "d";
	}else if(S_ISFIFO(md))
	{
		return "p";
	}else 
	{
		return "o" ;
	}
}
//执行ls -l得到的数据传递给客户端
void do_ls(char* assist_buf, socket_t fd) {
	// 这个函数目前还不稳定，只要一用完客户端就会退出
	DIR* pdir = opendir("./");
	if(pdir == NULL){
		int flag = -1 ;
		send(fd, &flag, sizeof(int), 0);
	}
	else{
		struct dirent* mydir ;
		int len ;
		while( (mydir = readdir(pdir)) != NULL){//读取目录
			if(strncmp(mydir->d_name, ".", 1) == 0 || strncmp(mydir->d_name,"..", 2) == 0){
				continue ;
			}
			struct stat mystat;
			bzero(&mystat, sizeof(stat));
			stat(mydir->d_name, &mystat);//获取某个文件的信息
			bzero(assist_buf, 1024);
			sprintf(assist_buf, "%-2s%-20s %10ldB", file_type(mystat.st_mode),mydir->d_name, mystat.st_size );
			len =  strlen(assist_buf);
			send(fd, &len, sizeof(int), 0);//发送数据给客户端
			sendn(fd, assist_buf, len);
		}
		len = 0 ;
		send(fd, &len, sizeof(int), 0);
	}
}
//直接通过chdir改变路径,通过getcwd把当前路径传会给客户端
void do_cd(char* assist_buf, socket_t fd) 
{
	char dir[128]= "";
	sscanf(assist_buf +3, "%s", dir);
	chdir(dir);
	getcwd(dir, 128);
	int len = strlen(dir);
	send(fd, &len, sizeof(int), 0);
	sendn(fd, dir, strlen(dir));
}
//通过getcwd拿到当前路径，并发送给客户端
void do_pwd(char* assist_buf, socket_t fd) 
{
	bzero(assist_buf, BUF_SIZE);
	getcwd(assist_buf, BUF_SIZE);
	int len = strlen(assist_buf);
	send(fd, &len, sizeof(int), 0);
	sendn(fd, assist_buf, len);
}
//服务器传文件给客户端
void do_gets(char* assist_buf, socket_t fd) 
{
	char file_name[256];
	int file_pos = 5 ;
	while(bzero(file_name, 256),sscanf(assist_buf + file_pos,"%s", file_name ) == 1)
	{
		file_pos += strlen(file_name) + 1; 
		if(upload(fd, file_name) == 0)
		{
			printf(" file-> %s upload success\n", file_name);
		}else 
		{
			printf(" file-> %s upload failed\n", file_name);
		}
	}
}
// 创建文件夹
void do_mkdir(char* assist_buf, socket_t fd){
	char dir_name[256] = {0};
	int dir_pos = 6 ;
	char path[100] = {0};   
	strcpy(dir_name,assist_buf + dir_pos);
	getcwd(path,sizeof(path)); //获取当前的工作目录
	if(create_dir(dir_name, path) == 0){
		strcpy(assist_buf,"MKDIR SUCCESS!");
		int len = strlen(assist_buf);
		send(fd, &len, sizeof(int),0);
		sendn(fd, assist_buf, len);
	}
	else{
		strcpy(assist_buf,"MKDIR FAIL!");
		int len = strlen(assist_buf);
		send(fd, &len, sizeof(int),0);
		sendn(fd, assist_buf, len);
	}
}
// 删除文件夹
void do_rmdir(char* assist_buf, socket_t fd){
	char dir_name[256] = {0};
	int dir_pos = 6 ;
	// char path[100] = {0};   
	strcpy(dir_name,assist_buf + dir_pos);
	// getcwd(path,sizeof(path)); //获取当前的工作目录
	printf("%s\n",dir_name);
	int ret = rmdir(dir_name);  //直接上就行,参数非常简单
	if(ret == 0){
		strcpy(assist_buf,"RMDIR SUCCESS!");
		int len = strlen(assist_buf);
		send(fd, &len, sizeof(int),0);
		sendn(fd, assist_buf, len);
	}
	else{
		strcpy(assist_buf,"RMDIR FAIL!");
		int len = strlen(assist_buf);
		send(fd, &len, sizeof(int),0);
		sendn(fd, assist_buf, len);
	}
}
// 创建文件
void do_touch(char* assist_buf, socket_t fd){
	char file_name[256] = {0};
	int file_pos = 6 ;
	char path[100] = {0};   
	strcpy(file_name,assist_buf + file_pos);
	int netfd = open(file_name,O_RDWR|O_CREAT,0666);
	if(netfd){
		strcpy(assist_buf,"TOUCH SUCCESS!");
		int len = strlen(assist_buf);
		send(fd, &len, sizeof(int),0);
		sendn(fd, assist_buf, len);
	}
	else{
		strcpy(assist_buf,"TOUCH FAIL!");
		int len = strlen(assist_buf);
		send(fd, &len, sizeof(int),0);
		sendn(fd, assist_buf, len);
	}
}
//上传文件
void do_puts(char* assist_buf, socket_t fd) //puts a.txt b.txt
{
	char file_name[256];
	int file_pos = 5 ;
	//puts file,文件名在file_name中
	while(bzero(file_name, 256), sscanf(assist_buf + file_pos,"%s", file_name) == 1)
	{
		file_pos += strlen(file_name) + 1 ;//为了puts后面跟多个文件名
		if(download(fd, file_name) == 0)
		{
			printf("file -> %s download success \n", file_name);
		}else
		{
			printf("file -> %s download failed \n", file_name);
		}
	}
}
//使用脚本的rm命令来删除文件，目前只能删除普通文件
void do_remove(char* assist_buf, socket_t fd)// remove file 
{
	char cmd[256] ="" ;
	sprintf(cmd, "rm -f %s", assist_buf + 7);
	system(cmd);
	bzero(assist_buf, BUF_SIZE);
	sprintf(assist_buf, "%s removed", assist_buf + 7);
	int len = strlen(assist_buf);
	send(fd, &len, sizeof(int),0);
	sendn(fd, assist_buf, len);
}
void do_error(char* assist_buf, socket_t fd){
	bzero(assist_buf,BUF_SIZE); //优先清理缓存养成习惯
	strcpy(assist_buf,"No this Functional Command!");
	int len = strlen(assist_buf);
	send(fd, &len, sizeof(int),0);
	sendn(fd, assist_buf, len);
}
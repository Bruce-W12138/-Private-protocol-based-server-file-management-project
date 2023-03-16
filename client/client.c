#include "head.h"
int main(int argc, char* argv[]){
    // 写死ip和端口号建立连接来调试
    ARGS_CHECK(argc,3);
    char recv_buf[BUF_SIZE], send_buf[BUF_SIZE];
	int recv_len ,send_len ,read_len, write_len;;
    int fd_client = socket(AF_INET,SOCK_STREAM,0);
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(atoi(argv[2]));
    serverAddr.sin_addr.s_addr = inet_addr(argv[1]);
    int ret = connect(fd_client,(struct sockaddr *)&serverAddr,sizeof(serverAddr));
    ERROR_CHECK(ret,-1,"connect");

    // 连接建立以后客户端开始尝试处理任务
    char cmd[128] = {0};
    while(1){
        bzero(cmd,sizeof(cmd));
        read_len = read(0, cmd, sizeof(cmd));
        trim_space(cmd);  // 写白板
        send_len = strlen(cmd);
        // 客户端中断请求
        if(strcmp("quit",cmd) == 0 || send_len == 0){
            break;
        }
        send(fd_client, &send_len, sizeof(int), 0);//发命令给服务器端
        sendn(fd_client, cmd, send_len);
        /*对应发送到服务端最终会给到子进程的event_loop*/
        if(strncmp("cd", cmd, 2) == 0){
			system("clear");
			recv(fd_client, &recv_len, sizeof(int), 0);//接cd之后的路径
			recvn(fd_client, recv_buf, recv_len );
			printf("%s\n", recv_buf);
		}
        else if(strcmp("ls", cmd) == 0){
			system("clear");
			while(1){
				recv(fd_client, &recv_len, sizeof(int), 0);//接文件信息
				if(recv_len == 0){  //目录读空为止
					break;
				}
				recvn(fd_client, recv_buf, recv_len );
				printf("%s\n", recv_buf);
			}
        }
        else if(strncmp("puts ", cmd, 5) == 0){
			char file_name[256];
			int file_pos = 5 ;
			while(bzero(file_name, 256),sscanf(cmd + file_pos,"%s", file_name ) == 1){
				file_pos += strlen(file_name) + 1; 
				if(upload(fd_client, file_name) == 0){
					printf(" file-> %s upload success\n", file_name);
				}
                else {
					printf(" file-> %s upload failed\n", file_name);
				}
			}
		}
        else if(strncmp("mkdir", cmd, 5) == 0){
            system("clear");
			recv(fd_client,&recv_len,sizeof(int),0);  
			recvn(fd_client,recv_buf,recv_len);  
			printf("%s\n",recv_buf);
        }
        else if(strncmp("touch", cmd, 5) == 0){
            system("clear");
			recv(fd_client,&recv_len,sizeof(int),0);  
			recvn(fd_client,recv_buf,recv_len);  
			printf("%s\n",recv_buf);
        }
        else if(strncmp("rmdir", cmd, 5) == 0){
            system("clear");
			recv(fd_client,&recv_len,sizeof(int),0);  
			recvn(fd_client,recv_buf,recv_len);  
			printf("%s\n",recv_buf);
        }
        else if( strncmp("gets", cmd, 4)== 0){
            char file_name[256];
            int file_pos = 5 ;
            while(bzero(file_name, 256), sscanf(cmd + file_pos,"%s", file_name) == 1){
                file_pos += strlen(file_name) + 1 ;
                if(download(fd_client,file_name )== 0){
                    printf("file -> %s download success \n", file_name);
                }
                else{
                    printf("file -> %s download failed \n", file_name);
                }
            }
		}
        else if( strncmp("remove ", cmd, 7) == 0){
			system("clear");
			recv(fd_client, &recv_len, sizeof(int), 0);
			recvn(fd_client, recv_buf, recv_len );
			printf("%s\n", recv_buf);
		}
        else if(strcmp("pwd", cmd) == 0){
			system("clear");
			recv(fd_client, &recv_len, sizeof(int), 0);
			recvn(fd_client, recv_buf, recv_len );
			printf("%s\n", recv_buf);
		}
        else{
            system("clear");
			recv(fd_client,&recv_len,sizeof(int),0);  //接收火车头
			// printf("%d\n",recv_len);
			recvn(fd_client,recv_buf,recv_len);  // 接收火车车厢，是recvbuf
			printf("%s\n",recv_buf);
			// printf("没有匹配上\n");
            continue;
        }
    }
    close(fd_client);
}
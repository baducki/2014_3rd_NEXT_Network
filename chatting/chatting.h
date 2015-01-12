#ifndef CHATTING_H
#define CHATTING_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>

#define BUF_SIZE 100
#define NAME_SIZE 23
#define MAX_INPUT_NAME 21 // NAME_SIZE에 '[', ']' 이 들어아야 함으로 NAME_SIZE - 2
#define EPOLL_SIZE 50
#define MAX_CLNT 256
#define BUFFER_SIZE 1024 // 긴 대화를 처리할 방법을 찾아야 
#define MAX_PORTNUMBER_SIZE 6 // 총 4자리 숫자 + 1자리 개행문자 + '\0'

#define setnonblockingmode(fd)                                     \
	do {                                                           \
	int flag = fcntl((fd), F_GETFL, 0);                            \
	fcntl((fd), F_SETFL, flag | O_NONBLOCK);                       \
	} while(0)

int valid_check_port_number(char *portnumber);
int get_index_of_EOF(char *string);
void get_port_number(char *portnumber);
void check_duplication_of_clnt_name(int clnt_sock);
int find_clnt_socket_count(int clnt_sock);
void rearrange_clnt_socks_and_clnt_names(int idx);
void send_msg_serv(char *msg, int len);
void error_handling_and_exit(char *msg);
void initialize_socket_address(struct sockaddr_in *addr, char *port_number);
void connect_client(struct sockaddr_in *addr, int serv_sock);

void get_name(char *name);
int check_duplication_of_name(char *name);
void* send_msg(void *arg);
void* recv_msg(void *arg);
void send_file(void);

#endif

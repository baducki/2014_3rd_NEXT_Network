#include "chatting.h"

int clnt_socket_count = 0;
int clnt_socks[MAX_CLNT];
char clnt_names[MAX_CLNT][MAX_INPUT_NAME];

int main(int argc, char *argv[]) {
	if (argc != 1) {
		printf("Usage : %s \n", argv[0]);
		exit(1);
	}

	char portnumber[MAX_PORTNUMBER_SIZE];

	get_port_number(portnumber);

	int sock, serv_sock, clnt_sock, closed_socket_count;
	struct sockaddr_in serv_addr, clnt_addr;
	socklen_t clnt_addr_size = sizeof(clnt_addr);

	int received_msg_len, idx;
	char buffer[BUFFER_SIZE];

	int event_counts, epfd;
	struct epoll_event *epoll_events;
	struct epoll_event event;

	serv_sock = socket(PF_INET, SOCK_STREAM, 0);
	if (serv_sock == -1)
		error_handling_and_exit("socket() error");

	initialize_socket_address(&serv_addr, portnumber);

	if (bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
		error_handling_and_exit("bind() error");

	if (listen(serv_sock, MAX_CLNT) == -1)
		error_handling_and_exit("listen() error");

	fputs("\n <Chatting Server is On> \n", stdout);

	epfd = epoll_create(1); //epll_create의 인자값은 0보다 크기만 하면 되기에 1을 인자로 사용
	if (epfd == -1)
		error_handling_and_exit("epoll_create() error");

	epoll_events = malloc(sizeof(struct epoll_event) * EPOLL_SIZE);
	if (epoll_events == NULL)
		error_handling_and_exit("malloc() error");

	setnonblockingmode(serv_sock);
	event.events = EPOLLIN;
	event.data.fd = serv_sock;
	epoll_ctl(epfd, EPOLL_CTL_ADD, serv_sock, &event);

	while (1) {
		event_counts = epoll_wait(epfd, epoll_events, EPOLL_SIZE, -1);
		if (event_counts == -1)
			error_handling_and_exit("epoll_wait() error");

		for (idx = 0; idx < event_counts; idx++) {
			sock = epoll_events[idx].data.fd;

			if (sock == serv_sock) { // 클라인언트가 입장했을 때
				clnt_sock = accept(sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
				if (clnt_sock < 0) {
					perror("accept() error : ");
					continue;
				}

				if (clnt_socket_count <= 50) {
					clnt_socket_count++;
					clnt_socks[clnt_socket_count] = clnt_sock;
					check_duplication_of_clnt_name(clnt_sock);

					setnonblockingmode(clnt_sock);
					event.events = EPOLLIN | EPOLLET;
					event.data.fd = clnt_sock;

					if (epoll_ctl(epfd, EPOLL_CTL_ADD, clnt_sock, &event) == -1) {
						perror("epoll_ctl() error : ");
						continue;
					}

					printf("connected client %d\n", clnt_sock);
					send_msg_serv("\n ----- < ", strlen("\n ----- < "));
					send_msg_serv(clnt_names[clnt_socket_count], strlen(clnt_names[clnt_socket_count]));
					send_msg_serv("님이 방에 입장하셨습니다. > ----- \n", strlen("님이 방에 입장하셨습니다. > ----- \n"));
				}
				else {
					send_msg_serv("정원이 초과되어 방에 입장하실 수 없습니다! \n", strlen("정원이 초과되어 방에 입장하실 수 없습니다! \n"));
					close(clnt_sock);
				}
			}

			else { // 클라이언트가 메세지를 보낼 때
				clnt_sock = epoll_events[idx].data.fd;
				memset(buffer, 0, BUFFER_SIZE);
				received_msg_len = read(clnt_sock, buffer, BUFFER_SIZE);

				if (received_msg_len == 0) {
					epoll_ctl(epfd, EPOLL_CTL_DEL, clnt_sock, NULL);
					printf("closed client %d\n", clnt_sock);
					closed_socket_count = find_clnt_socket_count(clnt_sock);
					close(clnt_sock);

					send_msg_serv("\n ----- < ", strlen("\n ----- < "));
					send_msg_serv(clnt_names[closed_socket_count], strlen(clnt_names[closed_socket_count]));
					send_msg_serv("님이 방에서 나갔습니다. > ----- \n", strlen("님이 방에서 나갔습니다. > ----- \n"));

					rearrange_clnt_socks_and_clnt_names(closed_socket_count);
					clnt_socket_count--;
				}
				else if (received_msg_len < 0) {
					perror("read() error : ");
					continue;
				}
				else
					send_msg_serv(buffer, received_msg_len);
			} // the end of else
		} // the end of for loop
	} // the end of while loop

	close(serv_sock);

	return 0;
}

void get_port_number(char *portnumber) {
	int result;
	int idx;

	do {
		fputs(" - 서버 port 번호를 입력하세요(숫자 네자리 이하로 입력) :  ", stdout);
		fgets(portnumber, MAX_PORTNUMBER_SIZE, stdin);

		idx = get_index_of_EOF(portnumber);
		if (idx == 0 || idx == 1)
			continue;

		else
			portnumber[idx - 1] = '\0';

		result = valid_check_port_number(portnumber);
	} while (result != 0);
}

int get_index_of_EOF(char *string) {
	if (string == NULL)
		return 0;

	int idx = 0;

	while (string[idx] != '\0')
		idx++;

	return idx;
}

int valid_check_port_number(char *portnumber) {
	int idx;

	for (idx = 0; portnumber[idx] != '\0'; idx++) {
		if (portnumber[idx] < 48 || portnumber[idx] > 57) {
			fputs("\n<port 번호는 반드시 숫자를 입력해야 합니다>\n\n", stderr);
			return -1;
		}
	}
	return 0;
}

void initialize_socket_address(struct sockaddr_in *addr, char *port_number) {
	addr->sin_family = AF_INET;
	addr->sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	addr->sin_port = htons(atoi(port_number));
}

void check_duplication_of_clnt_name(int clnt_sock) {
	int idx, str_len, check_result;

	do {
		str_len = read(clnt_sock, clnt_names[clnt_socket_count], MAX_INPUT_NAME - 1);
		clnt_names[clnt_socket_count][str_len] = '\0';

		for (idx = 0; idx < clnt_socket_count; idx++) {
			check_result = strcmp(clnt_names[clnt_socket_count], clnt_names[idx]);

			if (check_result == 0) {
				clnt_names[clnt_socket_count][0] = '\0';
				write(clnt_sock, "-1", strlen("-1"));
				break;
			}
		} // end of for-loop
	} while (check_result == 0);

	write(clnt_sock, "0", strlen("0"));
}

void send_msg_serv(char *msg, int len) {
	int idx;
	for (idx = 1; idx <= clnt_socket_count; idx++)
		write(clnt_socks[idx], msg, len);
}

int find_clnt_socket_count(int clnt_sock) {
	int idx = 1;

	while (clnt_sock != clnt_socks[idx])
		idx++;

	return idx;
}

void rearrange_clnt_socks_and_clnt_names(int idx) {
	while (idx < clnt_socket_count) {
		clnt_socks[idx] = clnt_socks[idx + 1];
		strcpy(clnt_names[idx], clnt_names[idx + 1]);
		idx++;
	}
	clnt_names[idx][0] = '\0';
}

void error_handling_and_exit(char * msg) {
	perror(msg);
	exit(1);
}

#include "chatting.h"

char name[NAME_SIZE];
char msg[BUF_SIZE];

int sock;

int main(int argc, char *argv[]) {
	struct sockaddr_in serv_addr;

	pthread_t send_thread, recv_thread;
	void *thread_return;

	if (argc != 3) {
		printf("Usage : %s <IP> <PORT>\n", argv[0]);
		exit(1);
	}

	sock = socket(PF_INET, SOCK_STREAM, 0);
	if (sock == -1)
		error_handling_and_exit("socket() error");

	initialize_socket_address(&serv_addr, argv[2]);

	if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
		error_handling_and_exit("connect() error");

	get_name(name);

	pthread_create(&send_thread, NULL, send_msg, NULL);
	pthread_create(&recv_thread, NULL, recv_msg, NULL);

	pthread_join(send_thread, &thread_return);
	pthread_join(recv_thread, &thread_return);

	close(sock);

	return 0;
}

void get_name(char *name) {
	int idx, result;
	char *temp_name;
	temp_name = (char*)malloc(sizeof(char)* MAX_INPUT_NAME);

	if (temp_name == NULL)
		error_handling_and_exit("malloc() error");

	while (1) {
		fputs(" - 채팅이름을 입력하세요(한글 열자 이하로 입력) :  ", stdout);
		fgets(temp_name, MAX_INPUT_NAME, stdin);
		idx = get_index_of_EOF(temp_name);

		if (idx <= 1) continue;

		temp_name[idx - 1] = '\0';

		result = check_duplication_of_name(temp_name);

		if (result == -1) {
			fputs("이미 사용하고 있는 이름입니다. 다시입력해주세요.\n", stdout);
			continue;
		}

		break;
	}

	sprintf(name, "[%s]", temp_name);
	free(temp_name);
}

int check_duplication_of_name(char *name) {
	int str_len, result;
	char check_result[3];

	write(sock, name, strlen(name));

	str_len = read(sock, check_result, 2);
	check_result[str_len] = '\0';

	result = strcmp(check_result, "-1");

	if (result == 0) return -1;

	return 0;
}

int get_index_of_EOF(char *string) {
	if (string == NULL)
		return 0;

	int idx = 0;

	while (string[idx] != '\0')
		idx++;

	return idx;
}

void initialize_socket_address(struct sockaddr_in *addr, char *port) {
	addr->sin_family = AF_INET;
	addr->sin_addr.s_addr = htonl(INADDR_ANY);
	addr->sin_port = htons(atoi(port));
}

void error_handling_and_exit(char * msg) {
	perror(msg);
	exit(1);
}

void* send_msg(void *arg) {
	char name_msg[NAME_SIZE + BUF_SIZE];

	while (1) {
		fgets(msg, BUF_SIZE, stdin);

		if (!strcmp(msg, "q\n") || !strcmp(msg, "Q\n"))
			exit(0);

		if (!strcmp(msg, "/sendfile\n") || !strcmp(msg, "/SENDFILE\n")) {
			send_file();
			continue;
		}

		sprintf(name_msg, "%s %s", name, msg);
		write(sock, name_msg, strlen(name_msg));
	}

	return NULL;
}

void send_file(void){

}

void* recv_msg(void *arg) {
	char name_msg[NAME_SIZE + BUF_SIZE];
	int str_len;

	while (1) {
		str_len = read(sock, name_msg, NAME_SIZE + BUF_SIZE - 1);

		if (str_len == -1)
			return (void*)-1;

		name_msg[str_len] = '\0';
		fputs(name_msg, stdout);
	}

	return NULL;
}

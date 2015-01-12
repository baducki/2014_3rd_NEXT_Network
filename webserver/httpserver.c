#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>

#define BUF_SIZE   4096
#define QUEUE_SIZE 1024
#define PORT       9000

void error_handling_and_exit(char *msg);

int main(int argc, char *argv[]) {
	if (argc != 1) {
		printf("Usage : %s \n", argv[0]);
		exit(1);
	}

	int serv_sock, clnt_sock, received_msg_len, fd;
	struct sockaddr_in serv_addr, clnt_addr;
	socklen_t clnt_addr_size = sizeof(clnt_addr);

	char HTTPheader[] = "HTTP/1.0 200 OK\r\nDATE: Mon, 12 January 2015 17:17:17 GMT\r\n\r\n";
	char message[BUF_SIZE];
	char buffer[BUF_SIZE];

	char *end_of_file_location;
	char *start_of_file_location;
	char *file_location_ptr;
	char file_locations[BUF_SIZE];

	serv_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (serv_sock == -1)
		error_handling_and_exit("socket() error");

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(PORT);

	if (bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
		error_handling_and_exit("bind() error");

	fputs("\n 브라우저를 실행 후 주소창에 localhost:9000번을 입력하세요! \n", stdout);

	if (listen(serv_sock, QUEUE_SIZE) == -1)
		error_handling_and_exit("listen() error");

	while (1) {
		clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_addr, &clnt_addr_size);

		if (clnt_sock == -1)
			error_handling_and_exit("accept() error");

		if (clnt_sock < 4)
			continue;

		received_msg_len = recv(clnt_sock, message, BUF_SIZE, 0);
		message[received_msg_len] = '\0';

		end_of_file_location = strstr(message, " HTTP");
		start_of_file_location = strstr(message, "/");

		memset(file_locations, 0, sizeof(file_locations));
		file_location_ptr = strncpy(file_locations, start_of_file_location, end_of_file_location - (start_of_file_location));
		file_location_ptr = strtok(file_locations, " ");

		if (!strcmp(file_locations, "/"))
			file_location_ptr = "/index.html";

		printf("%s\n", file_location_ptr);

		send(clnt_sock, HTTPheader, strlen(HTTPheader), 0);

		if ((fd = (int)fopen(++file_location_ptr, "r"))) {
			while ((received_msg_len = fread(buffer, 1, BUF_SIZE, (FILE *)fd)) > 0)
				send(clnt_sock, buffer, received_msg_len, 0);
		}
		else {
			fd = open("404error.html", O_RDONLY);

			while ((received_msg_len = read(fd, buffer, BUF_SIZE)) > 0)
				send(clnt_sock, buffer, received_msg_len, 0);
		}
		close(fd);
		close(clnt_sock);
	}
	close(serv_sock);

	return 0;
}

void error_handling_and_exit(char * msg) {
	perror(msg);
	exit(1);
}

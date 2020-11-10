// Server program 
#include <arpa/inet.h> 
#include <errno.h> 
#include <netinet/in.h> 
#include <signal.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <strings.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <unistd.h> 
#define PORT 12345 
#define BUFSIZE 256 


//패킷구조 : TYPE 1byte + Code 1byte + MESSAGE(the buffer - string)
// Type value used in USP message
#define MSG_REGISTER 1
#define MSG_DEREGISTER 3
#define MSG_GET 5
#define MSG_VERIFY 7
#define MSG_VERIFY_RESPONSE 8

// Code value used in USP message
#define MSG_SUCCESS 0
#define MSG_FAILED 1

int main(int argc, char * argv[])
{
	fd_set reads, temp;
	int str_len;

	//sock들 셋팅하기
	int sock1;                                      //VM3로 보낼때
	struct sockaddr_in server_addr;
	int len_sock1 = sizeof(server_addr);
	if ((sock1 = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
	{
		perror("socket for verify (VM1->VM3) is not made");
		exit(1);
	}
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(12345);
	server_addr.sin_addr.s_addr = inet_addr("10.1.2.2");
	bind(sock1, (struct sockaddr*)&server_addr, sizeof(server_addr));


	int sock2;                           //VM1로 보낼때
	struct sockaddr_in s_addr;
	int len_sock2 = sizeof(s_addr);
	if ((sock2 = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
	{
		perror("socket for verify (VM3->VM1) is not made");
		exit(1);
	}
	memset(&s_addr, 0, sizeof(s_addr));
	s_addr.sin_family = AF_INET;
	s_addr.sin_port = htons(12345);
	s_addr.sin_addr.s_addr = inet_addr("10.1.1.2");
	bind(sock2, (struct sockaddr*)&s_addr, sizeof(s_addr));

	int sock3;
	struct sockaddr_in se_addr;      //VM2로 보낼때
	if ((sock3 = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
	{
		perror("socket for DNS SERVER is not made");
		exit(1);
	}
	memset(&se_addr, 0, sizeof(se_addr));
	se_addr.sin_family = AF_INET;
	se_addr.sin_port = htons(12345);
	se_addr.sin_addr.s_addr = inet_addr("10.1.1.1");
	int len_sock3 = sizeof(se_addr);
	bind(sock3, (struct sockaddr*)&se_addr, sizeof(se_addr));


	int sock4;                                      //VM3로 보낼때
	struct sockaddr_in _addr;
	int len_sock4 = sizeof(_addr);
	if ((sock4 = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
	{
		perror("socket for verify (VM1->VM3) is not made");
		exit(1);
	}
	memset(&_addr, 0, sizeof(_addr));
	_addr.sin_family = AF_INET;
	_addr.sin_port = htons(12345);
	_addr.sin_addr.s_addr = inet_addr("10.1.2.2");
	bind(sock4, (struct sockaddr*)&_addr, sizeof(_addr));





	FD_ZERO(&reads);
	FD_SET(sock1, &reads); //VM3
	FD_SET(sock2, &reads); //VM1
	FD_SET(sock3, &reads); //서버랑 통신
	FD_SET(0, &reads);
	while (1)
	{
		int result, maxfd = 0;
		char message[BUFSIZE] = { 0 };
		char buffer[BUFSIZE] = { 0 };
		char verifyname[10] = { 0 }; //get의 이름이 저장되어있는 곳
		char verifyip[100] = { 0 };
		char str[15] = { 0 };
		unsigned char type;
		//unsigned char result;
		unsigned short len;
		ssize_t rcv_bytes;
		ssize_t sent_bytes;
		int max = 0;
		int mode = 0;//verify인지 아닌지 확인
		temp = reads;

		if (max < sock1)   max = sock1;
		if (max < sock2)   max = sock2;
		if (max < sock3)   max = sock3;
		printf("\n\n");
		printf("quit\n");
		printf("add <My hostname> <My IPv4 address>\n");
		printf("del <My hostname> <My IPv4 address>\n");
		printf("get <target hostname>\n");
		printf("verify <target hostname>\n\n");
		printf("위의 5줄중 하나와 동일한 양식으로 타이핑 후 enter하세요\n : 예시) get VM3\n");
		printf("예시) verify vm3\n");
		printf("예시) add vm10 10.10.10.10\n");

		result = select(max + 1, &temp, 0, 0, 0);
		if (result < 0) //select오류 발생
		{
			perror(" ERROR : SELECT got error");
			exit(1);
		}
		if (result == 0) continue; //입력 받지 못함

		if (FD_ISSET(0, &temp))  //키보드 입력데이터를 서버로 전송
		{
			fgets(message, sizeof(message), stdin); //standard input
			printf("\nsent_data  :  %s\n", message);
			char  name[10];
			char  ipv4__addr[100];
			char what_client_want[10];
			sscanf(message, "%s %s %s", what_client_want, name, ipv4__addr);
			printf("\nsend_data  :  %s\n", what_client_want);
			printf("\nsend_data  :  %s\n", name);
			printf("\nsend_data  :  %s\n", ipv4__addr);

			if (strlen(message) > 0)
			{
				if (message[0] == 'q') break;
				else if (message[0] == 'a' || message[0] == 'd' || message[0] == 'g' || message[0] == 'v')
				{
					if (strcmp(what_client_want, "verify") == 0) {
						mode = MSG_VERIFY;
					}
					sent_bytes = sendto(sock3, message, sizeof(message), 0, (struct sockaddr *)&se_addr, sizeof(struct sockaddr));
				}
				else {
					printf("\n형식에 맞춰 입력해주세요\n형식에 맞춰 입력해주세요\n형식에 맞춰 입력해주세요\n형식에 맞춰 입력해주세요\n");
				}
			}
			FD_CLR(0, &temp);
		}

		if (FD_ISSET(sock1, &temp)) //VM3에서----상대방과 통신verify-->VM1  : VM1이 답장하는 부분
		{
			rcv_bytes = recvfrom(sock1, message, sizeof(message), 0, (struct sockaddr *)&server_addr, &len_sock1);
			if (message[0] == 'v')
			{
				printf("\n VERIFICATION COMPLETE \n");
			}
			FD_CLR(sock1, &temp);
		}

		if (FD_ISSET(sock2, &temp)) //VM1에서----상대방과 통신verify-->VM3  : VM3가 답장하는 부분
		{
			rcv_bytes = recvfrom(sock2, message, sizeof(message), 0, (struct sockaddr *)&s_addr, &len_sock2);
			if (message[0] == 'v')
			{
				strcpy(message, "VERIFICATION COMPLETE");
				sent_bytes = sendto(sock1, message, sizeof(message), 0, (struct sockaddr *)&server_addr, sizeof(struct sockaddr));

			}
			FD_CLR(sock2, &temp);
		}

		if (FD_ISSET(sock3, &temp)) //DNS와 통신  : DNS의 답장을 받는 부분
		{
			rcv_bytes = recvfrom(sock3, message, sizeof(message), 0, (struct sockaddr *)&se_addr, &len_sock3);      //here???
			if (mode == MSG_VERIFY) {
				if (verifyip == NULL) {
					printf("\n Verify 시도전에 GET을 먼저 실행해야합니다.\n");
					continue;
				}
				memset(&_addr, 0, sizeof(_addr));
				_addr.sin_family = AF_INET;
				_addr.sin_port = htons(12345);
				_addr.sin_addr.s_addr = inet_addr(verifyip);
				char sendtoOther[250] = "VERIFY REQUEST";
				sent_bytes = sendto(sock4, sendtoOther, sizeof(message), 0, (struct sockaddr *)&_addr, sizeof(struct sockaddr));

	
			}
			else {
				if (message[0] == 'G' && message[4] == 'S') {
					char tmp1[10]; //GET
					char tmp2[10]; //SUCCESS
					sscanf(message, "%s %s %s", tmp1, tmp2, verifyip);
				}
				else if(message[0] == 'G' && message[4] == 'F')
				{
					memset(verifyname, 0, sizeof(verifyname));
				}
				if (message[0] == 'G' && message[4] == 'S') { //GET한 IP주소를 네트워크바이트--> 00.00.00.00형식으로
					char tmp1[10]; //GET
					char tmp2[10]; //SUCCESS
					char tmp3[100]; //겟한 IP주소
					char* _string = NULL;
					char printre[200] = {0};

					sscanf(message, "%s %s %s", tmp1, tmp2, tmp3);
					char * addrp = *((unsigned int *)tmp3);
					struct sockaddr_in ip_print;
					ip_print.sin_addr.s_addr = htonl(addrp);//transform to Network Byte Form
					_string = inet_ntoa(ip_print.sin_addr);//Network Byte Form ->  00.00.00.00형식으로

					printf("%s\n++++",_string);




				}
				printf("\n--The RESPONSE <<<<  %s  >>>>----------------------\n", message);
			}
			FD_CLR(sock3, &temp);
		}
	}
}
#include <sys/socket.h>
#include <linux/netlink.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#define NETLINK_USER 31

// Type value used in kernel message
#define MSG_REGISTER 1
#define MSG_REGISTER_RESPONSE 2
#define MSG_DEREGISTER 3
#define MSG_DEREGISTER_RESPONSE 4
#define MSG_GET 5
#define MSG_GET_RESPONSE 6

// Code value used in kernel message
#define MSG_SUCCESS 0
#define MSG_FAILED 1
#define SET_MSG_CHAR(name, x) (*name = x); (name += 1);
#define SET_MSG_SHORT(name, x) (*(unsigned short *)name = htons(x)); (name += 2);
#define SET_MSG_INTEGER(name, x) (*(unsigned int *)name = htonl(x)); (name += 4);
#define SET_MSG_STRING(name, x) (strcpy(name, x)); (name += strlen(x)+1);
#define MAX_PAYLOAD 1024 /* maximum payload size*/

int sock_fd; // netlink socket
int service_sock_fd;
struct sockaddr_in server_addr_sendto;


unsigned short set_add_message(char * name, unsigned int ipv4_addr, char * hdr_pointer) {
	unsigned short msg_len = 7 + strlen(name) + 1; // default kernel request header length 3 + sizeof(integer) + strlen(name) + NULL Value for string => 7 + strlen(name) + 1
	SET_MSG_CHAR(hdr_pointer, MSG_REGISTER)
	SET_MSG_SHORT(hdr_pointer, msg_len)
	SET_MSG_INTEGER(hdr_pointer, ipv4_addr)
	SET_MSG_STRING(hdr_pointer, name)
		return msg_len;
}


unsigned short set_del_message(char * name, unsigned int ipv4_addr, char * hdr_pointer) {
	unsigned short msg_len = 7 + strlen(name) + 1;

	// default kernel request header length 3 + sizeof(integer) + strlen(name) + NULL Value for string => 7 + strlen(name) + 1
	SET_MSG_CHAR(hdr_pointer, MSG_DEREGISTER)
	SET_MSG_SHORT(hdr_pointer, msg_len)
	SET_MSG_INTEGER(hdr_pointer, ipv4_addr)
	SET_MSG_STRING(hdr_pointer, name)
		return msg_len;
}


unsigned short set_get_message(char * name, char * hdr_pointer) {
	unsigned short msg_len = 3 + strlen(name) + 1;

	// default kernel request header length 3 + strlen(name) + NULL Value for string => 3 + strlen(name) + 1
	SET_MSG_CHAR(hdr_pointer, MSG_GET)
	SET_MSG_SHORT(hdr_pointer, msg_len)
	SET_MSG_STRING(hdr_pointer, name)
		return msg_len;
}



unsigned short send_add_message(char * name, unsigned int ipv4_addr, char * buf_rcv) {
	// buf_rcv : response message buffer for client, not netlink
	// With this function, you can register entry that has name and ipv4_addr.
	char * d_point = NULL;
	char * additional_data_ptr;
	unsigned short rcv_len;
	struct nlmsghdr *nlh = NULL;
	char buf[NLMSG_SPACE(MAX_PAYLOAD)];
	unsigned int host_ipv4 = htonl(ipv4_addr);

	memset(buf, 0, NLMSG_SPACE(MAX_PAYLOAD));

	nlh = (struct nlmsghdr *)buf;
	nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
	nlh->nlmsg_pid = getpid();
	nlh->nlmsg_flags = 0;

	printf("Sending Registration message(%s, %s) to kernel\n", name, inet_ntoa(*(struct in_addr *)&host_ipv4));
	set_add_message(name, ipv4_addr, (char *)NLMSG_DATA(nlh));
	send(sock_fd, buf, nlh->nlmsg_len, 0);
	printf("Waiting for message from kernel\n");

	/* Read message from kernel */
	recv(sock_fd, buf, nlh->nlmsg_len, 0);
	// response from kernel is saved to buf (read data and )
	d_point = (char *)NLMSG_DATA(nlh);// get data starting pointer to process received data

	struct nlmsghdr* receivedData = (struct nlmsghdr*)d_point;
	char* readmsg = (char*)d_point;

	if (readmsg[1] == MSG_SUCCESS) {
		printf("\nget the right response from hello module success\n");
		strcpy(buf_rcv, "SUCCESS");            //0x00 0x53 0x48....
		return 1; //success
	}
	else if (readmsg[1] == MSG_FAILED) {
		strcpy(buf_rcv, "FAILED");               //0x01 0x45 0x41....
		return 0;  //failed
	}
	// At this point, you can process GET Response message from kernel. Fill the blank.
}


unsigned short send_del_message(char * name, unsigned int ipv4_addr, char * buf_rcv) {
	// buf_rcv : response message buffer for client, not netlink
	// With this function, you can delete entry that has name and ipv4_addr.
	char * d_point;
	char * additional_data_ptr;
	unsigned short rcv_len;
	struct nlmsghdr *nlh = NULL;
	char buf[NLMSG_SPACE(MAX_PAYLOAD)];
	unsigned int host_ipv4 = htonl(ipv4_addr);
	memset(buf, 0, NLMSG_SPACE(MAX_PAYLOAD));
	nlh = (struct nlmsghdr *)buf;
	nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
	nlh->nlmsg_pid = getpid();
	nlh->nlmsg_flags = 0;

	printf("Sending De-registration message(%s, %s) to kernel\n", name, inet_ntoa(*(struct in_addr *)&host_ipv4));
	set_del_message(name, ipv4_addr, (char *)NLMSG_DATA(nlh));
	send(sock_fd, buf, nlh->nlmsg_len, 0);
	printf("Waiting for message from kernel\n");

	/* Read message from kernel */
	recv(sock_fd, buf, nlh->nlmsg_len, 0);
	d_point = (char *)NLMSG_DATA(nlh); // get data starting pointer to process received data
	 // At this point, you can process GET Response message from kernel. Fill the blank.

	char* forudp = (char*)d_point;
	if (forudp[1] == MSG_SUCCESS) {
		strcpy(buf_rcv, "DELETE SUCCESS \n");
		return 1; //success
	}
	else {
		strcpy(buf_rcv, "DELETE FAIL \n");
		return 0; //fail
	}
}



unsigned short send_get_message(char * name, char * buf_rcv) {
	// buf_rcv : response message buffer for client, not netlink
	// With this function, you can get IPv4 address which of name is same.
	char * d_point;
	char * additional_data_ptr;
	unsigned short rcv_len;
	struct nlmsghdr *nlh = NULL;
	char buf[NLMSG_SPACE(MAX_PAYLOAD)];
	memset(buf, 0, NLMSG_SPACE(MAX_PAYLOAD));
	nlh = (struct nlmsghdr *)buf; // See buffer as struct nlmsghdr메세지 헤더형태로 강제 형변환시킴
								  //이 메세지들을 아래에 send함수로 커널로
	nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD); // set message length
	nlh->nlmsg_pid = getpid(); // set my pid자기PID를 알아내서 셋팅해줌
	nlh->nlmsg_flags = 0; // set flags
	printf("Sending Translation Request(%s -> IPv4) message to kernel\n", name);
	// format get message using name, and write get message to payload part of buffer.
	set_get_message(name, (char *)NLMSG_DATA(nlh));  //name의 메세지의 데이터파트 (커널로)복사해주는 부분이다
	send(sock_fd, buf, nlh->nlmsg_len, 0); // send data이렇게 보내면 hello_nl_recv_msg함수가 받고 복사해서 되돌려줌
	 //sockk_fd는 아까 열었던 그 소 켓이다

	printf("Waiting for message from kernel\n");

	/* Read message from kernel */
	recv(sock_fd, buf, nlh->nlmsg_len, 0); // receive얘가 기다렸다가 받는다
	d_point = (char *)NLMSG_DATA(nlh); // get data starting pointer to process received data
	char* point = d_point;				   // At this point, you can process GET Response message from kernel. Fill the blank.
	point++;

	if (*point++ == MSG_SUCCESS)
	{
		point++;
		point++;
		struct sockaddr_in address;
      	address.sin_addr.s_addr = htonl(point);//transform to Network Byte Form
      	char* tmpstr = inet_ntoa(address.sin_addr);//Network Byte Form ->  Dotted-Decimal Notation
		strcat(buf_rcv, "\nGET SUCCESS ");
		strcat(buf_rcv, tmpstr);
		return 1; //success
	}
	else {
		printf("\nGET request FAIL\n");
		strcat(buf_rcv, "GET FAIL");
		return 0; //fail
	}
}





int main(void) {
	char buff_rcv[256] = { 0 };
	struct sockaddr_nl src_addr;
	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;
	int addr_len;

	service_sock_fd = socket(PF_INET, SOCK_DGRAM, 0); // Client <-> Client VM2<------>VM1/VM3
	sock_fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_USER); // VM2 <-> Module
											  //SOCK_RAW == SOCK_DATAGRAM
	if (sock_fd < 0 || service_sock_fd < 0)
		return -1;
	memset(&server_addr, 0, sizeof(server_addr));
	memset(&src_addr, 0, sizeof(src_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(12345); // Service Port is 12345 !!!!(Use this port at VM1 and VM3.)!!!!
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // receive data regardless dest IPv4 address
													 // I have interface that this destination address is assigned => dest port is 12345 => receive.
													 // Identifier is (UDP, *, 12345, *, *), (UDP, D IP, D Port, S IP, S Port)

	src_addr.nl_family = AF_NETLINK;
	src_addr.nl_pid = getpid(); /* self pid */
	bind(service_sock_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
	bind(sock_fd, (struct sockaddr*)&src_addr, sizeof(src_addr));
	addr_len = sizeof(client_addr);
	char * copy = (char*)malloc(sizeof(char) * 256);


	while (1) {
		char buf[256];
		memset(buf, 0, 256);
		memset(copy, 0, 256);
		int len = 0;
		int flag = 0;
		//(UDP통신)recvfrom으로부터 client로부터 UDP패킷이 왔는지 확인하고, socket으로 받으면 자료수신을 함
		if (len = recvfrom(service_sock_fd, buff_rcv, 256, 0, (struct sockaddr *)&client_addr, &addr_len) > 0)//UDP/IP통신에서 소켓으로부터 자료수신함수
		{
			char name[20] = { 0 };
			char ipv4__addr[20] = { 0 };
			char what_client_want[20] = { 0 };
			int forname = 0;
			int foraddr = 0;
			strcpy(copy, buff_rcv);               // original에다가 가장 처음에 UDP패킷에 담겨져온 원본자료를 그대로 보관함  
			printf("Data given: %s\n", buff_rcv);
			copy = strtok(copy, " ");
			strcpy(what_client_want, copy);
			copy = strtok(NULL, " ");
			strcpy(name, copy);
			name[3] = '\0';
			//      struct sockaddr_in server_addr_sendto;
			memset(&server_addr_sendto, 0, sizeof(server_addr_sendto));
			server_addr_sendto.sin_family = AF_INET;
			struct sockaddr_in temp;

			if (!name) {
				fprintf(stderr, "receive from error!\n");
				continue;
			}

			switch (buff_rcv[0]) {
			case 'a':
				copy = strtok(NULL, " ");
				strcpy(ipv4__addr, copy);
				printf("buff_rcv: %s\n", buff_rcv);
				printf("what_client_want: %s\n", what_client_want);
				printf("name: %s\n", name);
				printf("ipv4__addr: %s\n", ipv4__addr);
				unsigned int ipv4_addr = ntohl(inet_addr(ipv4__addr));
				flag = send_add_message(name,ipv4_addr, buf); //buf인지 buf_rcv인지 확인
				if (flag == 1) strcat(buf, "ADD SUCCESS");
				else strcat(buf, "ADD FAILED");
				break;

			case 'd':
				copy = strtok(NULL, " ");
				strcpy(ipv4__addr, copy);
				printf("buff_rcv: %s\n", buff_rcv);
				printf("what_client_want: %s\n", what_client_want);
				("name: %s\n", name);
				printf("ipv4__addr: %s\n", ipv4__addr);
				ipv4_addr = atoi(ipv4__addr);
				flag = send_del_message(name, (unsigned int)ipv4_addr, buf);
				if (flag == 1) strcpy(buf, "DELETE SUCCESS");
				else strcpy(buf, "DELETE FAILED");
				break;

			case 'g':
				printf("Data given: %s\n", buff_rcv);
				printf("what_client_want: %s\n", what_client_want);
				printf("name: %s\n", name);
				flag = send_get_message(name, buf);
				break;

			case 'v':


				break;

			default:
				//Error processing
				break;
			}

			if (len = sendto(service_sock_fd, buf, sizeof(buf), 0, (struct sockaddr*)&client_addr, addr_len) < 0) {
				printf("\n FAILED sending UDP packet back to client  \n");//UDP/IP통신에서 소켓으로부터 자료수신함수
			}
			
			else {
				printf("%s\n", buf);
				printf("\n SUCCESS sending UDP packet back to client  \n");
			}

			//sendto(service_sock_fd, buf, sizeof(buf), 0, (struct sockaddr *)&client_addr, &addr_len);
		}
		//close(sock_fd);
	}
	free(copy);
	close(sock_fd);
}


#define NOMINMAX
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib, "Ws2_32")
/*CS423 Project1
  Aldo Anaya
  This program will create a socket and send/receive messages
  to a server.  the body of the messages will need to be
  encrypted and decrypted.*/

#include<stdio.h>
#include<stdlib.h> 
#include<WinSock2.h>
#include<Windows.h>
#include<process.h>
#include<WS2tcpip.h>
#include<iostream>
#include<time.h>
#include<sstream>
#include<string>
#include"cipher.h"
#include <time.h>
#include<limits>


using namespace std;

//		global variable listen_on, set to 1 when child thread is listening
//		 set to 0 when message has arrived
int listen_on = 1;
CONST CHAR *SERVER = "192.168.10.200";
CONST CHAR *PORT = "23456";
char server_reply[500];

//		global variable endFlag is set to 1 when user wants to 
//		 exit the program	
int endFlag = 0;
//		 sendMsg function to send message to buddy
void sendMsg(SOCKET	socket, int I, string s, struct addrinfo *server, int MessageType);
//		function listen(void *) used by child thread to 
//		listen to socket continuesly.
void listen(void *);
//		 function to decrypt Message from server using cipher.h
void decryptMessage(int recv_size);

struct addrinfo server_struct, *servinfo, *p;
int msgNumber;
string user = "";

int main(int argc, char*argv[]) {
	WSADATA wsa;
	SOCKET s;
	cout << "Welcome to CS423 IM program" << endl;
	
	char *nameMessage;
	srand(time(NULL));
	int return_value;
	msgNumber = 10000 + rand() % 90000;

	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		printf("Failed. Error Code: %d", WSAGetLastError());
		return 1;
	}

	

	memset(&server_struct, 0, sizeof server_struct);
	server_struct.ai_family = AF_UNSPEC;
	server_struct.ai_socktype = SOCK_DGRAM;
	//Create a socket
	if ((return_value = getaddrinfo(SERVER, PORT, &server_struct, &servinfo)) != 0){
		fprintf(stderr,"getaddrinfo:%s\n", gai_strerror(return_value));
		return 1;
	}
	s = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);

	//set SO_RESEADDR on a socket s to true 
	
	int optval=1;
	setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const char*)&optval, sizeof optval);

	printf("connected to %s\n", SERVER);

	
	printf("initial message number is %d\n", msgNumber);
	printf("Enter your IM name: \n");
	
	getline(cin, user);

	string encryptedMessage = "";
	
	sendMsg(s, msgNumber, user, servinfo, 1);
	msgNumber = msgNumber+1;
	/*
	Create second thread here to listen at all times
	*/
	_beginthread(listen, 0, (void *)&s);

	char action;




	do {
		
		cout << "Enter q (for quit), s (send msg), or c (check for msgs)" << endl;
		if (listen_on == 0)
			action = 'c';
		else {
			cin.get(action);
		}
		//	Loop through menu
		switch (action) {
		case 's':
			sendMsg(s,msgNumber, user, servinfo,2);
			msgNumber += 1;
			//cout << "Enter q (for quit), s (send msg), or c (check for msgs)" << endl;
			//cin.sync();
			//cin.ignore();
		//	if (listen_on == 0)
		//		action = 'c';
		//	else
		//	{
		//		cin.get(action);
		//	}
			break;

		case 'c':
			if (listen_on == 0) {
				printf("New Message Arrived \n");
				cout << server_reply << endl;
				listen_on = 1;
			}
			else {
				printf("No new Messages\n");
			}

			break;
		case 'q':
		{
			sendMsg(s, msgNumber, user, servinfo, 3);
			cout << "Sent Quit Message\n";
			cout << server_reply << endl;
		
			goto endProgram;
		}
		default:
			cin.sync();
			cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			break;
		}

		
	} while (true);

	endProgram:		// end program
	closesocket(s);
	WSACleanup();
	system("pause");

	return 0;
}
/* the listen function is used by the _beginthread and
    listens continually to the socket for incoming messages
*/
void listen(void * socket) {
	
		int recv_size;
		SOCKET s;
		s = *(SOCKET *)socket;
		 do {
			//while (listen_on == 1) {

				if ((recv_size = recvfrom(s, server_reply, 500, 0, NULL, NULL)) != SOCKET_ERROR) {
					listen_on = 0;
					server_reply[recv_size] = '\0';
					decryptMessage(recv_size);
				}


		}while (endFlag == 0); // if endFlag == 1 the listen functions returns

		
}
/* the sendMsg function is used to send all messages to the server.  as parameters is 
   takes the socket, message number user name struct addrinfo to communicate with the
   server.  message type is used in a switch statement to know what type of message
   to send.  
*/
void sendMsg(SOCKET s, int MsgNum, string user, struct addrinfo *p, int messageType) {
	char *message;
	string msg = "";
	string encryptedMessage = "";
	string encryptedUser = "";
	for (int i = 0; i < user.length(); i++) {
		encryptedUser += encrypt(user[i]);
	}

	switch (messageType) {
		case 1: {

			msg = to_string(MsgNum) + ";1;" + encryptedUser;
			message = new char[msg.length() + 1];
			strcpy(message, msg.c_str());

			if (sendto(s, message, strlen(message), 0, p->ai_addr, p->ai_addrlen) < 0) {
				cout << "Send Failed" << endl;
				return;
			}
			cout << "Data Sent" << endl;
			break;
	}
		case 2: {
			string buddy = "";
			string body = "";

			user = user + "\n";
			for (int i = 0; i < user.length(); i++)
				encryptedMessage += encrypt(user[i]);

			string msg = to_string(MsgNum) + ";2;" + encryptedMessage;

			encryptedMessage = "";

			printf("Enter your buddy name: \n");
			cin.sync();
			cin.ignore();
			getline(cin, buddy);
			buddy = buddy + "\n";
			for (int i = 0; i < buddy.length(); i++)
				encryptedMessage += encrypt(buddy[i]);

			msg = msg + encryptedMessage;
			encryptedMessage = "";

			printf("Enter message to be sent : \n");
			getline(cin, body);
			body = body + "\n";
			for (int i = 0; i < body.length(); i++)
				encryptedMessage += encrypt(body[i]);

			msg = msg + encryptedMessage;

			message = new char[msg.length() + 1];
			strcpy(message, msg.c_str());

			if (sendto(s, message, strlen(message), 0, p->ai_addr, p->ai_addrlen) < 0) {
				cout << "Send Failed" << endl;
			}
			else {
				cout << "Data Sent" << endl;
			}
			break;
		}
		case 3: {

			msg = to_string(MsgNum) + ";3;" + encryptedUser;
			message = new char[msg.length() + 1];
			strcpy(message, msg.c_str());

			if (sendto(s, message, strlen(message), 0, p->ai_addr, p->ai_addrlen) < 0) {
				cout << "Send Failed" << endl;
			}
			Sleep(500);
			endFlag = 1;
			break;
		}

		default:
			break;



	}
	
}
/* decryptMessage take the size of the received message and decrypts
   the message stored in server_reply
*/
void decryptMessage(int recv_size) {
	int check_msg = 0;
	int next =recv_size;
	for (int i = 0; i < recv_size; i++) {
		if (server_reply[i] == ';' || server_reply[i] == '.')
			check_msg += 1;
		if (check_msg == 2) {
			next = i + 1;
			break;
		}
	}
	for (int j = next; j < recv_size; j++)
		server_reply[j] = decrypt(server_reply[j]);
}
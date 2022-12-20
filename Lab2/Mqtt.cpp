// Server side C/C++ program to demonstrate Socket
// programming
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <string>
#include <list>
#include <pthread.h>


using namespace std;

#define PORT 1883

pthread_t thread_tcp [100], thread_file [10];
int threadno_tcp = 0;

string connectAck() {
	string message = "";
	unsigned char params = 0x20;
	unsigned char remainingBytes = 0x02;
	unsigned char session = 0x00;
	unsigned char result = 0x00;
	message.push_back(params);
	message.push_back(remainingBytes);
	message.push_back(session);
	message.push_back(result);
	return message;
}

string pingAck() {
	string message = "";
	unsigned char params = 0xd0;
	unsigned char remainingBytes = 0x00;
	message.push_back(params);
	message.push_back(remainingBytes);
	return message;
}

string subscribeAck(unsigned char packetID[]) {
	string message = "";
	unsigned char params = 0x90;
	unsigned char remainingBytes = 0x03;
	unsigned char returnCode = 0x00;
	message.push_back(params);
	message.push_back(remainingBytes);
	message.push_back(packetID[0]);
	message.push_back(packetID[1]);
	message.push_back(returnCode);
	return message;
}

string unsubscribeAck(unsigned char packetID[]) {
	string message = "";
	unsigned char params = 0xb0;
	unsigned char remainingBytes = 0x03;
	unsigned char returnCode = 0x00;
	message.push_back(params);
	message.push_back(remainingBytes);
	message.push_back(packetID[0]);
	message.push_back(packetID[1]);
	message.push_back(returnCode);
	return message;
}


void getControlPacketType(char* buffer, unsigned char* controlPacketType, int msgLen){
	for(int i = 0; i < msgLen; i++){
		controlPacketType[i] = buffer[i];
		//cout << hex << (int)controlPacketType[i];
	}
	//cout << endl;
} 

void getTopic(char* buffer, unsigned char* topic, unsigned char* controlPacketType){
	int topicLength = controlPacketType[3];
	//print topic 
	cout << "topic : ";
	for(int i = 4; i < topicLength + 4; i++){
		topic[i] = controlPacketType[i];
		cout << topic[i];
	}
	cout << endl;
}

void getMessage(char* buffer, unsigned char* message, unsigned char* controlPacketType){
	int topicLength = controlPacketType[3];
	//print msg
	cout << "message : ";
	for(int i = topicLength + 4; i < 2 + buffer[1]; i++){
		message[i] = controlPacketType[i];
		cout << message[i];
	}
	cout << endl;
}

int main(int argc, char const* argv[])
{
	int server_fd, new_socket, valread;
	struct sockaddr_in address;
	int opt = 1;
	int addrlen = sizeof(address);
	char buffer[1024] = { 0 };
	unsigned char controlPacketType[100] = {0};
	string response;
	unsigned char packetID[2] = {0};

	unsigned char topic[100] = {0};
	unsigned char message[1024] = {0};

	// Creating socket file descriptor
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket failed");
		exit(EXIT_FAILURE);
	}

	// Forcefully attaching socket to the port 8080
	if (setsockopt(server_fd, SOL_SOCKET,
				SO_REUSEADDR | SO_REUSEPORT, &opt,
				sizeof(opt))) {
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);

	// Forcefully attaching socket to the port 8080
	if (bind(server_fd, (struct sockaddr*)&address,
			sizeof(address))
		< 0) {
		perror("bind failed");
		exit(EXIT_FAILURE);
	}
	if (listen(server_fd, 3) < 0) {
		perror("listen");
		exit(EXIT_FAILURE);
	}

	if ((new_socket
			= accept(server_fd, (struct sockaddr*)&address,
					(socklen_t*)&addrlen))
			< 0) {
			perror("accept");
			exit(EXIT_FAILURE);
		} 

    while (1){

        valread = read(new_socket, buffer, 1024);
		getControlPacketType(buffer, controlPacketType, 2 + buffer[1]);
		cout << hex << (int)controlPacketType[0] << endl;
		
		switch (controlPacketType[0])
		{
		//connect
		case 0x10:
			response = connectAck();
			send(new_socket, response.c_str(), response.length(), 0);
			break;
		//ping
		case 0xc0:
			response = pingAck();
			send(new_socket, response.c_str(), response.length(), 0);
			break;
		//subscribe
		case 0x82:
			packetID[0] = controlPacketType[2];
			packetID[1] = controlPacketType[3];
			response = subscribeAck(packetID);
			send(new_socket, response.c_str(), response.length(), 0);
			break;
		//unsubscribe
		case 0xa2:
			packetID[0] = controlPacketType[2];
			packetID[1] = controlPacketType[3];
			response = unsubscribeAck(packetID);
			send(new_socket, response.c_str(), response.length(), 0);
			break;
		//publish
		case 0x30:
			getTopic(buffer, topic, controlPacketType);
			getMessage(buffer, message, controlPacketType);
			//Todo : add the message to the topic list and send it to the other clients
			break;
		//disconnect
		case 0xe0:
			close(new_socket);
			valread = 0;
			buffer[0] = '\0';
			break;
		default:
			break;
		}	
    }
	return 0;
}



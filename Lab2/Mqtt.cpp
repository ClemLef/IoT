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

using namespace std;

#define PORT 1883


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
	cout << hex << message << endl;
	return message;
}

string pingAck() {
	string message = "";
	unsigned char params = 0xd0;
	unsigned char remainingBytes = 0x02;
	unsigned char session = 0x00;
	unsigned char result = 0x00;
	message.push_back(params);
	message.push_back(remainingBytes);
	message.push_back(session);
	message.push_back(result);
	cout << hex << message << endl;
	return message;
}


void getControlPacketType(char* buffer, unsigned char* controlPacketType){
	controlPacketType[0] = buffer[0];
	 /* for(int i = 0; i < 1; i++){
			cout << hex << (int)controlPacketType[i];
		}
		cout << endl; */
} 

int main(int argc, char const* argv[])
{
	int server_fd, new_socket, valread;
	struct sockaddr_in address;
	int opt = 1;
	int addrlen = sizeof(address);
	char buffer[1024] = { 0 };
	unsigned char controlPacketType[1] = {0};
	string response;

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
		getControlPacketType(buffer, controlPacketType);
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
		//disconnect
		case 0xe0:
			close(new_socket);
			break;
		default:
			break;
		}	
    }
	return 0;
}



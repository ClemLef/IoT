// Client side implementation of UDP client-server model
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <iostream>

using namespace std;
	
#define PORT	5683
#define MAXLINE 1024


string getHeaders(string buffer){
	string header;
	header = buffer.substr(0, buffer.find(0b11111111));
	//cout << header << endl;
	return header;
}

string getContent(string buffer){
	string content;
	content = buffer.substr(1 + buffer.find(0b11111111));
	//cout << content << endl;
	return content;
}

string get() {
	// Declaring parameters
	string message = "";
    unsigned char settings = 0b01010000;
    unsigned char keyword = 0b00000001;
    unsigned char msgId[] = {0b10101010, 0b01010101};
	unsigned char uriOption = 0b00110111;
	unsigned char uri[] = {0b01100011,0b01101111,0b01100001,0b01110000,0b00101110,0b01101101,0b01100101};
	unsigned char pathOption = 0b10000100;
	unsigned char path[] = {0b01110100, 0b01100101, 0b01110011, 0b01110100};

	// Forming a message based on the parameters
	message.push_back(settings);
	message.push_back(keyword);
	message.push_back(msgId[0]);
	message.push_back(msgId[1]);
	message.push_back(uriOption);
	for (int i = 0; i < 7; i++){
		message.push_back(uri[i]);
	}
	message.push_back(pathOption);
	for (int i = 0; i < 4; i++){
		message.push_back(path[i]);
	}
}

	
// Driver code
int main() {
	int sockfd;
	char buffer[MAXLINE];
	struct sockaddr_in servaddr;
	unsigned int n, len;

	// Create a socket file descriptor
	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
		perror("socket creation failed");
		exit(EXIT_FAILURE);
	}
	
	memset(&servaddr, 0, sizeof(servaddr));
		
	// CoAP server network info (134.102.218.18 is coap.me IP)
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(PORT);
	servaddr.sin_addr.s_addr =  inet_addr("134.102.218.18");
	
	string message = get();
	
	// Sending the message to the test server
	sendto(sockfd, message.c_str(), message.length(), MSG_CONFIRM, (const struct sockaddr *) &servaddr, sizeof(servaddr));
        //printf("Hello message sent.\n");
	
    n = recvfrom(sockfd, (char *)buffer, MAXLINE,
            MSG_WAITALL, (struct sockaddr *) &servaddr,
            &len);
    buffer[n] = '\0';

    cout << getHeaders(buffer) << endl;
	cout << getContent(buffer) << endl;
	
	close(sockfd);
	return 0;
}

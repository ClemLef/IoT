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
#include <bitset>
#include <netdb.h>

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

// not the best solution
char pathOptions(int size){
	switch (size)
	{
	case 4:
		return 0b10000100;
		break;
	case 5:
		return 0b10000101;
		break;
	default:
		break;
	}
}

string randomMsgId(){
	char randomId[] = {rand() % 255 + 1,rand() % 255 + 1};
	return randomId;
}

string get(string path) {
	// Declaring parameters
	string message = "";

	int size = path.length();

    unsigned char settings = 0b01010000;
    unsigned char method = 0b00000001;
	// TODO : randomize message id
    //unsigned char msgId[] = {0b10111010, 0b01010101};
	string msgId = randomMsgId();
	unsigned char uriOption = 0b00110111;
	unsigned char uri[] = {0b01100011,0b01101111,0b01100001,0b01110000,0b00101110,0b01101101,0b01100101};
	unsigned char pathOption = pathOptions(size);
	// test 
	//unsigned char path[] = {0b01110100, 0b01100101, 0b01110011, 0b01110100};
	// sink unsigned char path[] = {0b01110011,0b01101001,0b01101110,0b01101011,0b00001010};

	// Forming a message based on the parameters
	message.push_back(settings);
	message.push_back(method);
	message += msgId;
	//message.push_back(msgId[0]);
	//message.push_back(msgId[1]);
	message.push_back(uriOption);
	for (int i = 0; i < 7; i++){
		message.push_back(uri[i]);
	}
	message.push_back(pathOption);
	message += path;
	
	return message;
}



string post(string input, string path) {
	// Declaring parameters
	string message = "";
	
	int size = path.length();

    unsigned char settings = 0b01010000;
    unsigned char method = 0b00000010;
	// TODO : randomize message id
    //unsigned char msgId[] = {0b10111110, 0b01010101};
	string msgId = randomMsgId();
	unsigned char uriOption = 0b00110111;
	unsigned char uri[] = {0b01100011,0b01101111,0b01100001,0b01110000,0b00101110,0b01101101,0b01100101};
	unsigned char pathOption = pathOptions(size) ;
	
	//unsigned char path[] = {0b01110011,0b01101001,0b01101110,0b01101011,0b00001010};
	unsigned char payloadOption = 0b00010000;
	unsigned char separator = 0b11111111;
	unsigned char payload[] = {0b00110100,0b00110010};

	// Forming a message based on the parameters
	message.push_back(settings);
	message.push_back(method);
	message.push_back(msgId[0]);
	message.push_back(msgId[1]);
	message.push_back(uriOption);
	for (int i = 0; i < 7; i++){
		message.push_back(uri[i]);
	}
	message.push_back(pathOption);
	message += path;
	message.push_back(payloadOption);
	message.push_back(separator);
	message += input;
	return message;
}

string del(string path){
	// Declaring parameters
	string message = "";

	int size = path.length();

    unsigned char settings = 0b01010000;
    unsigned char method = 0b00000100;
	// TODO : randomize message id
    //unsigned char msgId[] = {0b10111110, 0b01010101};
	string msgId = randomMsgId();
	unsigned char uriOption = 0b00110111;
	unsigned char uri[] = {0b01100011,0b01101111,0b01100001,0b01110000,0b00101110,0b01101101,0b01100101};
	unsigned char pathOption = pathOptions(size);
	//unsigned char path[] = {0b01110011,0b01101001,0b01101110,0b01101011,0b00001010};
	
	// Forming a message based on the parameters
	message.push_back(settings);
	message.push_back(method);
	message.push_back(msgId[0]);
	message.push_back(msgId[1]);
	message.push_back(uriOption);
	for (int i = 0; i < 7; i++){
		message.push_back(uri[i]);
	}
	message.push_back(pathOption);
	message += path;
	
	return message;
}

string put(string input, string path){
	// Declaring parameters
	string message = "";
	
	int size = path.length();

    unsigned char settings = 0b01010000;
    unsigned char method = 0b00000011;
	// TODO : randomize message id
    //unsigned char msgId[] = {0b10111110, 0b01010101};
	string msgId = randomMsgId();
	unsigned char uriOption = 0b00110111;
	unsigned char uri[] = {0b01100011,0b01101111,0b01100001,0b01110000,0b00101110,0b01101101,0b01100101};
	unsigned char pathOption = pathOptions(size) ;
	
	//unsigned char path[] = {0b01110011,0b01101001,0b01101110,0b01101011,0b00001010};
	unsigned char payloadOption = 0b00010000;
	unsigned char separator = 0b11111111;
	unsigned char payload[] = {0b00110100,0b00110010};

	// Forming a message based on the parameters
	message.push_back(settings);
	message.push_back(method);
	message.push_back(msgId[0]);
	message.push_back(msgId[1]);
	message.push_back(uriOption);
	for (int i = 0; i < 7; i++){
		message.push_back(uri[i]);
	}
	message.push_back(pathOption);
	message += path;
	message.push_back(payloadOption);
	message.push_back(separator);
	message += input;
	return message;
}

void sendRequest(int sockfd, sockaddr_in servaddr, string message, char* buffer){
	unsigned int n, len;
	// Sending the message to the test server
	sendto(sockfd, message.c_str(), message.length(), MSG_CONFIRM, (const struct sockaddr *) &servaddr, sizeof(servaddr));
        //printf("Hello message sent.\n");
	
    n = recvfrom(sockfd, (char *)buffer, MAXLINE,
            MSG_WAITALL, (struct sockaddr *) &servaddr,
            &len);
    buffer[n] = '\0';

    //cout << getHeaders(buffer) << endl;
	cout << getContent(buffer) << endl;
}



// Driver code
int main() {
	int sockfd;
	char buffer[MAXLINE];
	struct sockaddr_in servaddr;
	unsigned int n, len;
	string message, input, path;
	int choice = -1;
	struct addrinfo hints, *result, *rp;

	// Create a socket file descriptor
	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
		perror("socket creation failed");
		exit(EXIT_FAILURE);
	}
	
	memset (&servaddr, 0, sizeof(servaddr));

	memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
    hints.ai_flags = 0;
    hints.ai_protocol = 0;          /* Any protocol */
		
	// CoAP server network info (134.102.218.18 is coap.me IP)
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(PORT);
	//servaddr.sin_addr.s_addr =  inet_addr("134.102.218.18");
	//servaddr.sin_addr.s_addr = 
	
	getaddrinfo("coap.me", NULL, &hints, &result);
	for (rp = result; rp != NULL; rp = rp->ai_next) {
		cout << rp->ai_addr;
	}
	while(choice != 0){
		cout << endl;
		cout << "*------- Menu -------*" << endl;
		cout << "  1. Send a GET" << endl;
		cout << "  2. Send a POST" << endl;
		cout << "  3. Send a PUT" << endl;
		cout << "  4. Send a DELETE" << endl;
		cout << "  0. Quit" << endl;
		cout << "*--------------------*" << endl;
		cout << "Make your choice : ";
		cin >> choice;
		system("clear");
		switch (choice)
		{
		case 0:
			break;
		case 1:
			cout << "Enter the path to display : ";
			cin >> path;
			message = get(path);
			sendRequest(sockfd, servaddr, message, buffer);
			break;
		case 2:
			cout << "Enter the value you want to send : ";
			cin >> input;
			cout << "Enter the path : ";
			cin >> path;
			message = post(input, path);
			cout << "Status : ";
			sendRequest(sockfd, servaddr, message, buffer);
			message = get(path);
			cout << "Contents : ";
			sendRequest(sockfd, servaddr, message, buffer);
			break;
		case 3:
			cout << "Enter the value you want to send : ";
			cin >> input;
			cout << "Enter the path : ";
			cin >> path;
			message = put(input, path);
			cout << "Status : ";
			sendRequest(sockfd, servaddr, message, buffer);
			message = get(path);
			cout << "Contents : ";
			sendRequest(sockfd, servaddr, message, buffer);
			break;
		case 4:
			cout << "Enter the path to delete : ";
			cin >> path;
			message = del(path);
			sendRequest(sockfd, servaddr, message, buffer);
			break;
		default:
			cout << "Choose a number between 0 and 5" << endl;
			break;
		}
	}

	return 0;
}

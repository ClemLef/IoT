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

// define coap port
#define PORT	5683
// define size of the buffer
#define MAXLINE 2048

// Function getting the headers of the response
string getHeaders(string buffer){
	string header;
	// split received message and get the part from the beginning to the payload separator (11111111)
	header = buffer.substr(0, buffer.find(0b11111111));
	return header;
}

// Function getting the contents of the response
string getContent(string buffer){
	string content;
	// split received message and get the part from the the payload separator (11111111) + 1 to the end 
	content = buffer.substr(1 + buffer.find(0b11111111));
	return content;
}

// convert the size as integer to a binary value for the payload options
char pathOptions(int size){
	switch (size)
	{
	case 4:
		return 0b10000100;
		break;
	case 5:
		return 0b10000101;
		break;
	case 6:
		return 0b10000110;
		break;
	case 7:
		return 0b10000111;
		break;
	default:
		return 0;
		break;
	}
}

// Generating a random message ID
string randomMsgId(){
	char randomId[] = {rand() % 255 + 1, rand() % 255 + 1};
	return randomId;
}

string get(string path) {
	// Declaring parameters
	string message = "";
	// Get the path length
	int size = path.length();

	// Declaring parameters as bits
	// Settings represent Version (01), Type (01) and Token length (0000)
    unsigned char settings = 0b01010000;
	// Method respresent the method used (GET = 0.01)
    unsigned char method = 0b00000001;
    // Generate a random message ID
	string msgId = randomMsgId();
	// uriOption contains the parameters for the host (3 = 0011 for uri-host and 7 = 0111 for size of coap.me)
	unsigned char uriOption = 0b00110111;
	// uri is coap.me in binary
	unsigned char uri[] = {0b01100011,0b01101111,0b01100001,0b01110000,0b00101110,0b01101101,0b01100101};
	// pathOption contains the parameters for the path (8 = 1000 for Location-path and size is defined with the switch)
	unsigned char pathOption = pathOptions(size);

	// Forming a message based on the parameters
	message.push_back(settings);
	message.push_back(method);
	message += msgId;
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
	// Get the path length
	int size = path.length();

	// Declaring parameters as bits
	// Settings represent Version (01), Type (01) and Token length (0000)
    unsigned char settings = 0b01010000;
	// Method respresent the method used (POST = 0.02)
    unsigned char method = 0b00000010;
	// Generate a random message ID
	string msgId = randomMsgId();
	// uriOption contains the parameters for the host (3 = 0011 for uri-host and 7 = 0111 for size of coap.me)
	unsigned char uriOption = 0b00110111;
	// uri is coap.me in binary
	unsigned char uri[] = {0b01100011,0b01101111,0b01100001,0b01110000,0b00101110,0b01101101,0b01100101};
	// pathOption contains the parameters for the path (8 = 1000 for Location-path and size is defined with the switch)
	unsigned char pathOption = pathOptions(size) ;
	// payloadOption contains the parameters for the payload (1 = 0001 and 0 = 0000 for length)
	unsigned char payloadOption = 0b00010000;
	// separate header and payload with 11111111
	unsigned char separator = 0b11111111;

	// Forming a message based on the parameters
	message.push_back(settings);
	message.push_back(method);
	message += msgId;
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
	// Get the path length
	int size = path.length();

	// Declaring parameters as bits
	// Settings represent Version (01), Type (01) and Token length (0000)
    unsigned char settings = 0b01010000;
	// Method respresent the method used (DELETE = 0.04)
    unsigned char method = 0b00000100;
	// Generate a random message ID
	string msgId = randomMsgId();
	// uriOption contains the parameters for the host (3 = 0011 for uri-host and 7 = 0111 for size of coap.me)
	unsigned char uriOption = 0b00110111;
	// uri is coap.me in binary
	unsigned char uri[] = {0b01100011,0b01101111,0b01100001,0b01110000,0b00101110,0b01101101,0b01100101};
	// pathOption contains the parameters for the path (8 = 1000 for Location-path and size is defined with the switch)
	unsigned char pathOption = pathOptions(size);
	
	// Forming a message based on the parameters
	message.push_back(settings);
	message.push_back(method);
	message += msgId;
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
	// Get the path length
	int size = path.length();

	// Declaring parameters as bits
	// Settings represent Version (01), Type (01) and Token length (0000)
    unsigned char settings = 0b01010000;
	// Method respresent the method used (PUT = 0.03)
    unsigned char method = 0b00000011;
	// Generate a random message ID
	string msgId = randomMsgId();
	// uriOption contains the parameters for the host (3 = 0011 for uri-host and 7 = 0111 for size of coap.me)
	unsigned char uriOption = 0b00110111;
	// uri is coap.me in binary
	unsigned char uri[] = {0b01100011,0b01101111,0b01100001,0b01110000,0b00101110,0b01101101,0b01100101};
	// pathOption contains the parameters for the path (8 = 1000 for Location-path and size is defined with the switch)
	unsigned char pathOption = pathOptions(size) ;
	// payloadOption contains the parameters for the payload (1 = 0001 and 0 = 0000 for length)
	unsigned char payloadOption = 0b00010000;
	// separate header and payload with 11111111
	unsigned char separator = 0b11111111;

	// Forming a message based on the parameters
	message.push_back(settings);
	message.push_back(method);
	message += msgId;
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
	unsigned int n = 0;
	unsigned int len = 0;
	// Sending the message to the test server
	sendto(sockfd, message.c_str(), message.length(), MSG_CONFIRM, (const struct sockaddr *) &servaddr, sizeof(servaddr));
	
    n = recvfrom(sockfd, (char *)buffer, MAXLINE + 1,
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

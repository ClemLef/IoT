
/* 
   Clement LEFEBVRE / Implementing IoT protocols
   Code created for lab 3 : Project
   Simple code to send GET/POST/PUT/DELETE messages to a CoAP server
*/

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
#define MAXLINE 1024

// Function getting the headers of the response
string getHeaders(string buffer){
	string header;
	// split received message and get the part from the beginning to the payload separator (11111111)
	header = buffer.substr(0, buffer.find(0b11111111));
	return header;
}

// Function getting the contents of the response
string getContent(char* buffer, int bufferSize){
	string content;
	for(int i = 8; i < bufferSize; i++){
		content += buffer[i];
	}
	return content;
}

// Generating a random message ID
string randomMsgId(){
	char randomId[] = {char(rand() % 255 + 1), char(rand() % 255 + 1)};
	return randomId;
}

string get(string path, string ip) {
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
	// Forming a message based on the parameters
	message.push_back(settings);
	message.push_back(method);
	message += msgId;
	// 16+32+128 correspond to the bits we need to set to configure the option URI-path (11)
	message.push_back(16 + 32 + 128 + size);
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
	// payloadOption contains the parameters for the payload (1 = 0001 and 0 = 0000 for length)
	unsigned char payloadOption = 0b00010000;
	// separate header and payload with 11111111
	unsigned char separator = 0b11111111;

	// Forming a message based on the parameters
	message.push_back(settings);
	message.push_back(method);
	message += msgId;
	message.push_back(16 + 32 + 128 + size);
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
	
	// Forming a message based on the parameters
	message.push_back(settings);
	message.push_back(method);
	message += msgId;
	message.push_back(16 + 32 + 128 + size);
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
	// payloadOption contains the parameters for the payload (1 = 0001 and 0 = 0000 for length)
	unsigned char payloadOption = 0b00010000;
	// separate header and payload with 11111111
	unsigned char separator = 0b11111111;

	// Forming a message based on the parameters
	message.push_back(settings);
	message.push_back(method);
	message += msgId;
	message.push_back(16 + 32 + 128 + size);
	message += path;
	message.push_back(payloadOption);
	message.push_back(separator);
	message += input;
	return message;
}

// Function used to send requests to send a request to a coap server
void sendRequest(int sockfd, sockaddr_in servaddr, string message, char *buffer){
	// Size of the response
	unsigned int n = 0;
	// Size of the server address
	unsigned int len = 0;
	// Sending the message to the test server
	cout << "sending" << endl;
	cout << sendto(sockfd, message.c_str(), message.length(), MSG_CONFIRM, (const struct sockaddr *) &servaddr, sizeof(servaddr));
	cout << endl;
	// Get the response from the server
    n = recvfrom(sockfd, buffer, MAXLINE + 1,
            MSG_WAITALL, (struct sockaddr *) &servaddr,
            &len);
	cout << n << endl;
	// Closing the answer at the end
    buffer[n] = '\0';

	// Printing the headers and contents
    //cout << getHeaders(buffer) << endl;
	cout << getContent(buffer, n) << endl;
}

// Main function of the program
int main() {
	// Socket file descriptor
	int sockfd;
	// Buffer to store the response
	char buffer[MAXLINE];
	// Address of the server
	struct sockaddr_in servaddr;
	// message to send the request, input of the user, path the user wants to access
	string message, input, path;
	// Variable determining the choice of the user
	int choice = -1;
	string ip;

	// Create a socket file descriptor
	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
		perror("socket creation failed");
		exit(EXIT_FAILURE);
	}
	
	// Reserve memory to store the server address
	memset(&servaddr, 0, sizeof(servaddr));

	//cout << "Enter the ip address of the sensor : ";
	ip = "192.168.137.102";
		
	// CoAP server network info
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(PORT);
	servaddr.sin_addr.s_addr = inet_addr(ip.c_str());
	
	// Loop asking the user an action to realise
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
		// Clearing the console after a choice is made
		system("clear");
		switch (choice)
		{
		case 0:
			// Quit the app
			break;
		case 1:
			// Ask for a path and send get request
			cout << "Enter the path to display : ";
			cin >> path;
			message = get(path, ip);
			sendRequest(sockfd, servaddr, message, buffer);
			break;
		case 2:
			// Ask for a value and a path and sends a request
			cout << "Enter the value you want to send : ";
			cin >> input;
			cout << "Enter the path : ";
			cin >> path;
			message = post(input, path);
			// Print the status of the command
			cout << "Status : ";
			sendRequest(sockfd, servaddr, message, buffer);
			// Display the contents of the path using a get
			message = get(path, ip);
			cout << "Contents : ";
			sendRequest(sockfd, servaddr, message, buffer);
			break;
		case 3:
			// Ask for a value and a path and sends a request
			cout << "Enter the value you want to send : ";
			cin >> input;
			cout << "Enter the path : ";
			cin >> path;
			message = put(input, path);
			// Print the status of the command
			cout << "Status : ";
			sendRequest(sockfd, servaddr, message, buffer);
			// Display the contents of the path using a get
			message = get(path, ip);
			cout << "Contents : ";
			sendRequest(sockfd, servaddr, message, buffer);
			break;
		case 4:
			// Ask for a path and send delete request
			cout << "Enter the path to delete : ";
			cin >> path;
			message = del(path);
			sendRequest(sockfd, servaddr, message, buffer);
			break;
		default:
			// a random value was choosen
			cout << "Choose a number between 0 and 5" << endl;
			break;
		}
	}

	return 0;
}

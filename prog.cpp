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

#define PORT 8080


string getAction(string buffer) {
    string action;
    action = buffer.substr(0, buffer.find(" "));
    //cout << action << endl;
    return action;
}

string getParams(string buffer){
	string param;
	param = buffer.substr(buffer.find("\r\n\r\n"));
	//cout << param << endl;
    return param;
}

void printList(list<string> list){
    for (string x : list) {
		cout << x  << endl;
	}
	cout << endl;
}

bool inList(list<string> list, string element) {
	for (string x : list) {
		if(x == element){
			return true;
		}
	}
	return false;
}

int main(int argc, char const* argv[])
{
	int server_fd, new_socket, valread;
	struct sockaddr_in address;
	int opt = 1;
	int addrlen = sizeof(address);
	char buffer[1024] = { 0 };
	//char* hello = "Hello from server";
   	list<string> sensors;
    string sensorValue = "sensor value = 42";

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
	
    while (1){

		if ((new_socket
				= accept(server_fd, (struct sockaddr*)&address,
						(socklen_t*)&addrlen))
				< 0) {
				perror("accept");
				exit(EXIT_FAILURE);
		}

        valread = read(new_socket, buffer, 1024);
	    cout << buffer << endl;
        string action = getAction(buffer);
        if (action.compare("GET") == 0){
			char* response = "HTTP/1.1 200 OK \r\n Content-Type: text/html \r\n\r\n sensor value = 42";
            send(new_socket, response, strlen(response), 0);
			close(new_socket);
        } else if (action.compare("POST") == 0){
            string param = getParams(buffer);
			//1 + to prevent the = being taken in 
			string sensorName = param.substr(1 + param.find("="));
			//printList(sensors);
			cout << inList(sensors, sensorName) << endl;
			if (inList(sensors, sensorName) == 0){
				sensors.push_back(sensorName);
				printList(sensors);
				send(new_socket, "HTTP/1.1 201 OK \r\n Content-Type: text/html \r\n\r\n OK", strlen("HTTP/1.1 201 OK \r\n Content-Type: text/html \r\n\r\n OK"), 0);
				close(new_socket);
			} else {
				send(new_socket, "HTTP/1.1 409 \r\n Content-Type: text/html \r\n\r\n sensor already exists", strlen("HTTP/1.1 409 \r\n Content-Type: text/html \r\n\r\n sensor already exists"), 0);
				close(new_socket);
			}
			// closing the connected socket
			close(new_socket);
        }
        
    }
	return 0;
}



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

typedef struct
{
	int sock;
	struct sockaddr address;
	socklen_t addr_len;
} connection_t;



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

void getTopic(char* buffer, unsigned char* topic){
	int topicLength = buffer[3];
	//print topic 
	cout << "topic : ";
	for(int i = 4; i < topicLength + 4; i++){
		topic[i] = buffer[i];
		cout << topic[i];
	}
	cout << endl;
}

void getMessage(char* buffer, unsigned char* message){
	int topicLength = buffer[3];
	//print msg
	cout << "message : ";
	for(int i = topicLength + 4; i < 2 + buffer[1]; i++){
		message[i] = buffer[i];
		cout << message[i];
	}
	cout << endl;
}

void * process(void * ptr)
{
    char buffer[1024] = {0};
    connection_t * conn;
    long addr = 0;
	string response;
	unsigned char controlPacketType;
	unsigned char packetID[2] = {0};

	unsigned char topic[100] = {0};
	unsigned char message[1024] = {0};

	if (!ptr) pthread_exit(0);
    conn = (connection_t *)ptr;

	cout << hex << (int)buffer[0] << endl;
	while (buffer[0] != 0xe0){
		//addr = (long)((struct sockaddr_in *)&conn->address)->sin_addr.s_addr;
		//buffer = (char *)malloc((len+1)*sizeof(char));
		//buffer[len] = 0;
		/* read message */
		read(conn->sock, buffer, 1024);
		/* print message */
		/*printf("%d.%d.%d.%d:\n",
			(addr      ) & 0xff,
			(addr >>  8) & 0xff,
			(addr >> 16) & 0xff,
			(addr >> 24) & 0xff,
			);*/
		//cout << len << endl;
		for(int i = 0; i < 200; i++){
			cout << hex << (int)buffer[i];
		}
		cout << endl;
		cout << hex << (int)buffer[0] << endl;
		controlPacketType = buffer[0];
		switch (controlPacketType)
		{
		//connect
		case 0x10:
			response = connectAck();
			send(conn->sock, response.c_str(), response.length(), 0);
			break;
		//ping
		case 0xc0:
			response = pingAck();
			send(conn->sock, response.c_str(), response.length(), 0);
			break;
		//subscribe
		case 0x82:
			packetID[0] = buffer[2];
			packetID[1] = buffer[3];
			response = subscribeAck(packetID);
			send(conn->sock, response.c_str(), response.length(), 0);
			break;
		//unsubscribe
		case 0xa2:
			packetID[0] = buffer[2];
			packetID[1] = buffer[3];
			response = unsubscribeAck(packetID);
			send(conn->sock, response.c_str(), response.length(), 0);
			break;
		//publish
		case 0x30:
			getTopic(buffer, topic);
			getMessage(buffer, message);
			//Todo : add the message to the topic list and send it to the other clients
			break;
		//disconnect
		case 0xe0:
			close(conn->sock);
			break;
		default:
			break;
		}
	}
	/* close socket and clean up */
	free(buffer);
    close(conn->sock);
    free(conn);
    pthread_exit(0);
}

int main(int argc, char const* argv[])
{
	int sock = -1;
    struct sockaddr_in address;
    connection_t * connection;
    pthread_t thread;

	/* create socket */
    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock <= 0)
    {
        fprintf(stderr, "%s: error: cannot create socket\n", argv[0]);
        return -3;
    }

	/* bind socket to port */
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    if (bind(sock, (struct sockaddr *)&address, sizeof(struct sockaddr_in)) < 0)
    {
        fprintf(stderr, "%s: error: cannot bind socket to port %d\n", argv[0], PORT);
        return -4;
    }

	/* listen on port */
    if (listen(sock, 5) < 0)
    {
        fprintf(stderr, "%s: error: cannot listen on port\n", argv[0]);
        return -5;
    }

	printf("%s: ready and listening\n", argv[0]);

    while (1){

		/* accept incoming connections */
        connection = (connection_t *)malloc(sizeof(connection_t));
        connection->sock = accept(sock, &connection->address, &connection->addr_len);
        if (connection->sock <= 0)
        {
            free(connection);
        }
        else
        {
            /* start a new thread but do not wait for it */
            pthread_create(&thread, 0, process, (void *)connection);
            pthread_detach(thread);
        }



        /*valread = read(new_socket, buffer, 1024);
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
		}	*/
    }
	return 0;
}



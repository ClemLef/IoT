// Server side C/C++ program to demonstrate Socket
// programming
#include <list>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <string>
#include <pthread.h>
#include <vector>
#include <map>


using namespace std;

#define PORT 1883

typedef struct
{
	int sock;
	struct sockaddr address;
	socklen_t addr_len;
} connection_t;

map<string, list<int>> topicSockets;

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

string publish(unsigned char packetID[], string topic, string payload) {
	string message = "";
	unsigned char params = 0x30;
	unsigned char remainingBytes = sizeof(topic) + sizeof(payload);
	unsigned char topicLength[2];
	topicLength[0] = sizeof(topic) / 256;
	topicLength[1] = sizeof(topic) % 256;
	cout << "rb " << (int)remainingBytes << endl;
	message.push_back(params);
	message.push_back(remainingBytes);
	message.push_back(topicLength[0]);
	message.push_back(topicLength[1]);
	message += topic;
	message.push_back(packetID[0]);
	message.push_back(packetID[1]);
	message += payload;
	return message;
}


string getTopic(char* buffer, int topicLength, string pubSub){
	//print topic 
	string topic;
	int position;
	if(pubSub == "publish"){
		position = 4;
	} else {
		position = 6;
	}
	//cout << topicLength << endl;
	for(int i = position; i < position + topicLength; i++){
		topic += buffer[i];
	}
	//cout << "topic : " << topic << endl;
	return topic;
}

string getMessage(char* buffer){
	string message;
	int topicLength = buffer[3];
	//print msg
	//cout << "message : ";
	for(int i = topicLength + 4; i < 2 + buffer[1]; i++){
		message += buffer[i];
		//cout << message[i];
	}
	//cout << endl;
	return message;
}

void addToMap(string topic, int sock){
	list<int> sockList;
	// If topic is not in the map : 
	if(topicSockets.find(topic) != topicSockets.end()){
		sockList = topicSockets.find(topic)->second;
		sockList.push_back(sock);
		cout << "socklist" << endl;
		for (auto const &i: sockList) {
			std::cout << i << std::endl;
		}
		topicSockets.find(topic)->second = sockList;
	} else {
		sockList.push_back(sock);
		cout << "socklist2" << endl;
		for (auto const &i: sockList) {
			std::cout << i << std::endl;
		}
		topicSockets.insert({topic, sockList});
	}
}

void sendPublish(string message, string topic, unsigned char* packetID, list<int> sockList, int socket){
	//Todo : add the message to the topic list and send it to the other clients
	string response;
	if(topicSockets.find(topic) != topicSockets.end()){
		sockList = topicSockets.find(topic)->second;
		response = publish(packetID, topic, message);
		cout << "sending to " << endl;
		for (auto const &i: sockList) {
			std::cout << i<< std::endl;
			if(i != socket){
				send(i, response.c_str(), response.length(), 0);
			}
		}
	}

}

void * process(void * ptr)
{
    char buffer[1024] = {0};
    connection_t * conn;
    long addr = 0;
	string response;
	int topicLength;
	unsigned char controlPacketType;
	unsigned char packetID[2] = {0};
	list<int> sockList = {0};

	string topic;
	string message;

	if (!ptr) pthread_exit(0);
    conn = (connection_t *)ptr;

	//cout << hex << (int)buffer[0] << endl;
	while (buffer[0] != 0xffffffe0){
		//cout << hex << (int)buffer[0] << endl;
		addr = (long)((struct sockaddr_in *)&conn->address)->sin_addr.s_addr;
		
		/* read message */
		read(conn->sock, buffer, 1024);
		/*printf("%d.%d.%d.%d:\n",
			(addr      ) & 0xff,
			(addr >>  8) & 0xff,
			(addr >> 16) & 0xff,
			(addr >> 24) & 0xff,
			);*/
		/*for(int i = 0; i < 200; i++){
			cout << hex << (int)buffer[i];
		}
		cout << endl;*/
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
		//améliorer la gestion de l'id
			packetID[0] = buffer[2];
			packetID[1] = buffer[3];
			//todo use the two bytes for the topic length
			topicLength = (int)buffer[5] + (int)buffer[4];
			topic = getTopic(buffer, topicLength, "subscribe");
			addToMap(topic, conn->sock);
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
			packetID[0] = buffer[2];
			packetID[1] = buffer[3];
			topicLength = (int)buffer[3] + (int)buffer[2];
			topic = getTopic(buffer, topicLength, "publish");
			message = getMessage(buffer);
			sendPublish(message, topic, packetID, sockList, conn->sock);
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

	map<string, vector<connection_t>> topicSockets;

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


    }
	return 0;
}



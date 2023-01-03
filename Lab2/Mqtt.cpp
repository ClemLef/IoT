// Server side C/C++ program to demonstrate Socket
// programming
#include <list>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <pthread.h>
#include <map>

using namespace std;

// MQTT port is 1883 by default
#define PORT 1883

// struct used for saving the details of the connection 
typedef struct
{
	int sock;
	struct sockaddr address;
	socklen_t addr_len;
} connection_t;

// global maps that are used to save the subscribed sockets and retained messages 
map<string, list<int>> topicSockets;
map<string, string> topicRetain;

// creates the connect acknowledgment message
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

// creates the ping acknowledgment message
string pingAck() {
	string message = "";
	unsigned char params = 0xd0;
	unsigned char remainingBytes = 0x00;
	message.push_back(params);
	message.push_back(remainingBytes);
	return message;
}

// creates the subscribe acknowledgment message
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

// creates the unsubscribe acknowledgment message
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

// creates the publish message
string publish(string topic, string payload) {
	string message = "";
	unsigned char params = 0x30;
	unsigned char remainingBytes;
	unsigned char topicLength[2];

	// gets the topic size as int and separate them as two bytes
	int topicSize = topic.size();
	topicLength[0] = topicSize / 256;
	topicLength[1] = topicSize % 256;

	// gets the size of the payload
	int payloadSize = payload.size();

	// calculates the overall size of the message
	remainingBytes = topicSize + payloadSize + 2;

	message.push_back(params);
	message.push_back(remainingBytes);
	message.push_back(topicLength[0]);
	message.push_back(topicLength[1]);
	message += topic;
	message += payload;
	return message;
}

// extracts the topic from the buffer
string getTopic(char* buffer, int topicLength, string pubSub){ 
	string topic;
	int position;

	// the topic is either starting at byte 4 for the publish message or 6 for subscribe/unsubscribe
	if(pubSub == "publish"){
		position = 4;
	} else {
		position = 6;
	}

	for(int i = position; i < position + topicLength; i++){
		topic += buffer[i];
	}

	return topic;
}

// extracts the message from the buffer
string getMessage(char* buffer, int topicLength){
	string message;
	// topicLength is used to get the starting point of the message inside the buffer
	// the loop ends at packetLength + 2 (2 is the two bytes at the start not counted by remainingLength)
	for(int i = topicLength + 4; i < 2 + buffer[1]; i++){
		message += buffer[i];
	}
	return message;
}

void removeFromMap(string topic, int sock){
	// internal socket list used to remove a socket from the list 
	list<int> sockList;
	// If topic is in the map : 
	if(topicSockets.find(topic) != topicSockets.end()){
		// gets the list of sockets from the map
		sockList = topicSockets.find(topic)->second;
		// removes the disconnected socket
		sockList.remove(sock);
		// puts back the socketlist without the disconnected client
		topicSockets.find(topic)->second = sockList;
	}
}

void addToMap(string topic, int sock){
	// internal socket list used to add a socket to the list 
	list<int> sockList;
	// If topic is in the map : 
	if(topicSockets.find(topic) != topicSockets.end()){
		// gets the list of sockets from the map
		sockList = topicSockets.find(topic)->second;
		// adds the new connected socket
		sockList.push_back(sock);
		// puts back the socketlist with the new connection
		topicSockets.find(topic)->second = sockList;
	} else {
		// if topic is not in the map
		sockList.push_back(sock);
		topicSockets.insert({topic, sockList});
	}
}

void sendPublish(string message, string topic, list<int> sockList, int socket){
	string response;
	
	// If topic is in the map
	if(topicSockets.find(topic) != topicSockets.end()){
		// gets all the sockets subscribed to the topic
		sockList = topicSockets.find(topic)->second;
		// creates the packet to send 
		response = publish(topic, message);
		cout << "sending ..." << endl;
		// sending to all sockets in the subscribed list
		for (auto const &i: sockList) {
			if(i != socket){
				send(i, response.c_str(), response.length(), 0);
			}
		}
	}
}

// function used by each new thread
void * process(void * ptr)
{
	// buffer is the array receivd in the socket
    char buffer[1024] = {0};
	// conn is a struct containing all info about the sockets 
    connection_t * conn;
	// addr is the adress of the connected client(s)
    long addr = 0;
	// response is a string used to create and send the correct response to a message 
	string response;
	// topicLength  is a variable used to get the length of the topic from the buffer
	int topicLength; // check if useful ---------------------------------------------------------------
	// controlPacketType is a variable used to get the type of message received
	unsigned char controlPacketType;
	// packetID is the variable used to get the receiving packet ID and to put on the repsonse message
	unsigned char packetID[2] = {0};
	// list of all sockets subscribed to a topic
	list<int> sockList = {0};
	// boolean used to managd the retained messages
	bool retain;
	// string used to send the last retain message to clients connecting 
	string pubRetainMessage;
	// string used to extract the topic from the buffer
	string topic;
	// string used to extract the message from the buffer
	string message;

	// some threading init
	if (!ptr) pthread_exit(0);
    conn = (connection_t *)ptr;

	// while message type is not disconnect
	while (buffer[0] != 0xffffffe0){
		// get the address of the client
		addr = (long)((struct sockaddr_in *)&conn->address)->sin_addr.s_addr;
		
		/* read message */
		read(conn->sock, buffer, 1024);
		
		controlPacketType = buffer[0];
		// treatment of the different messages types
		switch (controlPacketType)
		{
		// connect
		case 0x10:
			response = connectAck();
			send(conn->sock, response.c_str(), response.length(), 0);
			break;
		// ping
		case 0xc0:
			response = pingAck();
			send(conn->sock, response.c_str(), response.length(), 0);
			break;
		// subscribe
		case 0x82:
			packetID[0] = buffer[2];
			packetID[1] = buffer[3];
			message = "";
			// topicLength is spread on two bytes in the buffer, so we combine them and convert to int
			topicLength = (int)buffer[5] + (int)buffer[4];
			topic = getTopic(buffer, topicLength, "subscribe");
			addToMap(topic, conn->sock);
			response = subscribeAck(packetID);
			send(conn->sock, response.c_str(), response.length(), 0);
			// send last retain to new subscriber
			cout << topic << endl;
			/*pubRetainMessage = publish(packetID, topic, lastMessage);
			send(conn->sock, pubRetainMessage.c_str(), pubRetainMessage.length(), 0);*/
			for(std::map<string, string>::iterator it = topicRetain.begin(); it != topicRetain.end(); ++it) {
				cout << it->first << " | " << it-> second << endl;
			}
			if(topicSockets.find(topic) != topicSockets.end() && topicRetain.find(topic)->second != ""){
				// gets the last retained message of the topic
				message = topicRetain.find(topic)->second; //problem here the retain msg is also put in other topics
				// creates the packet to send 
				pubRetainMessage = publish(topic, message);
				cout << "sending last message ..." << endl;
				send(conn->sock, pubRetainMessage.c_str(), pubRetainMessage.length(), 0);
			}
			break;
		// unsubscribe
		case 0xa2:
			packetID[0] = buffer[2];
			packetID[1] = buffer[3];
			topicLength = (int)buffer[4] + (int)buffer[5];
			topic = getTopic(buffer, topicLength, "unsubscribe");
			removeFromMap(topic, conn->sock);
			response = unsubscribeAck(packetID);
			send(conn->sock, response.c_str(), response.length(), 0);
			break;
		// publish without retain
		case 0x30:
			packetID[0] = buffer[2];
			packetID[1] = buffer[3];
			topicLength = (int)buffer[3] + (int)buffer[2];
			topic = getTopic(buffer, topicLength, "publish");
			message = getMessage(buffer, topicLength);
			sendPublish(message, topic, sockList, conn->sock);
			break;
		// publish with retain option
		case 0x31:
			topicLength = (int)buffer[3] + (int)buffer[2];
			topic = getTopic(buffer, topicLength, "publish");
			message = getMessage(buffer, topicLength);
			// if the retain option is set, set this message as the one to be sent 
			if(topicRetain.find(topic) != topicRetain.end()){
				topicRetain.find(topic)->second = message;
			} else {
				// if topic doesnt exist in the map
				topicRetain[topic] = message;
			}
			/* for(std::map<string, string>::iterator it = topicRetain.begin(); it != topicRetain.end(); ++it) {
				cout << it->first << " | " << it-> second << endl;
			} */
			sendPublish(message, topic, sockList, conn->sock);
			break;
		// disconnect
		case 0xe0:
			close(conn->sock);
			break;
		default:
			break;
		}
	}
	// close socket and clean up
    close(conn->sock);
    free(conn);
    pthread_exit(0);
}

int main(int argc, char const* argv[]) {
	// socket identifier 
	int sock = -1;
	// address struct
    struct sockaddr_in address;
	// connection struct
    connection_t * connection;
	// thread struct
    pthread_t thread;

	// create socket
    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock <= 0) {
        fprintf(stderr, "%s: error: cannot create socket\n", argv[0]);
        return -3;
    }

	// bind the socket to the MQTT port
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    if (bind(sock, (struct sockaddr *)&address, sizeof(struct sockaddr_in)) < 0) {
        fprintf(stderr, "%s: error: cannot bind socket to port %d\n", argv[0], PORT);
        return -4;
    }

	// listen on the MQTT port
    if (listen(sock, 5) < 0) {
        fprintf(stderr, "%s: error: cannot listen on port\n", argv[0]);
        return -5;
    }

	printf("%s: listening...\n", argv[0]);

	// infinite loop accepting new connections
    while (1){
		// accept new incoming connections
        connection = (connection_t *)malloc(sizeof(connection_t));
        connection->sock = accept(sock, &connection->address, &connection->addr_len);
        if (connection->sock <= 0) {
            free(connection);
        } else {
            // start a new thread without waiting for it 
            pthread_create(&thread, 0, process, (void *)connection);
            pthread_detach(thread);
        }
    }
	return 0;
}



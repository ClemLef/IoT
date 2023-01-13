/* 
   Clement LEFEBVRE / Implementing IoT protocols
   Code created for lab 3 : Project
   Multithread: 
   Main thread: converts MQTT publish messages into CoAP and sends to sensor to turn ON/OFF light
   2nd thread: polling occupancy from sensor via CoAP and sending the value to the MQTT broker

   This code is based from the examples of the library paho.mqtt.c (https://github.com/eclipse/paho.mqtt.c)
*/

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <MQTTClient.h>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <iostream>

using namespace std;

const char *ADDRESS_MQTT_BROKER = "tcp://192.168.137.64";
const char *ADDRESS_SENSOR = "192.168.137.102";

#define CLIENTID    "ESP8266"
#define TOPIC       "light"
#define PAYLOAD     "0"
#define QOS         0
#define TIMEOUT     10000L

volatile MQTTClient_deliveryToken deliveredtoken;

// define coap port
#define PORT	5683
// define size of the buffer
#define MAXLINE 1024

// Generating a random message ID
// For some reason this function did not work on the raspberry
string randomMsgId(){
	char randomId[] = {char(rand() % 255 + 1), char(rand() % 255 + 1)};
	return randomId;
}

string get() {
	// Declaring parameters
	string message = "";
	// Declaring parameters as bits
	// Settings represent Version (01), Type (01) and Token length (0000)
    unsigned char settings = 0b01010000;
	// Method respresent the method used (GET = 0.01)
    unsigned char method = 0b00000001;
	// Have to use a fixed ID because random ID added unwanted bytes to the message on the raspberry
	unsigned char msgId[] = {0b00000000, 0b00000001};
	// URI-Path is option 11 (1011) and occupancy is 9 bytes long (1001)
	unsigned char uriPath = 0b10111001;
    // binary representation of string "occupancy"
	unsigned char pathOccupancy[] = {0b01101111,0b01100011,0b01100011,0b01110101,0b01110000,0b01100001,0b01101110,0b01100011,0b01111001};
	
	// Forming a message based on the parameters
	message.push_back(settings);
	message.push_back(method);
	message.push_back(msgId[0]);
	message.push_back(msgId[1]);
	message += uriPath;
	for(int i = 0; i < 9; i++){
		message.push_back(pathOccupancy[i]);
	}
	return message;
}

string put(char input, string path){
	// Declaring parameters
	string message = "";

	// Declaring parameters as bits
	// Settings represent Version (01), Type (01) and Token length (0000)
    unsigned char settings = 0b01010000;
	// Method respresent the method used (PUT = 0.03)
    unsigned char method = 0b00000011;
	// Have to use a fixed ID because random ID added unwanted bytes to the message on the raspberry
	unsigned char msgId[] = {0b00000000, 0b00000001};
	// URI-Path is option 11 (1011) and light is 5 bytes long (0101)
	unsigned char uriPath = 0b10110101;
	// binary representation of string "light"
	unsigned char pathLight[] = {0b01101100,0b01101001,0b01100111,0b01101000,0b01110100};
	// payloadOption contains the parameters for the payload (1 = 0001 and 0 = 0000 for length)
	unsigned char payloadOption = 0b00010000;
	// separate header and payload with 11111111
	unsigned char separator = 0b11111111;

	// Forming a message based on the parameters
	message.push_back(settings);
	message.push_back(method);
	message.push_back(msgId[0]);
	message.push_back(msgId[1]);
	message.push_back(uriPath);
	for(int i = 0; i < 5; i++){
		message.push_back(pathLight[i]);
	}
	message.push_back(payloadOption);
	message.push_back(separator);
	message += input;
	return message;
}


// Function getting the contents of the response
string getContent(char *buffer, int bufferSize){
	string content;
	for(int i = 8; i < bufferSize; i++){
		content += buffer[i];
	}
	return content;
}

// Function used to send requests to send a request to a coap server
void sendRequest(int sockfd, sockaddr_in servaddr, string message, char *buffer){
	// Size of the responseoccupancy
	unsigned int n = 0;
	// Size of the server address
	unsigned int len = 0;
	// Sending the message to the test server
	sendto(sockfd, message.c_str(), message.length(), MSG_CONFIRM, (const struct sockaddr *) &servaddr, sizeof(servaddr));

	// Get the response from the server
    n = recvfrom(sockfd, buffer, MAXLINE + 1,
            MSG_WAITALL, (struct sockaddr *) &servaddr,
            &len);
	// Closing the answer at the end
    buffer[n] = '\0';

	// Printing the contents
	// cout << getContent(buffer, n) << endl;
}

int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
	//CoAP init used to send light ON/OFF to sensor
	// Socket file descriptor
	int sockfd;
	// Buffer to store the response
	char buffer[MAXLINE];
	// Address of the server
	struct sockaddr_in servaddr;
	// message to send the request, input of the user, path the user wants to access
	string messageCoAP;

	// Create a socket file descriptor
	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
		perror("socket creation failed");
		exit(EXIT_FAILURE);
	}
	
	// Reserve memory to store the server address
	memset(&servaddr, 0, sizeof(servaddr));

	// CoAP server network info
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(PORT);
	servaddr.sin_addr.s_addr = inet_addr(ADDRESS_SENSOR);

	// Getting the MQTT message payload and forming a CoAP put message
	char* payload = (char*)message->payload;
	messageCoAP = put(*(payload), TOPIC);
	sendRequest(sockfd, servaddr, messageCoAP, buffer);
	
	// Clear values
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
}

// Function called when connection is lost
void connlost(void *context, char *cause)
{
    printf("\nConnection lost\n");
    printf("     cause: %s\n", cause);
}

// Function called when message is delivered (not used here)
void delivered(void *context, MQTTClient_deliveryToken dt)
{
    printf("Message with token value %d delivery confirmed\n", dt);
    deliveredtoken = dt;
}

// Threaded function getting the value of occupancy via CoAP and sends it to the MQTT broker
void * process(void * ptr)
{
	//MQTT init used to send occuppancy to broker
	int returnCode;
    MQTTClient mqttClient;
    MQTTClient_connectOptions connectionOptions = MQTTClient_connectOptions_initializer;
    MQTTClient_message messageMQTT = MQTTClient_message_initializer;
    MQTTClient_deliveryToken deliveryToken;

	connectionOptions.keepAliveInterval = 60;
    connectionOptions.cleansession = 1;

	messageMQTT.qos = QOS;
	messageMQTT.retained = 0;

	if ((returnCode = MQTTClient_create(&mqttClient, ADDRESS_MQTT_BROKER, CLIENTID,
        MQTTCLIENT_PERSISTENCE_NONE, NULL)) != MQTTCLIENT_SUCCESS)
    {
         printf("Failed to create client, return code %d\n", returnCode);
         exit(EXIT_FAILURE);
    }

	if ((returnCode = MQTTClient_connect(mqttClient, &connectionOptions)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to connect, return code %d\n", returnCode);
		pthread_exit(0);
        exit(EXIT_FAILURE);
    }

	//CoAP init used too retreive occupancy from sensor
	// Socket file descriptor
	int sockfd;
	// Buffer to store the response
	char buffer[MAXLINE];
	// Address of the server
	struct sockaddr_in servaddr;
	// message to send the request, input of the user, path the user wants to access
	string message;

	// Create a socket file descriptor
	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
		perror("socket creation failed");
		exit(EXIT_FAILURE);
	}

	// Reserve memory to store the server address
	memset(&servaddr, 0, sizeof(servaddr));

	// CoAP server network info
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(PORT);
	servaddr.sin_addr.s_addr = inet_addr(ADDRESS_SENSOR);

	//infinite loop polling CoAP occupancy and sending it to the broker
	while(1){
		message = get();
		sendRequest(sockfd, servaddr, message, buffer);
		// Getting the CoAP occuancy and forming a MQTT message
		const char* payloadMQTT = getContent(buffer, 9).c_str();
		messageMQTT.payload = (void *)payloadMQTT;
		messageMQTT.payloadlen = 1;

		MQTTClient_publishMessage(mqttClient, "occupancy", &messageMQTT, &deliveryToken);
		sleep(5);
	}
    pthread_exit(0);
}

int main(int argc, char* argv[]) {
	// thread struct
    pthread_t thread;
    //creates the thread used to get the occupancy every 5 seconds
    pthread_create(&thread, 0, process, 0);
	pthread_detach(thread);

    //MQTT init
	int returnCode;
    MQTTClient mqttClient;
    MQTTClient_connectOptions connectionOptions =     MQTTClient_connectOptions_initializer;
    MQTTClient_message message = MQTTClient_message_initializer;
    MQTTClient_deliveryToken deliveryToken;

	connectionOptions.keepAliveInterval = 60;
    connectionOptions.cleansession = 1;

    if ((returnCode = MQTTClient_create(&mqttClient, ADDRESS_MQTT_BROKER, CLIENTID,
        MQTTCLIENT_PERSISTENCE_NONE, NULL)) != MQTTCLIENT_SUCCESS)
    {
         printf("Failed to create client, return code %d\n", returnCode);
         exit(EXIT_FAILURE);
    }

	if ((returnCode = MQTTClient_setCallbacks(mqttClient, NULL, connlost, msgarrvd, delivered)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to set callbacks, return code %d\n", returnCode);
        returnCode = EXIT_FAILURE;
        MQTTClient_destroy(&mqttClient);
    }

    if (MQTTClient_connect(mqttClient, &connectionOptions) != MQTTCLIENT_SUCCESS) {
        printf("Failed to connect\n");
        return (-1);
    }

    if ((returnCode = MQTTClient_subscribe(mqttClient, TOPIC, QOS)) != MQTTCLIENT_SUCCESS)
    {
    	printf("Failed to subscribe, return code %d\n", returnCode);
    	returnCode = EXIT_FAILURE;
    }
    else
    {
    	int ch;
    	do
    	{
        	ch = getchar();
    	} while (ch!='Q' && ch != 'q');
        if ((returnCode = MQTTClient_unsubscribe(mqttClient, TOPIC)) != MQTTCLIENT_SUCCESS)
        {
        	printf("Failed to unsubscribe, return code %d\n", returnCode);
        	returnCode = EXIT_FAILURE;
        }
    }
    if ((returnCode = MQTTClient_disconnect(mqttClient, 10000)) != MQTTCLIENT_SUCCESS)
    	printf("Failed to disconnect, return code %d\n", returnCode);

    MQTTClient_destroy(&mqttClient);
    return (0);
}
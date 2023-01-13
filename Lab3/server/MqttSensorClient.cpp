/* 
   Clement LEFEBVRE / Implementing IoT protocols
   Code created for lab 3 : Project
   Multithread: 
   Main thread: converts MQTT publish messages into CoAP and sends to sensor to turn ON/OFF light
   2nd thread: polling occupancy from sensor via CoAP and sending the value to the MQTT broker
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

const char *ADDRESS_MQTT_BROKER = "tcp://192.168.137.107";
const char *ADDRESS_SENSOR = "192.168.137.236";

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
string randomMsgId(){
	char randomId[] = {char(rand() % 255 + 1), char(rand() % 255 + 1)};
	return randomId;
}

string get(string path) {
	// Declaring parameters
	string message = "";
	// Get the path length
	int size = 5;
	// Declaring parameters as bits
	// Settings represent Version (01), Type (01) and Token length (0000)
    unsigned char settings = 0b01010000;
	// Method respresent the method used (GET = 0.01)
    unsigned char method = 0b00000001;

	unsigned char msgId[] = {0b00000000, 0b00000001};

	unsigned char uriPath = 0b10111001;
    // Generate a random message ID
	unsigned char pathOccupancy[] = {0b01101111,0b01100011,0b01100011,0b01110101,0b01110000,0b01100001,0b01101110,0b01100011,0b01111001};
	
	// Forming a message based on the parameters
	message.push_back(settings);
	message.push_back(method);
	message.push_back(msgId[0]);
	message.push_back(msgId[1]);
	// 16+32+128 correspond to the bits we need to set to configure the option URI-path (11)
	message += uriPath;
	for(int i = 0; i < 8; i++){
		message.push_back(pathOccupancy[i]);
	}
	cout << message.length() << endl;
	return message;
}

string put(char input, string path){
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
	// Size of the response
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

	// Printing the headers and contents
    //cout << getHeaders(buffer) << endl;
	//cout << getContent(buffer, n) << endl;
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

	char* payload = (char*)message->payload;
	messageCoAP = put(*(payload), TOPIC);
	sendRequest(sockfd, servaddr, messageCoAP, buffer);
	
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
}

void connlost(void *context, char *cause)
{
    printf("\nConnection lost\n");
    printf("     cause: %s\n", cause);
}

void delivered(void *context, MQTTClient_deliveryToken dt)
{
    printf("Message with token value %d delivery confirmed\n", dt);
    deliveredtoken = dt;
}


void * process(void * ptr)
{
	//MQTT init used to send occuppancy to broker
	int rc;
    MQTTClient mqttClient;
    MQTTClient_connectOptions connectionOptions =     MQTTClient_connectOptions_initializer;
    MQTTClient_message messageMQTT = MQTTClient_message_initializer;
    MQTTClient_deliveryToken deliveryToken;

    MQTTClient_create(&mqttClient, ADDRESS_MQTT_BROKER, CLIENTID, 
    MQTTCLIENT_PERSISTENCE_NONE, NULL);

	connectionOptions.keepAliveInterval = 20;
    connectionOptions.cleansession = 1;

    if (MQTTClient_connect(mqttClient, &connectionOptions) != MQTTCLIENT_SUCCESS) {
        printf("Failed to connect\n");
        pthread_exit(0);
    }

	messageMQTT.qos = QOS;
	messageMQTT.retained = 0;

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
		message = get("occupancy");
		sendRequest(sockfd, servaddr, message, buffer);
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
	int rc;
    MQTTClient mqttClient;
    MQTTClient_connectOptions connectionOptions =     MQTTClient_connectOptions_initializer;
    MQTTClient_message message = MQTTClient_message_initializer;
    MQTTClient_deliveryToken deliveryToken;

    MQTTClient_create(&mqttClient, ADDRESS_MQTT_BROKER, CLIENTID, 
    MQTTCLIENT_PERSISTENCE_NONE, NULL);

	if ((rc = MQTTClient_setCallbacks(mqttClient, NULL, connlost, msgarrvd, delivered)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to set callbacks, return code %d\n", rc);
        rc = EXIT_FAILURE;
        MQTTClient_destroy(&mqttClient);
    }

    connectionOptions.keepAliveInterval = 60;
    connectionOptions.cleansession = 1;

    if (MQTTClient_connect(mqttClient, &connectionOptions) != MQTTCLIENT_SUCCESS) {
        printf("Failed to connect\n");
        return (-1);
    }

    if ((rc = MQTTClient_subscribe(mqttClient, TOPIC, QOS)) != MQTTCLIENT_SUCCESS)
    {
    	printf("Failed to subscribe, return code %d\n", rc);
    	rc = EXIT_FAILURE;
    }
    else
    {
    	int ch;
    	do
    	{
        	ch = getchar();
    	} while (ch!='Q' && ch != 'q');
        if ((rc = MQTTClient_unsubscribe(mqttClient, TOPIC)) != MQTTCLIENT_SUCCESS)
        {
        	printf("Failed to unsubscribe, return code %d\n", rc);
        	rc = EXIT_FAILURE;
        }
    }
    MQTTClient_disconnect(mqttClient, 5000);
    MQTTClient_destroy(&mqttClient);

    return (0);
}
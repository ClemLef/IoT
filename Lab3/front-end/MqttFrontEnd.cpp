/* 
   Clement LEFEBVRE / Implementing IoT protocols
   Code created for lab 3: Project
   This code presents a menu to the user that can choose between: 
    Printing the occupancy of the sensor (polling every 5s)
    Turning ON the light on the sensor
    Turning OFF the light on the sensor
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "MQTTClient.h"
#include <string>
#include <iostream>

using namespace std;

// Address of the MQTT broker
#define ADDRESS     "192.168.137.107"
#define CLIENTID    "MqttFrontEnd"
#define TOPIC       "occupancy"
#define QOS         0
#define TIMEOUT     10000L

volatile MQTTClient_deliveryToken deliveredtoken;

// Function called when a new message arrives to the subscribed topic
int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
	char* payload = (char*)message->payload;
    if(*(payload) == '1'){
        system("clear");
        cout << "the room is free" << endl;
    } else {
        system("clear");
        cout << "the room is occupied" << endl;
    }
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


int main(int argc, char* argv[])
{
    //MQTT init
    MQTTClient client;
    MQTTClient_connectOptions connectionOptions = MQTTClient_connectOptions_initializer;
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    MQTTClient_deliveryToken token;
    int returnCode;

    // Used for choice loop
    int choice = -1;

    if ((returnCode = MQTTClient_create(&client, ADDRESS, CLIENTID,
        MQTTCLIENT_PERSISTENCE_NONE, NULL)) != MQTTCLIENT_SUCCESS)
    {
         printf("Failed to create client, return code %d\n", returnCode);
         exit(EXIT_FAILURE);
    }

    if ((returnCode = MQTTClient_setCallbacks(client, NULL, connlost, msgarrvd, delivered)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to set callbacks, return code %d\n", returnCode);
        returnCode = EXIT_FAILURE;
        MQTTClient_destroy(&client);
    }

    connectionOptions.keepAliveInterval = 60;
    connectionOptions.cleansession = 1;

    if ((returnCode = MQTTClient_connect(client, &connectionOptions)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to connect, return code %d\n", returnCode);
        exit(EXIT_FAILURE);
    }
    
    while(choice != 0){
        cout << endl;
		cout << "  1. Show live occupancy" << endl;
		cout << "  2. Turn on" << endl;
        cout << "  3. Turn off" << endl;
		cout << "  0. Quit" << endl;
        cout << "Make your choice: " << endl;
        cin >> choice;
        system("clear");
		switch (choice)
		{
		case 0:
			// Quit the app
			break;
		case 1:
            // Show live occupancy
            cout << "press q to quit" << endl;
            if ((returnCode = MQTTClient_subscribe(client, TOPIC, QOS)) != MQTTCLIENT_SUCCESS)
            {
                printf("Failed to subscribe, return code %d\n", returnCode);
                returnCode = EXIT_FAILURE;
            }
            else {
                int ch;
                do {
                    ch = getchar();
                } while (ch!='Q' && ch != 'q');

                if ((returnCode = MQTTClient_unsubscribe(client, TOPIC)) != MQTTCLIENT_SUCCESS) {
                    printf("Failed to unsubscribe, return code %d\n", returnCode);
                    returnCode = EXIT_FAILURE;
                }
            }
			break;
		case 2:
            // Turn ON the light
            pubmsg.payload = (void*)"0";
            pubmsg.payloadlen = 1;
            pubmsg.qos = QOS;
            pubmsg.retained = 0;

            if ((returnCode = MQTTClient_publishMessage(client, "light", &pubmsg, &token)) != MQTTCLIENT_SUCCESS)
            {
                printf("Failed to publish message, return code %d\n", returnCode);
                exit(EXIT_FAILURE);
            }

            returnCode = MQTTClient_waitForCompletion(client, token, TIMEOUT);
            break;
        case 3:
            // Turn OFF the light
            pubmsg.payload = (void*)"1";
            pubmsg.payloadlen = 1;
            pubmsg.qos = QOS;
            pubmsg.retained = 0;

            if ((returnCode = MQTTClient_publishMessage(client, "light", &pubmsg, &token)) != MQTTCLIENT_SUCCESS)
            {
                printf("Failed to publish message, return code %d\n", returnCode);
                exit(EXIT_FAILURE);
            }

            returnCode = MQTTClient_waitForCompletion(client, token, TIMEOUT);
            break;
        }
    }

    if ((returnCode = MQTTClient_disconnect(client, 10000)) != MQTTCLIENT_SUCCESS)
    	printf("Failed to disconnect, return code %d\n", returnCode);

    MQTTClient_destroy(&client);
    
    return returnCode;
}
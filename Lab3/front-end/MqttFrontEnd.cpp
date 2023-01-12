#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "MQTTClient.h"
#include <string>
#include <iostream>

using namespace std;

#define ADDRESS     "tcp://192.168.137.107:1883"
#define CLIENTID    "MqttFrontEnd"
#define TOPIC       "light"
#define PAYLOAD     "0"
#define QOS         0
#define TIMEOUT     10000L

int main(int argc, char* argv[])
{
    MQTTClient client;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    MQTTClient_deliveryToken token;
    int rc;

    if ((rc = MQTTClient_create(&client, ADDRESS, CLIENTID,
        MQTTCLIENT_PERSISTENCE_NONE, NULL)) != MQTTCLIENT_SUCCESS)
    {
         printf("Failed to create client, return code %d\n", rc);
         exit(EXIT_FAILURE);
    }

    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to connect, return code %d\n", rc);
        exit(EXIT_FAILURE);
    }

    
    int choice = -1;

    while(choice != 0){
        cout << endl;
		cout << "  1. Send a GET" << endl;
		cout << "  2. Turn on" << endl;
        cout << "  3. Turn off" << endl;
		cout << "  0. Quit" << endl;
        cin >> choice;
        system("clear");
		switch (choice)
		{
		case 0:
			// Quit the app
			break;
		case 1:
			break;
		case 2:
            pubmsg.payload = (void*)"0";
            pubmsg.payloadlen = (int)strlen(PAYLOAD);
            pubmsg.qos = QOS;
            pubmsg.retained = 0;
            if ((rc = MQTTClient_publishMessage(client, TOPIC, &pubmsg, &token)) != MQTTCLIENT_SUCCESS)
            {
                printf("Failed to publish message, return code %d\n", rc);
                exit(EXIT_FAILURE);
            }
            rc = MQTTClient_waitForCompletion(client, token, TIMEOUT);
            break;
        case 3:
            pubmsg.payload = (void*)"1";
            pubmsg.payloadlen = (int)strlen(PAYLOAD);
            pubmsg.qos = QOS;
            pubmsg.retained = 0;
            if ((rc = MQTTClient_publishMessage(client, TOPIC, &pubmsg, &token)) != MQTTCLIENT_SUCCESS)
            {
                printf("Failed to publish message, return code %d\n", rc);
                exit(EXIT_FAILURE);
            }
            rc = MQTTClient_waitForCompletion(client, token, TIMEOUT);
            break;
        }
    }
    printf("Waiting for up to %d seconds for publication of %s\n"
            "on topic %s for client with ClientID: %s\n",
            (int)(TIMEOUT/1000), PAYLOAD, TOPIC, CLIENTID);
    rc = MQTTClient_waitForCompletion(client, token, TIMEOUT);
    printf("Message with delivery token %d delivered\n", token);

    if ((rc = MQTTClient_disconnect(client, 10000)) != MQTTCLIENT_SUCCESS)
    	printf("Failed to disconnect, return code %d\n", rc);
    MQTTClient_destroy(&client);
    return rc;
}
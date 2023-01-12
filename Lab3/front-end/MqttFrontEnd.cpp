#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "MQTTClient.h"
#include <string>
#include <iostream>

using namespace std;

#define ADDRESS     "192.168.137.107"
#define CLIENTID    "MqttFrontEnd"
#define TOPIC       "occupancy"
#define QOS         0
#define TIMEOUT     10000L

volatile MQTTClient_deliveryToken deliveredtoken;


int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
    cout << "press q to quit" << endl;
	char* payload = (char*)message->payload;
    if(*(payload) == '1'){
        cout << "the room is free" << endl;
    } else {
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

    if ((rc = MQTTClient_setCallbacks(client, NULL, connlost, msgarrvd, delivered)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to set callbacks, return code %d\n", rc);
        rc = EXIT_FAILURE;
        MQTTClient_destroy(&client);
    }

    conn_opts.keepAliveInterval = 60;
    conn_opts.cleansession = 1;

    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to connect, return code %d\n", rc);
        exit(EXIT_FAILURE);
    }
    
    int choice = -1;

    while(choice != 0){
        cout << endl;
		cout << "  1. Show live occupancy" << endl;
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

            if ((rc = MQTTClient_subscribe(client, TOPIC, QOS)) != MQTTCLIENT_SUCCESS)
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
                if ((rc = MQTTClient_unsubscribe(client, TOPIC)) != MQTTCLIENT_SUCCESS)
                {
                    printf("Failed to unsubscribe, return code %d\n", rc);
                    rc = EXIT_FAILURE;
                }
            }
            MQTTClient_disconnect(client, 5000);
            MQTTClient_destroy(&client);
			break;
		case 2:
            pubmsg.payload = (void*)"0";
            pubmsg.payloadlen = 1;
            pubmsg.qos = QOS;
            pubmsg.retained = 0;
            if ((rc = MQTTClient_publishMessage(client, "light", &pubmsg, &token)) != MQTTCLIENT_SUCCESS)
            {
                printf("Failed to publish message, return code %d\n", rc);
                exit(EXIT_FAILURE);
            }
            rc = MQTTClient_waitForCompletion(client, token, TIMEOUT);
            break;
        case 3:
            pubmsg.payload = (void*)"1";
            pubmsg.payloadlen = 1;
            pubmsg.qos = QOS;
            pubmsg.retained = 0;
            if ((rc = MQTTClient_publishMessage(client, "light", &pubmsg, &token)) != MQTTCLIENT_SUCCESS)
            {
                printf("Failed to publish message, return code %d\n", rc);
                exit(EXIT_FAILURE);
            }
            rc = MQTTClient_waitForCompletion(client, token, TIMEOUT);
            break;
        }
    }

    if ((rc = MQTTClient_disconnect(client, 10000)) != MQTTCLIENT_SUCCESS)
    	printf("Failed to disconnect, return code %d\n", rc);
    MQTTClient_destroy(&client);
    return rc;
}
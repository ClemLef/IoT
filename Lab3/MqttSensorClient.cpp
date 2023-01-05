#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <MQTTClient.h>

using namespace std;

const char *ADDRESS = "tcp://192.168.137.64";
const char *TOPIC = "testtopic/hello";

int main(int argc, char* argv[]) {
    const char *MESSAGE_STRING = "Hello, this is a message from the RevPi";

    MQTTClient mqttClient;
    MQTTClient_connectOptions connectionOptions =     MQTTClient_connectOptions_initializer;
    MQTTClient_message message = MQTTClient_message_initializer;
    MQTTClient_deliveryToken deliveryToken;

    MQTTClient_create(&mqttClient, ADDRESS, "RevPi", 
    MQTTCLIENT_PERSISTENCE_NONE, NULL);

    connectionOptions.keepAliveInterval = 20;
    connectionOptions.cleansession = 1;

    if (MQTTClient_connect(mqttClient, &connectionOptions) != MQTTCLIENT_SUCCESS) {
        printf("Failed to connect\n");
        return (-1);
    }

     // Copy the message string to avoid compiler warnings
    char *s = new char[strlen(MESSAGE_STRING)+1];
    strcpy(s,MESSAGE_STRING);

    message.payload = s;
    message.payloadlen = strlen(s);
    message.qos = 0;
    message.retained = 0;
    
    MQTTClient_publishMessage(mqttClient, TOPIC, &message, &deliveryToken);
    MQTTClient_disconnect(mqttClient, 5000);
    MQTTClient_destroy(&mqttClient);

    return (0);
}
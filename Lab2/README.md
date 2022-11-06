# Programming Project 2: MQTT Broker
In the second programming project you will create an MQTT broker program that can broker MQTT messages. The program should be able to handle subscribe and unsubscribe requests, as well as handle clients that publish information to the subscribed topics. Hence, you will need to create a program that can correctly recieve connections from MQTT clients and then interpret the basic MQTT messages.

Remember that you must code everything from scratch. No existing MQTT library is allowed to used. Build it yourself.

Your MQTT client need to minimally support the following commands (mqtt version 3.1.1)

## Connect
The program should be able to receive and handle clients that want to connect

## Subscribe
The program should be able to receive and handle client requests for subscribing to different topics.

## Unsubscribe
The program should be able to receive and handle client request for unsubscribing to previously subscribed topics.

## Publish
The program should be able to receive and handle clients that want to publish messages to certain topics, including retain functionality.

## MQTT Ping
The program should be able to receive and handle clients that sends MQTT Ping requests

## Disconnect
The program should be able to receive and handle clients that wants to gracefully disconnect. As well as recover when someone ungracefully disconnects.

# Testing Tools

You can for example use these MQTT clients to test your broker


MQTT explorer: http://mqtt-explorer.com/

MQTT-spy: https://github.com/eclipse/paho.mqtt-spy


MQTT X: https://mqttx.app/

Hive MQTT Client: https://www.hivemq.com/blog/mqtt-client-library-enyclopedia-hivemq-mqtt-client/

More tools can be found at: https://www.hivemq.com/blog/seven-best-mqtt-client-tools/

# Submit your code and short lab report
You need to have to shown and orally presented your project to the teacher before you hand in your code and a short lab report. Remember to explain your implementation from the given protocol specifications in the report.

Make sure this works in your code: Start Wireshark. Connect 2 clients, subscribe with one, then publish with the other. Unsubscribe, then publish again, disconnect, publish with retention, connect back again and subscribe. Finally, do an ungraceful shutdown and publish again. Check that everything was ok in Wireshark.

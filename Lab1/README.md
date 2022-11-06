# Programming Project 1: CoAP Client
In the first programming project you will create a client program that can talk to an IoT server (for example a sensor) via CoAP. The program should be able to discover available information on the IoT server, retrieve data from the IoT server, and push data to the IoT server. Hence, you will need to create a program that can correctly connect to a CoAP server, to then send, recive, and interpret CoAP messages.

Remember that you must code everything from scratch. No existing CoAP library is allowed to used. Build it yourself.

Your CoAP client need to minimally support the following commands, and then display the responses from the CoAP server just like a real CoAP client.

## POST
The program should be able to send POST messages to send data to sensors/actuators with a short path names.

## PUT
The program should be able to send PUT messages to send data to sensors/actuators with a short path names.

## GET
The program should be able to send GET messages to retrieve data from sensors/actuators with a short path names. (You don't need to implement block transfer)

## DELETE
The program should be able to send DELETE messages to remove data from sensors/actuators with a short path names.

#Testing Tools

Easiest is to test against CoAP.me on port 5683

For testing GET: coap.me/test  

For testing POST, UPDATE, and DELETE: coap.me/sink

For testing DISCOVER: coap.me  or coap.me/.well known/core

# Submit your code and short lab report
You need to have to shown and orally presented your project to the teacher before you hand in your code and a short lab report. Remember to explain your implementation from the given protocol specifications in the report.

Make sure this works in your code: Start Wireshark. Connect your client. GET both sink och hello. POST to both sink and hello. Check that you it was received correctly with GET. PUT to both SINK and hello. Check that you it was received correctly with GET. Check that everything was ok in Wireshark, packet size in particular.

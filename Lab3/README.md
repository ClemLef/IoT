# Programming Project 3: Combined IoT Project
In the third and final programming project you will create a combined IoT system, using your code from the two previous projects. You will be provided with a basic structure/skeleton setup, which you then can add more things to yourself. You will have to come up with your own scenario and implementation choices. Below is a figure of the basic skeleton system:

## Overview of the lab

 ![diagram of the project](/tree/main/Lab3/overview.png)

The components in the basic skeleton are: One program running a CoAP server. One program acting as the gateway which is running a CoAP client and an MQTT client. One program running the MQTT broker. And finally, one program running an MQTT client and some form of front-end for your system.

The project is quite open to what scenario you choose, what you choose to extend, etc. But we have some basic requirements:

## Requirements
- You must use both of your previous laboration projects. The MQTT broker and the CoAP client.
- You must have at least one sensors in your project, easiest is to just emulate/fake it to start with.
- You must have some type of human friendly front-end/interface
- You must quantitatively measure and evaluate your system. For example by measuring scalability, response times, etc.
- You don't need to create the corresponding clients/servers from scratch. You can use ready made libraries for the Coap Server and the MQTT clients, since they should be compatible with your self made code.
- It doesn't have to be run on different devices etc. you can run everything on the same machine etc. But it should be possible to move the programs to different devices and they should still work.

## Tips to extend the basic skeleton

- Run on real IoT devices, like Raspberry Pi
- Making it actually correspond to a real scenario (smart home, smart healthcare, industrial control system, etc.)
- Some form of actuator (alarm, controller, of just a screen)
- Adding more sensors and actuators, both CoAP and MQTT connected.
- Real-world sensing and actuating using real sensors/actuators
- Arduino CoAP sensor server (ex. ESP32, 8266)
- REST interface
- More advanced measurements and evaluation.
- Security aspects (using SSL/TLS, attacking the system, Denial of Service etc.)
- Making your system compatible with other IoT platforms such as NodeRed, TheThings.io, Thingsboard, etc. 

## Project presentation

In this final project you will also present your work for the class and this presentation is allowed to be maximum 15 minutes long. 

Presentation date: 2023-01-12 at 13:15 in C312

The presentation should summarize your work. Use presentation slides. Maximum length: 15 minutes.

Submit your code and project report

## Written report
The written report for this part shall be done in the form of a full project report, i.e. more than a normal lab report. Feel free to use the thesis report template if you want to. The project report should contain an introduction, background/theory, methodology, implementation, results/evaluation, and conclusions/discussion. Also, remember to include the following details in the project report:

- Describe of how this networking applications work and is designed. Discuss your choices.
- Show how well your system works by quantitative measurements. Analyze the measurements.
- Discuss for what it's purpose is for the Internet of Things and its advantages and problems. Describe it's relation to other systems/platforms/protocols.

Final report due: 2023-01-15 at 23:59

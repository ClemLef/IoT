#Programming Project 0: HTTP/REST Server (Optional/Excercise)
This is not one of the mandatory laboration projects.

This task is only here as an exercise. This whole task will also be studied and programmed in one the lectures as a learning task.

In this exercise project you will create a server that will act as a simple IoT sensor value database to save and retrieve sensor values. Hence, you will need to create a program that can correctly accept and interpret REST calls. As well as store the sensor values in some form of persistent storage.

Remember that you must code everything from scratch. No existing web server, REST interpreter, or database system is allowed to used. Build it yourself.

Your REST server need to minimally support the following commands

##POST
Should be used to initially create a sensor entity so that sensor values can be added later. Return 201 if successfully created and 409 if the sensor already exists.

##PUT
Should be used to persistently save a sensor value on the REST server. Return 200 if updated and 404 if not found.

##GET
Should be used to retrieve a sensor value from the REST server. Return 200 with the value or 404 if not found.

##DELETE
Should be used to remove a sensor entity and its sensor value from the REST server. Return 404 if not found or 200 if successfully deleted. 

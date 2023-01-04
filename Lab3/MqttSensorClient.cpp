#include <fstream> 

int readTemp(){
	string tempStr;
	float temp;
	// Read from the file
	ifstream MyReadFile("/sys/class/thermal/thermal_zone0/temp");
	getline (MyReadFile, tempStr);
	cout << tempStr;
	temp = stof(tempStr);
	temp = temp/1000;
	cout << temp;
	return temp;
}
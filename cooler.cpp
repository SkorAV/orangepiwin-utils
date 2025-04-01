#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <wiringPi.h>
#include <softPwm.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <ctime>

#define LOG_FILE ("/var/log/cooler.log")

bool shouldStop = false;
int pin = 19;

std::string getCurrentTime() {
    std::time_t now = std::time(nullptr);
    char buf[80];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
    return std::string(buf);
}

void writeToLogFile(const std::string& message) {
    std::ofstream logFile(LOG_FILE, std::ios_base::app);
    if (logFile.is_open()) {
        logFile << getCurrentTime() << " - " << message << std::endl;
        logFile.close();
    } else {
        std::cerr << "Unable to open log file." << std::endl;
    }
}

void sigHandler(int signo) {
    if(signo == SIGTERM) {
        shouldStop = true;
    } else if(signo == SIGINT) {
        printf("\nExiting...\n");
        shouldStop = true;
    }
} 

void temperatureReadLoop() {
    FILE *fp;
    int temperature;

    while(!shouldStop) {
    	fp = fopen("/sys/class/thermal/thermal_zone0/temp", "r");
        fscanf(fp, "%d", &temperature);
    	fclose(fp);

	if (temperature < 40000) {
	    softPwmWrite(pin, 0);
	} else if (temperature < 45000) {
	    softPwmWrite(pin, 25);
	} else if (temperature < 50000) {
	    softPwmWrite(pin, 50);
	} else if (temperature < 60000) {
	    softPwmWrite(pin, 75);
	} else {
	    softPwmWrite(pin, 100);
	}

	std::ostringstream oss;
	oss.precision(2);
	oss << std::fixed;
	oss << "Temperature: " << (temperature / 1000.0) << " °C";
	writeToLogFile(oss.str());
        sleep(1);
    }

    softPwmStop(pin);
}

int main() {
    signal(SIGTERM, sigHandler);
    signal(SIGINT, sigHandler);
    wiringPiSetup();
    pinMode(pin, OUTPUT);
    softPwmCreate(pin, 0, 100);
    temperatureReadLoop();
}

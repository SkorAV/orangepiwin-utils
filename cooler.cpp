#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <wiringPi.h>
#include <softPwm.h>

bool shouldStop = false;
int pin = 19;

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
	    softPwmStop(pin);
	} else if (temperature < 45000) {
	    softPwmWrite(pin, 25);
	} else if (temperature < 50000) {
	    softPwmWrite(pin, 50);
	} else if (temperature < 60000) {
	    softPwmWrite(pin, 75);
	} else {
	    softPwmWrite(pin, 100);
	}

        printf("\nTemperature: %.2f °C", temperature / 1000.0);
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

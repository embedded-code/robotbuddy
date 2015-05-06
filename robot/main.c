//
// main.cpp
// Main file
// ----------------------------------
// Developed with embedXcode
// http://embedXcode.weebly.com
//
// Project 		robot
//
// Created by 	Tarun Chaudhry, 5/6/15 12:00 AM
//              Tarun Chaudhry
//
// Copyright 	© Tarun Chaudhry, 2015
// Licence 		<#license#>
//
// See 			robot.ino and ReadMe.txt for references
//
// ----------------------------------
// DO NOT EDIT THIS FILE.
// THE SKETCH IS IN robot.ino
// ----------------------------------
//
// Last update: Apr 15, 2015 release 280
#include "I2C_MSP430.h"
#include "MSP430_MPU6050.h"
#include "MPU6050.h"
// ==================
// Include Serial Communication
#include "UART.h"
// ==================
// Include Timer Library (for Motor)
#include "Timer.h"
// ==================
// Include State Estimation
#include "StateEstimate.h"
// ==================
// Include PID Controller
#include "PIDController.h"
// ==================
// Include Bluetooth
#include "Bluetooth.h"
// ==================
// Include configuation constants
#include "Config.h"

// Define variables and constants
#define Kout 1


void setMotorSpeedAndDirection(double output) {
    if (output > 0) {
        isForward = 1;
    } else {
        isForward = 0;
    }

    duty = Kout * output;
}

void setupI2C() {
    // Assign I2C pins to USCI_B0
    P1SEL |= BIT6 + BIT7;
    P1SEL2|= BIT6 + BIT7;

    // Initialize IMU
    initializeIMU();

    P1DIR |= BIT4 + BIT5;

    // MS Delay
    msDelay(10);
}

//
// Brief	Setup
// Details	Define the pin the LED is connected to
//
// Add setup code
void setup() {
//    pinMode(RED_LED, OUTPUT);

    // Init UART if required
    #if UART_OUT // if UART is enabled

    // P1.1,1.2 are UART outputs
    P1SEL |= BIT1 + BIT2;
    P1SEL2 |= BIT1 + BIT2;
    UART_init();
    #endif // end UART

    // Init Timer
    TIMER_init();

    // Init Bluetooth
    // Bluetooth_init();

    // Setup I2C Communication
    setupI2C();
}

//
// Brief	Loop
//
// Add loop code
void loop() {
    //Bluetooth_command();

    double angle = getAngleEstimate();
    double output = getOutput(angle);
    setMotorSpeedAndDirection(output);

//    UART_out_double(angle);
//    UART_out_double(output)
}


// IDE selection
int main(void)
{
	setup();
    
	for (;;) {
		loop();
//		if (serialEventRun) serialEventRun();
	}
    
	return 0;
}
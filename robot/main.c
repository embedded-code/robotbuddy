#include "msp430g2553.h"
#include <stdint.h>
#include <stdio.h>

// Include I2C MSP430 and MPU6050 libraries.
#include "I2C_MSP430.h"						// Use the correct I2C library particular to your MCU.
#include "MSP430_MPU6050.h"
#include "MPU6050.h"

#include "hwuart.h"
// Sensor biases when still (accelx,accely,accelz,gyrox,gyroy,gyroz)
// -7.37593331e+02,   1.99838676e+03,   1.58679221e+04,
// -2.12991066e+02,   2.04664451e+02,   1.03180331e+01
#define GYRO_BIAS -213.0
#define ACCEL_BIAS -738.0
static const uint8_t UARTOUT=1;
unsigned int duty=50;
char forward=0;
unsigned int PWM_COUNTER=0;
unsigned int PWM_PERIOD=1000;

double dt=0.001;
double Kp=1;
double Ki=1;
double Kd=1;
char message[64];

double Kout=1;

double angleEstimate=0;
double cAlpha = 0.98;
double cBeta = 0.02;
double gyro,accel;
// Sensor sensitivities
#define gyroSense 131.0;
#define accelSense  16384.0;

double output;

void initializeTimer(void);

int main(void)
{
	  int16_t ax, ay, az;					//<! Holds the raw accelerometer data.
	  int16_t gx, gy, gz;					//<! Holds the raw gyroscope data.

	  WDTCTL = WDTPW + WDTHOLD;             // Stop watchdog timer
	  BCSCTL1 = CALBC1_1MHZ; 				// 1MHZ operation
	  DCOCTL = CALDCO_1MHZ;

	  P1SEL |= BIT6 + BIT7;                     // Assign I2C pins to USCI_B0
	  P1SEL2|= BIT6 + BIT7;
      initializeIMU();						// Initialize IMU
      if(UARTOUT){
      		 P1SEL |= BIT1 + BIT2;						// P1.1,1.2 are UART outputs
      		 P1SEL2 |= BIT1 + BIT2;
      		 initializeUART();
      }


	  P1DIR |= BIT4 + BIT5;
	  initializeTimer();

	  msDelay(10);							// Temporary wait. Can be shortened

	  double preverror;
	  double angle;
	  double error;
	  double integral;
	  double derivative;

	  Kout=Kout*PWM_PERIOD;

	  int c = 0;
	  for (; c < 64; c++) {
		  message[c] = ' ';
	  }

	  for (;;)
	  	{
		  	getMotion6( &ax, &ay, &az, &gx, &gy, &gz);
		  	gyro = ((double)(gx)-GYRO_BIAS)/gyroSense;
		  	accel = ((double)(ax)-ACCEL_BIAS)/accelSense;

		    /* State estimation */
		  	angleEstimate = cAlpha*(angleEstimate + gyro*dt) + cBeta*accel;
		  	angle = angleEstimate;
		  	/* PID Control */
		    error = -1*angle;
		    integral += error*dt;
		    derivative = (error - preverror)*dt;
		    output = Kp*error + Ki*integral + Kd*derivative;
		    preverror = error;
		    if (output>0){
		    	forward=1;
		    }else{
		    	forward=0;
		    }
		    duty = Kout*output;

		//{
		if(UARTOUT){
				// uartSendBytes(output,8);
				// uartSendByte(0x20);
				// ax, ay, az, gx, gy, gz have RAW values from accelerometer and gyroscope
				sprintf(message, "%x%x%x%x %x%x%x%x %x%x%x%x %x%x%x%x\n", gyro,accel,angleEstimate,output);
			//sprintf(message, "G:%x", gyro);
				//uartSendString(message);
				//uartSendBytes(message,messageLength);
				uartSendBytes(&output,8);
				uartSendByte(0x20);
				uartSendBytes(&gyro,8);
				uartSendByte(0x20);
				uartSendBytes(&accel,8);
				uartSendByte(0x20);
				uartSendBytes(&angleEstimate,8);
//				uartSendByte(0x20);
//				uartSendBytes(&error,4);
//				uartSendBytes(&ay,4);
//				uartSendByte(0x20);
//				uartSendBytes(&az,4);
//				uartSendByte(0x20);
//				uartSendBytes(&gx,4);
//				uartSendByte(0x20);
//				uartSendBytes(&gy,4);
//				uartSendByte(0x20);
//				uartSendBytes(&gz,4);
				uartSendByte(0x0A);
			}
	  	}
}

/**
 * Initializes the timer to call the interrupt every 1 second
 */
void initializeTimer(void){
	TA0CCR0 |= 100;
	TA0CCTL0 |= CCIE;
	TA0CTL |= TASSEL_2 + MC_1;

	P1DIR |= BIT0;
	//_BIS_SR(LPM0_bits + GIE);
}

#pragma vector=TIMER0_A0_VECTOR
   __interrupt void Timer0_A0 (void) {
	   PWM_COUNTER++;
	   if (PWM_COUNTER==duty){
		   P1OUT &= ~BIT4;
		   P1OUT &= ~BIT5;
	   }
	   if (PWM_COUNTER>=PWM_PERIOD){
		   if (forward && duty>0){
			   P1OUT |= BIT4;
		   }else if (duty>0){
			   P1OUT |= BIT5;
		   }
		   PWM_COUNTER = 0;
	   }
}

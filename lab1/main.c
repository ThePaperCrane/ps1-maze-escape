//////////////////////////////////////////////////////
// CS266 / CS189                                    //
// Lab1: IR proximity sensors                       //
// Christian Ahler 2012, mod by K. Petersen 2013    //
//////////////////////////////////////////////////////


#include "stdio.h"
#include "math.h"
#include "e_epuck_ports.h"
#include "e_init_port.h"
#include "motor_led/e_motors.h"
#include "motor_led/utility.h"
#include "motor_led/e_led.h"
#include "a2d/e_prox.h"
#include "uart/e_uart_char.h"
#include "bluetooth/btcom.h"

#define HALF_SPEED 500
#define FULL_SPEED 1000
#define E 2
#define W 5
#define NE 1

unsigned char sel;					//the position of the selector switch
char debugMessage[80]; 				//this is some data to store screen-bound debug messages

// Calibrated proximities
double cal_prox(int sen)
{
	double result;
	switch (sen)
	{
		case 0:
			result = 529.02 * pow((double)e_get_prox(sen), -.8455); // fitted function for sensor 0
			break;
		case 1:
			result = 156.66 * pow((double)e_get_prox(sen), -.6619); // sensor 1
			break;
		case 2:
			result = 1525.77 * pow((double)e_get_prox(sen), -.984); // sensor 2
			break;
		case 7:
			result = 185.96 * pow((double)e_get_prox(sen), -.717); // sensor 7
			break;
		default:
			result = 119.905767 * pow((double)e_get_prox(sen), -0.5956); // all other sensors not as important
	}
	return result;
}

// Debug
void printDebug(char *s)
{
	sprintf(debugMessage, s);
	btcomSendString(debugMessage);
}
// Debug
void printProxs(void) 
{
	int i = 0;	
	for (i; i < 8; i++)
	{
		int raw_prox = e_get_prox(i);
		double calib_prox = cal_prox(i);
		sprintf(debugMessage,"prox %i raw: %i, calibrated %f\r\n", i, raw_prox, calib_prox); 	//..read sensor in front (over the camera)
		btcomSendString(debugMessage);
	}
	sprintf(debugMessage, "\r\n");
	btcomSendString(debugMessage);
}

int main(void)
{	
	myWait(500);					//Wait period to prevent UART clogging
	e_init_port();					//Set up the Epuck port pins, refer to utility.c
	e_init_uart1();
	myWait(50);						//Setup uart1

	sel = getselector();			//Read position of selector switch, refer to utility.c
	


	if(sel==0)						//During this class, sel==0 should always do nothing. This will be the programming mode.
	{
		while(1) NOP();
	}
	if(sel==1)						//Flash LEDs
	{
		while(1)
		{
			allRedLEDsOn();
			myWait(500);
			allRedLEDsOff();
			myWait(500);
		}
	}
	else if(sel==2)					//Read proximity and act accordingly
	{
		e_init_prox();

		while(1)
		{	
			// Front is about to hit
			if (cal_prox(0) < 4 || cal_prox(7) < 4 || cal_prox(NE) < 2.5) {
				if (cal_prox(NE) < 3) {
					setSpeeds(-200, 200);
				}
			} else {
				setSpeeds(500,500);
			}
			if (cal_prox(6) < 3 || cal_prox(5) < 3) {
				setSpeeds(200, -200);
			}

			// Right side is >3cm away from wall
			double r = cal_prox(E);
			double ne = cal_prox(1);
			while (r > 3) {
				if (cal_prox(0) < 4 || cal_prox(7) < 4) {
					break;
				}
				setSpeeds(450,200);
				if (ne < 5) {
					setSpeeds(200,400);
				}
				r = cal_prox(E);
				ne = cal_prox(1);
			}

			// Too close to the wall, turning right
			while (r < 2.5 && ne < 3) {
				if (cal_prox(0) < 4 || cal_prox(7) < 4 || cal_prox(NE) < 2) {
					break;
				}
				setSpeeds(100,400);
				if (cal_prox(NE) > 1.5) {
					setSpeeds(300,100);
				}
				r = cal_prox(E);
				ne = cal_prox(NE);
			}
		}													
	}

	else							//None of those selected
		while(1) NOP();
}

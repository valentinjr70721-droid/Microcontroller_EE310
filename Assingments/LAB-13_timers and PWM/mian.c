#include <xc.h>
#include <stdint.h>
#include "PWM.h"
#include "Configfile.h"

#define _XTAL_FREQ 4000000

// Push button inputs
#define BTN_CENTER PORTBbits.RB5
#define BTN_LEFT   PORTBbits.RB6
#define BTN_RIGHT  PORTBbits.RB7

// Servo PWM limits
#define SERVO_LEFT_LIMIT    22
#define SERVO_MIDDLE_POS    51
#define SERVO_RIGHT_LIMIT   81

_Bool pwmState;

uint8_t servoPos      = SERVO_MIDDLE_POS;
uint8_t previousPos   = SERVO_MIDDLE_POS;
uint8_t updateDelay   = 0;

void main(void)
{
    // Set internal oscillator to 4 MHz
    OSCSTATbits.HFOR = 1;
    OSCFRQ = 0x02;

    // Make PORTB digital
    ANSELB = 0x00;

    // I/O directions
    TRISBbits.TRISB2 = 0;   // PWM output to servo
    TRISBbits.TRISB5 = 1;   // Center button
    TRISBbits.TRISB6 = 1;   // Left button
    TRISBbits.TRISB7 = 1;   // Right button

    // Enable weak pull-ups for buttons
    WPUBbits.WPUB5 = 1;
    WPUBbits.WPUB6 = 1;
    WPUBbits.WPUB7 = 1;

    // Timer2 setup for ~20 ms servo period
    TMR2_Initialize();
    T2PR = 155;
    TMR2_StartTimer();

    // PWM setup
    PWM_Output_D8_Enable();
    PWM2_Initialize();
    PWM2_LoadDutyValue(servoPos);

    while(1)
    {
        // Read PWM output status
        pwmState = PWM2_OutputStatusGet();
        PORTBbits.RB2 = pwmState;

        // Wait for Timer2 overflow
        if(PIR4bits.TMR2IF)
        {
            PIR4bits.TMR2IF = 0;

            updateDelay++;

            // Slow down servo movement slightly
            if(updateDelay >= 2)
            {
                updateDelay = 0;

                // Return to middle position
                if(BTN_CENTER == 0)
                {
                    servoPos = SERVO_MIDDLE_POS;
                }
                // Move left
                else if(BTN_LEFT == 0 && servoPos > SERVO_LEFT_LIMIT)
                {
                    servoPos--;
                }
                // Move right
                else if(BTN_RIGHT == 0 && servoPos < SERVO_RIGHT_LIMIT)
                {
                    servoPos++;
                }

                // Only update PWM if position changed
                if(servoPos != previousPos)
                {
                    PWM2_LoadDutyValue(servoPos);
                    previousPos = servoPos;
                }
            }
        }
    }
}

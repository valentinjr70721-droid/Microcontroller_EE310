/************************************************************
 * PROJECT TITLE: Servo Motor Control using PWM (CCP2)
 *
 * DESCRIPTION:
 * This program controls a servo motor using PWM output on
 * the PIC18F46K42. Two push buttons are used with internal
 * pull-ups to control left and right movement. The servo
 * position is adjusted by changing the PWM duty cycle.
 *
 * MCU: PIC18F46K42
 * COMPILER: MPLAB XC8 (v2)
 * CLOCK: Internal HFINTOSC @ 4 MHz
 *
 * VERSION: 1.0
 * DATE: May 14, 2026
 *
 * AUTHOR: Miguel Valentin
 ************************************************************/

// ==============================
// INPUTS
// ==============================
// RD0 → LEFT push button (active LOW, internal pull-up)
// RD1 → RIGHT push button (active LOW, internal pull-up)
//
// ==============================
// OUTPUTS
// ==============================
// RB3 → PWM output to servo (CCP2)
//
// ==============================
// PWM DETAILS
// ==============================
// PWM Period: ~20 ms
// Prescaler: 1:128
// Servo range: duty values 10–78
// Center: ~47
// ==============================
#include <xc.h>
#include <stdint.h>
#include "PWM.h"
#include "configFile.h"

#define _XTAL_FREQ 4000000

// ==============================
// BUTTONS (internal pull-ups)
// Active LOW when pressed
// ==============================
#define BUTTON_LEFT   PORTDbits.RD0
#define BUTTON_RIGHT  PORTDbits.RD1

// ==============================
// SERVO RANGE (adjusted values)
// ==============================
#define SERVO_MIN      10
#define SERVO_MAX      78

uint8_t duty_value = 47;   // center position

// ==============================
// MAIN
// ==============================
void main(void)
{
    // CLOCK
    OSCSTATbits.HFOR = 1;
    OSCFRQ = 0x02;   // 4 MHz

    // DIGITAL SETUP
    ANSELD = 0x00;   // digital pins only

    TRISDbits.TRISD0 = 1; // input
    TRISDbits.TRISD1 = 1; // input

    // ==============================
    // INTERNAL PULL-UPS ENABLE
    // ==============================
    WPUDbits.WPUD0 = 1;
    WPUDbits.WPUD1 = 1;

    // PORT B OUTPUTS (PWM)
    ANSELB = 0x00;
    TRISB = 0x00;

    // ==============================
    // TIMER2 + PWM SETUP
    // ==============================
    TMR2_Initialize();
    TMR2_StartTimer();

    PWM_Output_D8_Enable();
    PWM2_Initialize();

    PWM2_LoadDutyValue(duty_value);

    // small stabilization delay
    __delay_ms(300);

    // ==============================
    // MAIN LOOP
    // ==============================
    while(1)
    {
        // RIGHT button (pressed = 0)
        if(BUTTON_RIGHT == 0)
        {
            if(duty_value < SERVO_MAX)
                duty_value++;

            PWM2_LoadDutyValue(duty_value);
            __delay_ms(60);   // smoothing + debounce
        }

        // LEFT button (pressed = 0)
        else if(BUTTON_LEFT == 0)
        {
            if(duty_value > SERVO_MIN)
                duty_value--;

            PWM2_LoadDutyValue(duty_value);
            __delay_ms(60);   // smoothing + debounce
        }
    }
}

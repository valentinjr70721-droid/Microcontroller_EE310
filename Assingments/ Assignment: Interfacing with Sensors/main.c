/**********************************************************************
 * Programmer:      Miguel Valentin
 * Program Name:    Touchless Security System with PIC18F47K42
 *
 * Description:
 * This program implements a touchless security system using two 
 * photoresistors (PR1 and PR2) as input sensors. The user enters a 
 * two-digit secret code by covering the photoresistors a specified 
 * number of times. The entered digits are displayed on a 7-segment 
 * display. If the entered code matches the predefined secret code, 
 * a DC motor is activated via a relay. If the code is incorrect, 
 * a buzzer is activated for a short duration.
 * An emergency switch connected to INT0 triggers an interrupt service 
 * routine that plays a distinct buzzer melody and halts normal operation.
 *
 * Hardware Used:
 * - PIC18F47K42 Microcontroller
 * - 2 Photoresistors (PR1, PR2)
 * - 7-Segment Display (Common Cathode)
 * - Relay Module (5V)
 * - DC Motor (separate power supply)
 * - Piezo Buzzer (with transistor driver)
 * - Emergency Pushbutton Switch
 *
 * Inputs:
 * - PR1 (RA0/AN0): First digit input (light-based)
 * - PR2 (RA1/AN1): Second digit input (light-based)
 * - Emergency Switch (RB0/INT0): Interrupt trigger
 *
 * Outputs:
 * - SYS_LED (RC3): Indicates system is powered
 * - 7-Segment Display (PORTD): Displays entered digits
 * - Relay Control (RC7): Activates motor
 * - Buzzer Control (RC6): Audible feedback and emergency alert
 *
 * Development Environment:
 * - MPLAB X IDE v6.30
 * - XC8 Compiler v3.10
 * Version: 1.1v
 **********************************************************************/


#include "config_bits2.h"
#include "init.h"
#include "functions.h"

volatile uint8_t emergency_flag = 0;

void main(void)
{
    uint8_t digit1, digit2;

    init_system();
    SYS_LED_LAT = 1;

    while (1)
    {
        if (emergency_flag)
        {
            motor_off();
            display_blank();

            while (1)
            {
                SYS_LED_LAT = 1;
            }
        }

        // First digit from PR1
        display_blank();
        digit1 = read_digit_from_pr(pr1_dark);
        if (emergency_flag) continue;

        display_digit(digit1);
        __delay_ms(1000);

        // Second digit from PR2
        display_blank();
        digit2 = read_digit_from_pr(pr2_dark);
        if (emergency_flag) continue;

        display_digit(digit2);
        __delay_ms(1000);

        // Check secret code
        if (code_correct(digit1, digit2))
        {
            motor_on();

            while (!emergency_flag)
            {
                SYS_LED_LAT = 1;
            }

            motor_off();
        }
        else
        {
            buzz_error();
        }

        display_blank();
        __delay_ms(500);
    }
}

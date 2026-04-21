#include "functions.h"

// Common cathode 7-segment codes
static const uint8_t segDigits[10] = {
    0x3F, // 0
    0x06, // 1
    0x5B, // 2
    0x4F, // 3
    0x66, // 4
    0x6D, // 5
    0x7D, // 6
    0x07, // 7
    0x7F, // 8
    0x6F  // 9
};

void init_system(void)
{
    // 4 MHz internal oscillator
    OSCFRQ = 0x02;

    // Analog / digital selection
    ANSELA = 0x03;   // RA0 and RA1 are analog
    ANSELB = 0x00;
    ANSELC = 0x00;
    ANSELD = 0x00;
    ANSELE = 0x00;

    // Directions
    SYS_LED_TRIS = 0;
    BUZZER_TRIS  = 0;
    RELAY_TRIS   = 0;
    SEG_TRIS     = 0x00;

    PR1_TRIS = 1;
    PR2_TRIS = 1;
    EMG_TRIS = 1;

    // Initial outputs
    SYS_LED_LAT = 0;
    BUZZER_LAT  = 0;
    RELAY_LAT   = 0;
    SEG_LAT     = 0x00;

    // ADC setup
    ADREF = 0x00;          // VDD/VSS reference
    ADCLK = 0x3F;          // slower ADC clock
    ADCON0bits.FM = 1;     // right-justified result
    ADCON0bits.ON = 1;     // enable ADC

    // INT0 on falling edge
    INTCON0bits.INT0EDG = 0;
    PIR1bits.INT0IF = 0;
    PIE1bits.INT0IE = 1;
    INTCON0bits.GIE = 1;
}

uint16_t adc_read(uint8_t ch)
{
    ADPCH = ch;
    __delay_us(20);

    ADCON0bits.GO = 1;
    while (ADCON0bits.GO);

    return (((uint16_t)ADRESH << 8) | ADRESL);
}

bool pr1_dark(void)
{
    return (adc_read(0x00) < DARK_THRESHOLD);   // AN0
}

bool pr2_dark(void)
{
    return (adc_read(0x01) < DARK_THRESHOLD);   // AN1
}

void display_blank(void)
{
    SEG_LAT = 0x00;
}

void display_digit(uint8_t digit)
{
    if (digit <= 9)
    {
        SEG_LAT = segDigits[digit];
    }
    else
    {
        SEG_LAT = 0x00;
    }
}

void motor_on(void)
{
    RELAY_LAT = 1;
}

void motor_off(void)
{
    RELAY_LAT = 0;
}

void buzzer_on(void)
{
    BUZZER_LAT = 1;
}

void buzzer_off(void)
{
    BUZZER_LAT = 0;
}

void buzz_error(void)
{
    buzzer_on();
    __delay_ms(ERROR_BUZZ_MS);
    buzzer_off();
}

void emergency_melody(void)
{
    uint8_t i;

    for (i = 0; i < 4; i++)
    {
        SYS_LED_LAT = 0;

        BUZZER_LAT = 1;
        __delay_ms(150);
        BUZZER_LAT = 0;
        __delay_ms(80);

        BUZZER_LAT = 1;
        __delay_ms(300);
        BUZZER_LAT = 0;
        __delay_ms(120);

        SYS_LED_LAT = 1;
    }
}

void __interrupt() ISR(void)
{
    if (PIR1bits.INT0IF)
    {
        PIR1bits.INT0IF = 0;
        emergency_flag = 1;
        motor_off();
        emergency_melody();
    }
}

uint8_t read_digit_from_pr(bool (*sensor_dark)(void))
{
    uint8_t count = 0;
    bool prevDark = false;
    uint16_t idle = 0;

    // Wait until uncovered before starting
    while (sensor_dark() && !emergency_flag)
    {
        __delay_ms(20);
    }

    while (!emergency_flag)
    {
        bool nowDark = sensor_dark();

        if (nowDark && !prevDark)
        {
            if (count < 4)
            {
                count++;
                display_digit(count);
            }

            idle = 0;
            __delay_ms(DEBOUNCE_MS);
        }

        prevDark = nowDark;

        if (count > 0)
        {
            __delay_ms(50);
            idle += 50;

            if (idle >= DIGIT_DONE_MS)
            {
                return count;
            }
        }
        else
        {
            __delay_ms(20);
        }
    }

    return 0;
}

bool code_correct(uint8_t d1, uint8_t d2)
{
    return (d1 == SECRET_DIGIT_1) && (d2 == SECRET_DIGIT_2);
}

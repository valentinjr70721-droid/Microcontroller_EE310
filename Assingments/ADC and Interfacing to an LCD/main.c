#include <xc.h>
#include <stdio.h>

#pragma config FEXTOSC = OFF
#pragma config RSTOSC = HFINTOSC_1MHZ
#pragma config WDTE = OFF
#pragma config LVP = OFF

#define _XTAL_FREQ 1000000

// LCD pins
#define RS      LATD0
#define EN      LATD1
#define LCDDATA LATB

#define LCD_TRIS   TRISB
#define CTRL_TRIS  TRISD

// Status LED
#define STATUS_LED LATE0

volatile unsigned char pauseFlag = 0;

/* Function Prototypes */
void LCD_Init(void);
void LCD_Cmd(char cmd);
void LCD_Write(char data);
void LCD_Print(const char *text);
void LCD_SetCursor(char row, char col, const char *text);

void ADC_Setup(void);
unsigned int ReadSoundADC(void);
void ShowSoundLevel(unsigned int level);

void IOC_Setup(void);
void PauseMode(void);

void DelayMS(unsigned int ms);

/* ================= MAIN ================= */

void main(void)
{
    unsigned int soundLevel;
    unsigned long total;
    unsigned char sample;

    // Configure analog/digital pins
    ANSELA = 0x01;
    ANSELB = 0x00;
    ANSELC = 0x00;
    ANSELD = 0x00;
    ANSELE = 0x00;

    TRISE0 = 0;
    TRISC2 = 1;

    STATUS_LED = 0;

    LCD_Init();
    ADC_Setup();
    IOC_Setup();

    while(1)
    {
        // Enter pause state if button interrupt occurred
        if(pauseFlag)
        {
            pauseFlag = 0;
            PauseMode();
        }

        total = 0;

        // Average several ADC readings
        for(sample = 0; sample < 5; sample++)
        {
            total += ReadSoundADC();
            __delay_ms(100);
        }

        soundLevel = total / 5;

        ShowSoundLevel(soundLevel);

        __delay_ms(300);
    }
}

/* ================= IOC INTERRUPT ================= */

void IOC_Setup(void)
{
    WPUCbits.WPUC2 = 1;

    IOCCNbits.IOCCN2 = 1;
    IOCCFbits.IOCCF2 = 0;

    PIR0bits.IOCIF = 0;
    PIE0bits.IOCIE = 1;

    INTCON0bits.IPEN = 0;
    INTCON0bits.GIE = 1;
}

void __interrupt(irq(IOC), base(8)) IOC_ISR(void)
{
    if(IOCCFbits.IOCCF2)
    {
        pauseFlag = 1;
        IOCCFbits.IOCCF2 = 0;
    }

    PIR0bits.IOCIF = 0;
}

/* ================= PAUSE MODE ================= */

void PauseMode(void)
{
    unsigned char count;

    LCD_SetCursor(1, 0, "SYSTEM PAUSED    ");
    LCD_SetCursor(2, 0, "Waiting 10 sec   ");

    // Blink LED for 10 seconds
    for(count = 0; count < 20; count++)
    {
        STATUS_LED = 1;
        __delay_ms(250);

        STATUS_LED = 0;
        __delay_ms(250);
    }

    LCD_Cmd(0x01);
    __delay_ms(2);
}

/* ================= ADC ================= */

void ADC_Setup(void)
{
    TRISA0 = 1;

    ADPCH = 0x00;
    ADREF = 0x00;
    ADCLK = 0x3F;

    ADCON0 = 0x84;
}

unsigned int ReadSoundADC(void)
{
    unsigned int highest = 0;
    unsigned int lowest = 4095;
    unsigned int adcValue;
    unsigned char i;

    // Collect multiple samples
    for(i = 0; i < 200; i++)
    {
        __delay_us(200);

        ADCON0bits.GO = 1;
        while(ADCON0bits.GO);

        adcValue = ((unsigned int)ADRESH << 8) | ADRESL;

        if(adcValue > highest)
            highest = adcValue;

        if(adcValue < lowest)
            lowest = adcValue;
    }

    return (highest - lowest);
}

/* ================= DISPLAY ================= */

void ShowSoundLevel(unsigned int level)
{
    unsigned long voltage;
    char topLine[21];
    char bottomLine[21];

    voltage = ((unsigned long)level * 3300) / 4095;

    if(voltage < 3)
        sprintf(topLine, "Noise: Quiet     ");
    else if(voltage < 10)
        sprintf(topLine, "Noise: Normal    ");
    else if(voltage < 25)
        sprintf(topLine, "Noise: Loud      ");
    else
        sprintf(topLine, "Noise: Obnoxious ");

    sprintf(bottomLine, "%lu mV Detected  ", voltage);

    LCD_SetCursor(1, 0, topLine);
    LCD_SetCursor(2, 0, bottomLine);
}

/* ================= LCD FUNCTIONS ================= */

void LCD_Init(void)
{
    DelayMS(20);

    LCD_TRIS = 0x00;
    CTRL_TRIS = 0x00;

    LCD_Cmd(0x38);
    LCD_Cmd(0x0C);
    LCD_Cmd(0x06);
    LCD_Cmd(0x01);

    DelayMS(2);
}

void LCD_Cmd(char cmd)
{
    LCDDATA = cmd;

    RS = 0;
    EN = 1;
    __delay_us(50);
    EN = 0;

    __delay_ms(2);
}

void LCD_Write(char data)
{
    LCDDATA = data;

    RS = 1;
    EN = 1;
    __delay_us(50);
    EN = 0;

    __delay_ms(1);
}

void LCD_Print(const char *text)
{
    while(*text)
    {
        LCD_Write(*text++);
    }
}

void LCD_SetCursor(char row, char col, const char *text)
{
    char address;

    if(row == 1)
        address = 0x80 + col;
    else
        address = 0xC0 + col;

    LCD_Cmd(address);
    LCD_Print(text);
}

void DelayMS(unsigned int ms)
{
    unsigned int i, j;

    for(i = 0; i < ms; i++)
    {
        for(j = 0; j < 165; j++);
    }
}

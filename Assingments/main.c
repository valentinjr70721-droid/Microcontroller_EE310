#include <xc.h>

#pragma config FEXTOSC = OFF
#pragma config RSTOSC = HFINTOSC_64MHZ
#pragma config WDTE = OFF
#pragma config LVP = ON
#pragma config MCLRE = EXTMCLR
#pragma config BOREN = SBORDIS

#include <stdint.h>
#include <stdio.h>

#define _XTAL_FREQ 64000000UL

#define DHT_LAT     LATBbits.LATB0
#define DHT_PORT    PORTBbits.RB0
#define DHT_TRIS    TRISBbits.TRISB0

#define BUZZER_LAT  LATDbits.LATD1
#define FAN_LAT     LATDbits.LATD2
#define SERVO_LAT   LATDbits.LATD3

#define RED_LAT     LATBbits.LATB4
#define GREEN_LAT   LATBbits.LATB5
#define BLUE_LAT    LATBbits.LATB6

#define LCD_RS LATDbits.LATD4
#define LCD_E  LATDbits.LATD5
#define LCD_D4 LATDbits.LATD6
#define LCD_D5 LATDbits.LATD7
#define LCD_D6 LATBbits.LATB1
#define LCD_D7 LATBbits.LATB2

// Software I2C pins for GY-521 / MPU-6050
#define I2C_SCL_LAT   LATCbits.LATC3
#define I2C_SCL_TRIS  TRISCbits.TRISC3

#define I2C_SDA_LAT   LATCbits.LATC4
#define I2C_SDA_TRIS  TRISCbits.TRISC4
#define I2C_SDA_PORT  PORTCbits.RC4

#define MPU_ADDR 0x68

volatile uint8_t earthquake_alarm = 0;

void clock_init(void) {
    OSCCON1 = 0x60;
    OSCFRQ = 0x08;
}

void gpio_init(void) {
    ANSELA = 0x03;
    ANSELB = 0x00;
    ANSELC = 0x00;
    ANSELD = 0x00;

    TRISAbits.TRISA0 = 1;
    TRISAbits.TRISA1 = 1;

    TRISBbits.TRISB0 = 1;
    TRISBbits.TRISB1 = 0;
    TRISBbits.TRISB2 = 0;
    TRISBbits.TRISB4 = 0;
    TRISBbits.TRISB5 = 0;
    TRISBbits.TRISB6 = 0;

    TRISDbits.TRISD1 = 0;
    TRISDbits.TRISD2 = 0;
    TRISDbits.TRISD3 = 0;
    TRISDbits.TRISD4 = 0;
    TRISDbits.TRISD5 = 0;
    TRISDbits.TRISD6 = 0;
    TRISDbits.TRISD7 = 0;

    // I2C pins start released HIGH through pull-up resistors
    LATCbits.LATC3 = 0;
    LATCbits.LATC4 = 0;
    TRISCbits.TRISC3 = 1;
    TRISCbits.TRISC4 = 1;

    LATB = 0x00;
    LATD = 0x00;
}

// ================= LCD FUNCTIONS =================

void lcd_pulse(void) {
    LCD_E = 1;
    __delay_us(5);
    LCD_E = 0;
    __delay_us(100);
}

void lcd_nibble(uint8_t n) {
    LCD_D4 = (n >> 0) & 1;
    LCD_D5 = (n >> 1) & 1;
    LCD_D6 = (n >> 2) & 1;
    LCD_D7 = (n >> 3) & 1;
    lcd_pulse();
}

void lcd_cmd(uint8_t cmd) {
    LCD_RS = 0;
    lcd_nibble(cmd >> 4);
    lcd_nibble(cmd & 0x0F);
    __delay_ms(2);
}

void lcd_data(uint8_t data) {
    LCD_RS = 1;
    lcd_nibble(data >> 4);
    lcd_nibble(data & 0x0F);
    __delay_us(100);
}

void lcd_init(void) {
    __delay_ms(50);

    LCD_RS = 0;
    LCD_E = 0;

    lcd_nibble(0x03);
    __delay_ms(10);
    lcd_nibble(0x03);
    __delay_ms(10);
    lcd_nibble(0x03);
    __delay_ms(10);
    lcd_nibble(0x02);
    __delay_ms(10);

    lcd_cmd(0x28);
    lcd_cmd(0x0C);
    lcd_cmd(0x06);
    lcd_cmd(0x01);
    __delay_ms(5);
}

void lcd_set_cursor(uint8_t row, uint8_t col) {
    uint8_t address = row ? 0x40 : 0x00;
    lcd_cmd(0x80 | (address + col));
}

void lcd_print_fixed(const char *s) {
    uint8_t count = 0;

    while (*s && count < 16) {
        lcd_data(*s++);
        count++;
    }

    while (count < 16) {
        lcd_data(' ');
        count++;
    }
}

// ================= ADC FUNCTIONS =================

void adc_init(void) {
    ADCON0bits.ADFM = 1;
    ADCON0bits.CS = 1;

    ADREFbits.ADPREF = 0;
    ADREFbits.ADNREF = 0;

    ADCON0bits.ON = 1;
}

uint16_t adc_read(uint8_t channel) {
    ADPCH = channel;
    __delay_us(20);

    ADCON0bits.GO = 1;
    while (ADCON0bits.GO);

    return ((uint16_t)ADRESH << 8) | ADRESL;
}

uint16_t adc_average(uint8_t channel, uint8_t samples) {
    uint32_t total = 0;

    for (uint8_t i = 0; i < samples; i++) {
        total += adc_read(channel);
        __delay_ms(2);
    }

    return (uint16_t)(total / samples);
}

// ================= RGB FUNCTIONS =================

void rgb_off(void) {
    RED_LAT = 0;
    GREEN_LAT = 0;
    BLUE_LAT = 0;
}

void rgb_show_light_level(uint16_t light_adc) {
    rgb_off();

    if (light_adc < 120) {
        BLUE_LAT = 1;
    } else if (light_adc < 200) {
        GREEN_LAT = 1;
    } else {
        RED_LAT = 1;
    }
}

// ================= SOFTWARE I2C =================

void i2c_delay(void) {
    __delay_us(5);
}

void i2c_scl_low(void) {
    I2C_SCL_LAT = 0;
    I2C_SCL_TRIS = 0;
}

void i2c_scl_release(void) {
    I2C_SCL_TRIS = 1;
}

void i2c_sda_low(void) {
    I2C_SDA_LAT = 0;
    I2C_SDA_TRIS = 0;
}

void i2c_sda_release(void) {
    I2C_SDA_TRIS = 1;
}

void i2c_init(void) {
    I2C_SCL_LAT = 0;
    I2C_SDA_LAT = 0;

    i2c_scl_release();
    i2c_sda_release();
}

void i2c_start(void) {
    i2c_sda_release();
    i2c_scl_release();
    i2c_delay();

    i2c_sda_low();
    i2c_delay();

    i2c_scl_low();
    i2c_delay();
}

void i2c_stop(void) {
    i2c_sda_low();
    i2c_delay();

    i2c_scl_release();
    i2c_delay();

    i2c_sda_release();
    i2c_delay();
}

uint8_t i2c_write(uint8_t data) {
    uint8_t ack;

    for (uint8_t i = 0; i < 8; i++) {
        if (data & 0x80) {
            i2c_sda_release();
        } else {
            i2c_sda_low();
        }

        i2c_delay();
        i2c_scl_release();
        i2c_delay();
        i2c_scl_low();
        i2c_delay();

        data <<= 1;
    }

    i2c_sda_release();
    i2c_delay();

    i2c_scl_release();
    i2c_delay();

    ack = I2C_SDA_PORT;

    i2c_scl_low();
    i2c_delay();

    return ack == 0;
}

uint8_t i2c_read(uint8_t ack) {
    uint8_t data = 0;

    i2c_sda_release();

    for (uint8_t i = 0; i < 8; i++) {
        data <<= 1;

        i2c_scl_release();
        i2c_delay();

        if (I2C_SDA_PORT) {
            data |= 1;
        }

        i2c_scl_low();
        i2c_delay();
    }

    if (ack) {
        i2c_sda_low();
    } else {
        i2c_sda_release();
    }

    i2c_delay();
    i2c_scl_release();
    i2c_delay();
    i2c_scl_low();
    i2c_delay();

    i2c_sda_release();

    return data;
}

// ================= MPU-6050 / GY-521 =================

uint8_t mpu_write_reg(uint8_t reg, uint8_t value) {
    i2c_start();

    if (!i2c_write((MPU_ADDR << 1) | 0)) {
        i2c_stop();
        return 0;
    }

    if (!i2c_write(reg)) {
        i2c_stop();
        return 0;
    }

    if (!i2c_write(value)) {
        i2c_stop();
        return 0;
    }

    i2c_stop();
    return 1;
}

uint8_t mpu_read_regs(uint8_t reg, uint8_t *buffer, uint8_t length) {
    i2c_start();

    if (!i2c_write((MPU_ADDR << 1) | 0)) {
        i2c_stop();
        return 0;
    }

    if (!i2c_write(reg)) {
        i2c_stop();
        return 0;
    }

    i2c_start();

    if (!i2c_write((MPU_ADDR << 1) | 1)) {
        i2c_stop();
        return 0;
    }

    for (uint8_t i = 0; i < length; i++) {
        buffer[i] = i2c_read(i < (length - 1));
    }

    i2c_stop();
    return 1;
}

uint8_t mpu_init(void) {
    __delay_ms(100);

    // Wake up MPU-6050. Register 0x6B is power management.
    if (!mpu_write_reg(0x6B, 0x00)) {
        return 0;
    }

    __delay_ms(50);

    // Accelerometer range: +/-2g
    mpu_write_reg(0x1C, 0x00);

    // Gyroscope range: +/-250 deg/s
    mpu_write_reg(0x1B, 0x00);

    return 1;
}

uint8_t mpu_read_accel(int16_t *ax, int16_t *ay, int16_t *az) {
    uint8_t data[6];

    if (!mpu_read_regs(0x3B, data, 6)) {
        return 0;
    }

    *ax = ((int16_t)data[0] << 8) | data[1];
    *ay = ((int16_t)data[2] << 8) | data[3];
    *az = ((int16_t)data[4] << 8) | data[5];

    return 1;
}

uint16_t abs16_diff(int16_t a, int16_t b) {
    int32_t diff = (int32_t)a - (int32_t)b;

    if (diff < 0) {
        diff = -diff;
    }

    if (diff > 65535) {
        diff = 65535;
    }

    return (uint16_t)diff;
}

uint8_t mpu_detect_shake(void) {
    static uint8_t first_read = 1;
    static int16_t last_ax = 0;
    static int16_t last_ay = 0;
    static int16_t last_az = 0;

    int16_t ax;
    int16_t ay;
    int16_t az;

    uint32_t motion;

    if (!mpu_read_accel(&ax, &ay, &az)) {
        return 0;
    }

    if (first_read) {
        last_ax = ax;
        last_ay = ay;
        last_az = az;
        first_read = 0;
        return 0;
    }

    motion = 0;
    motion += abs16_diff(ax, last_ax);
    motion += abs16_diff(ay, last_ay);
    motion += abs16_diff(az, last_az);

    last_ax = ax;
    last_ay = ay;
    last_az = az;

    /*
       Shake threshold.
       Increase this if alarm is too sensitive.
       Decrease this if alarm is not sensitive enough.
    */
    if (motion > 12000UL) {
        return 1;
    }

    return 0;
}

// ================= DHT11 =================

uint8_t dht_read(uint8_t *temp, uint8_t *hum) {
    uint8_t data[5] = {0, 0, 0, 0, 0};
    uint16_t timeout;

    DHT_TRIS = 0;
    DHT_LAT = 0;
    __delay_ms(20);

    DHT_LAT = 1;
    __delay_us(30);
    DHT_TRIS = 1;

    timeout = 0;
    while (DHT_PORT && timeout++ < 10000);
    if (timeout >= 10000) return 0;

    timeout = 0;
    while (!DHT_PORT && timeout++ < 10000);
    if (timeout >= 10000) return 0;

    timeout = 0;
    while (DHT_PORT && timeout++ < 10000);
    if (timeout >= 10000) return 0;

    for (uint8_t i = 0; i < 40; i++) {
        timeout = 0;
        while (!DHT_PORT && timeout++ < 10000);
        if (timeout >= 10000) return 0;

        __delay_us(35);

        if (DHT_PORT) {
            data[i / 8] |= (1 << (7 - (i % 8)));
        }

        timeout = 0;
        while (DHT_PORT && timeout++ < 10000);
        if (timeout >= 10000) return 0;
    }

    if ((uint8_t)(data[0] + data[1] + data[2] + data[3]) != data[4]) {
        return 0;
    }

    *hum = data[0];
    *temp = data[2];

    return 1;
}

// ================= SERVO FUNCTIONS =================

void servo_cold_position(void) {
    SERVO_LAT = 1;
    __delay_us(600);
    SERVO_LAT = 0;
}

void servo_hot_position(void) {
    SERVO_LAT = 1;
    __delay_us(2000);
    SERVO_LAT = 0;
}

void servo_hold_cold(uint8_t cycles) {
    for (uint8_t i = 0; i < cycles; i++) {
        servo_cold_position();
        __delay_ms(20);
    }
}

void servo_hold_hot(uint8_t cycles) {
    for (uint8_t i = 0; i < cycles; i++) {
        servo_hot_position();
        __delay_ms(20);
    }
}

// ================= ALARM =================

void alarm_sequence(void) {
    lcd_set_cursor(0, 0);
    lcd_print_fixed("SHAKE DETECTED");
    lcd_set_cursor(1, 0);
    lcd_print_fixed("ALARM ACTIVE");

    for (uint8_t i = 0; i < 6; i++) {
        BUZZER_LAT = 1;
        RED_LAT = 1;
        GREEN_LAT = 0;
        BLUE_LAT = 0;
        __delay_ms(200);

        BUZZER_LAT = 0;
        RED_LAT = 0;
        __delay_ms(200);
    }
}

// ================= MAIN =================

int main(void) {
    WDTCON0bits.SWDTEN = 0;

    uint16_t light_adc;
    uint16_t pot_adc;

    uint8_t temp = 0;
    uint8_t hum = 0;
    uint8_t dht_ok = 0;

    uint8_t fan_on = 0;
    uint8_t last_temp = 0;
    uint8_t last_hum = 0;
    uint8_t dht_fail_count = 0;

    uint8_t mpu_ok = 0;

    char line1[17];
    char line2[17];

    clock_init();
    gpio_init();
    adc_init();
    lcd_init();
    i2c_init();

    FAN_LAT = 0;
    SERVO_LAT = 0;
    BUZZER_LAT = 0;

    lcd_set_cursor(0, 0);
    lcd_print_fixed("SMART HOME");
    lcd_set_cursor(1, 0);
    lcd_print_fixed("MPU STARTING");

    __delay_ms(1500);

    mpu_ok = mpu_init();

    lcd_set_cursor(0, 0);

    if (mpu_ok) {
        lcd_print_fixed("MPU6050 READY");
    } else {
        lcd_print_fixed("MPU6050 ERROR");
    }

    lcd_set_cursor(1, 0);
    lcd_print_fixed("CHECK WIRING");

    __delay_ms(1500);

    while (1) {
        light_adc = adc_average(0x00, 4);
        pot_adc   = adc_average(0x01, 4);

        rgb_show_light_level(light_adc);

        if (mpu_ok) {
            if (mpu_detect_shake()) {
                alarm_sequence();
            }
        } else {
            mpu_ok = mpu_init();
        }

        dht_ok = dht_read(&temp, &hum);

        if (dht_ok) {
            last_temp = temp;
            last_hum = hum;
            dht_fail_count = 0;
        } else {
            dht_fail_count++;
            temp = last_temp;
            hum = last_hum;
        }

        if (temp >= 31) {
            fan_on = 1;
            FAN_LAT = 1;
        } else {
            fan_on = 0;
            FAN_LAT = 0;
        }

        lcd_set_cursor(0, 0);

        if (dht_fail_count >= 5) {
            snprintf(line1, 17, "DHT ERROR F:%u", fan_on);
        } else {
            snprintf(line1, 17, "T:%02uC H:%02u%% F:%u", temp, hum, fan_on);
        }

        lcd_print_fixed(line1);

        lcd_set_cursor(1, 0);

        if (mpu_ok) {
            snprintf(line2, 17, "L:%04u MPU:OK", light_adc);
        } else {
            snprintf(line2, 17, "L:%04u MPU:ERR", light_adc);
        }

        lcd_print_fixed(line2);

        if (temp >= 31) {
            servo_hold_hot(5);
        } else {
            servo_hold_cold(5);
        }
    }

    return 0;
}

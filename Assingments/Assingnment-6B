;-------------------------------------------
; Title: Assingment 6B - PIC-AS version
; Purpose: Counter Design 
; Compiler: PIC-AS / XC8
; Author: Miguel Valentin
; Outputs: Number Display on 7 Segment (PORTD 0-6)
; Inputs: Switch A (RA0) and B (RA1) inputs
; Version: 
;    V1.0: 4/1/26 - First version 
;-------------------------------------------

#include "AssemblyConfig.inc"
#include <xc.inc>

;-------------------------
; Delay timing values
; (adjusts how fast count changes bigger number  = slower counting)
;-------------------------
d1 EQU 255
d2 EQU 255
d3 EQU 50

;-------------------------
; I/O Mapping
;-------------------------
#define BTN_INC   PORTB,0   ; increment button (active-low 0 = pressed )
#define BTN_DEC   PORTB,1   ; decrement button (active-low)
#define OUTSEG    LATD      ; 7-segment output port

;-------------------------
; RAM locations
;-------------------------
TBL_START EQU 0x20   ; start of lookup table
TBL_END   EQU 0x2F   ; end of lookup table each adress holds a 7 seg pattern 
C1 EQU 0x40          ; delay counter 1 long delay
C2 EQU 0x41          ; delay counter 2 medium delay 
C3 EQU 0x42          ; delay counter 3 small delay 

PSECT absdata,abs,ovrld
ORG 0x30

Boot:
; --- Configure PORTB as digital inputs not alalog ---
BANKSEL ANSELB
CLRF ANSELB ; 
BANKSEL TRISB
MOVLW 0x03       ; RB0, RB1 inputs
MOVWF TRISB
BANKSEL LATB
CLRF LATB

; --- Configure PORTD as digital outputs ---
BANKSEL ANSELD
CLRF ANSELD
BANKSEL TRISD
CLRF TRISD
BANKSEL LATD
CLRF LATD

; --- Load 7-segment hex values into RAM ---
; (0–F patterns)
MOVLW 0x3F
MOVWF TBL_START     ; 0
MOVLW 0x06
MOVWF TBL_START+1   ; 1
MOVLW 0x5B
MOVWF TBL_START+2   ; 2
MOVLW 0x4F
MOVWF TBL_START+3   ; 3
MOVLW 0x66
MOVWF TBL_START+4   ; 4
MOVLW 0x6D
MOVWF TBL_START+5   ; 5
MOVLW 0x7D
MOVWF TBL_START+6   ; 6
MOVLW 0x07
MOVWF TBL_START+7   ; 7
MOVLW 0x7F
MOVWF TBL_START+8   ; 8
MOVLW 0x6F
MOVWF TBL_START+9   ; 9
MOVLW 0x77
MOVWF TBL_START+10  ; A
MOVLW 0x7C
MOVWF TBL_START+11  ; b
MOVLW 0x39
MOVWF TBL_START+12  ; C
MOVLW 0x5E
MOVWF TBL_START+13  ; d
MOVLW 0x79
MOVWF TBL_START+14  ; E
MOVLW 0x71
MOVWF TBL_END       ; F

; --- Initialize pointer to first value (0) ---
LFSR 0, TBL_START ; FSRO now points to start of table (0)
MOVF INDF0, W ;Displays first value of 0 
MOVWF OUTSEG

;=================================================
; Main loop: constantly check buttons endless loop
;===================================================
Run:
; Check increment button first
; (0 = pressed, 1 = not pressed)
BTFSC BTN_INC ; = 0 continue
GOTO CheckDecOnly ; BTN_INC = 1 = skip nect go check DEC

; If INC pressed, check if DEC is also pressed
BTFSC BTN_DEC
GOTO DoIncrement    ; only INC pressed
GOTO ResetAll       ; both pressed → reset

CheckDecOnly:
; If only DEC pressed → go down
BTFSC BTN_DEC
GOTO Run            ; no button pressed
GOTO DoDecrement

;====================================
; Counter control logic
;====================================
DoIncrement:
; If already at max (F), wrap to 0 if not continue
MOVF INDF0, W
CPFSEQ TBL_END
GOTO IncStep
GOTO ResetAll

DoDecrement:
; If already at 0, wrap to F
MOVF INDF0, W
CPFSEQ TBL_START
GOTO DecStep
GOTO JumpMax

IncStep:
INCF FSR0L, F       ; move pointer forward
GOTO UpdateDisplay

DecStep:
DECF FSR0L, F       ; move pointer backward
GOTO UpdateDisplay

JumpMax:
; jump directly to last table entry (F)
LFSR 0, TBL_END
GOTO UpdateDisplay

ResetAll:
; reset pointer back to 0 if imprementing at F 
LFSR 0, TBL_START

;====================================
; Output update
;====================================
UpdateDisplay:
MOVF INDF0, W       ; read current value from table
MOVWF OUTSEG        ; send to 7-seg display
CALL Wait           ; small delay for readability
GOTO Run

;====================================
; Delay routine (triple nested loop)
;====================================
Wait:
MOVLW d1
MOVWF C1

OuterLoop:
MOVLW d2
MOVWF C2

MidLoop:
MOVLW d3
MOVWF C3

InnerLoop:
NOP                 ; small timing adjustment
DECFSZ C3, F
GOTO InnerLoop

DECFSZ C2, F
GOTO MidLoop

DECFSZ C1, F
GOTO OuterLoop

RETURN

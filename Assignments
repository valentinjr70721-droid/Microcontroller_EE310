;---------------------------------
; Title: Assingment 4 
;---------------------------------
; Purpose: Delope a heating and cooling control system
; Dependencies: NONE
; Compiler: MPLAB X IDE v6.30
; Author: Miguel Valentin
; OUTPUTS: Port D1 heating fan and D2 cooling fan
; INPUTS: Reftemp stored in 0x20 and mesuredTemp 0x21
; Versions:
; V1.0: 3/10/26 - First version 

#include <xc.inc>
PSECT resetVec,class=CODE,reloc=2
resetVec:
    goto start	    ;jumps to main program

PSECT code
start:

; Program Inputs
#define measuredTempInput  -5
#define refTempInput       15	    

; Definitions
#define SWITCH LATD,2
#define LED0   PORTD,0
#define LED1   PORTD,1   
  
; Program Constants   
REG10   EQU     10h	;temp storage for refTemp conversion
REG11   EQU     11h	;temp storage for measuredTemp conversion
REG01   EQU     1h

; Main Code initialize PORTD outputs start at low
    CLRF    TRISD
    CLRF    LATD
    
;loading inputs 
    MOVLW   refTempInput
    MOVWF   0x20
    MOVLW   measuredTempInput
    MOVWF   0x21
    
;Comparing measuredTemp and refTemp 
    MOVF    0x20, W        ; = refTemp
    SUBWF   0x21, W        ; = measuredTemp - refTemp
    
    BZ	    TEMPSEQUAL		;measuredTemp = refTemp all off
    BC	    measuredTempGREATER	;measuredTemp > refTemp cooler on
    BRA	    measuredTempLESS	;measurdTemp < refTemp heater on 

;functions
TEMPSEQUAL: ;led off
    CLRF    0x22	;contreg = 0
    BCF	    LATD,1
    BCF	    LATD,2
    BRA	    CONTINUE

;cooling turns ON
measuredTempGREATER: 
    MOVLW   0x02
    MOVWF   0x22	;contreg=2
    BSF	    LATD,2
    BCF	    LATD,1
    BRA	    CONTINUE

;heating turns ON
measuredTempLESS: 
    MOVLW   0x01
    MOVWF   0x22	;contreg=1
    BSF	    LATD,1
    BCF	    LATD,2
    BRA	    CONTINUE
    
CONTINUE:
;HEX to Decimal Conversion (refTemp)
    CLRF    0x60    ;1s 
    CLRF    0x61    ;10s 
    CLRF    0x62    ;100s
;load refTemp and move refTemp
    MOVF    0x20, W	
    MOVWF   REG10	

REFTENLOOP:
    MOVLW   10	
    SUBWF   REG10, F
    BNC	    REFSTORE	;stops at borrow 
    INCF    0x61, F	;increment 10
    BRA	    REFTENLOOP

REFSTORE:
    MOVLW   10
    ADDWF   REG10, F	;resotres last subtraction
    MOVF    REG10, W
    MOVWF   0x60	;stores 1s 

;converting measuredTempInput clr 1 10 100
    CLRF    0x70	
    CLRF    0x71	
    CLRF    0x72	

; loads mesuredTemp and moves
    MOVF    0x21, W	
    MOVWF   REG11	
    
MEASUREDTENLOOP:
    MOVLW   10
    SUBWF   REG11, F
    BNC	    MEASUREDSTORE 
    INCF    0x71, F	    ;increment 10
    BRA	    MEASUREDTENLOOP

MEASUREDSTORE:
    MOVLW   10
    ADDWF   REG11, F
    MOVF    REG11, W
    MOVWF   0x70	;stores 1s 
    
ENDPROGRAM:
    BRA	    ENDPROGRAM    
    END

;********************************************************************************
; ptr.asm: Test av assemblerkod med pekare. Tre lysdioder LED1 - LED3 samt
;          tre tryckknappar BUTTON1 - BUTTON3 ansluts enligt följande:
;
;          - LED1 ansluts till pin 8 (PORTB0).
;          - LED2 ansluts till pin 9 (PORTB1).
;          - LED3 ansluts till pin 10 (PORTB2).
;
;          - BUTTON1 ansluts till pin 11 (PORTB3).
;          - BUTTON2 ansluts till pin 12 (PORTB4).
;          - BUTTON3 ansluts till pin 13 (PORTB5).
;
;          Under programmets gång sker kontinuerligt polling (avläsning) av
;          tryckknapparna, där lysdiodernas utsignaler uppdateras enligt nedan:
;
;          - Vid nedtryckning av BUTTON1 tänds LED1, annars hålls LED1 släckt. 
;          - Vid nedtryckning av BUTTON2 tänds LED2, annars hålls LED2 släckt. 
;          - Vid nedtryckning av BUTTON3 tänds LED3, annars hålls LED3 släckt. 
;********************************************************************************

;********************************************************************************
; Makrodefinitioner:
;********************************************************************************
.EQU LED1 = PORTB0 ; Lysdiod 1 ansluten till pin 8 (PORTB0).
.EQU LED2 = PORTB1 ; Lysdiod 2 ansluten till pin 9 (PORTB1).
.EQU LED3 = PORTB2 ; Lysdiod 3 ansluten till pin 10 (PORTB2).

.EQU BUTTON1 = PORTB3 ; Tryckknapp 1 ansluten till pin 11 (PORTB3).
.EQU BUTTON2 = PORTB4 ; Tryckknapp 2 ansluten till pin 12 (PORTB4).
.EQU BUTTON3 = PORTB5 ; Tryckknapp 3 ansluten till pin 13 (PORTB5).

.EQU RESET_vect = 0x00 ; Programmets startpunkt.

;********************************************************************************
; .CSEG: Kodsegmentet, lagringsplats för programkoden.
;********************************************************************************
.CSEG

;********************************************************************************
; RESET_vect: Programmets startpunkt. Programhopp sker till subrutinen main
;             för att starta programmet.
;********************************************************************************
.ORG 0x00
   RJMP main

;********************************************************************************
; shift_left: Skiftar heltalet (1 << 0) åt vänster angivet antal gånger. 
;             Som exempel, om fem skiftningar ska genomföras skiftas talet
;             fem gånger, vilket medför att talet (1 << 5) returnereras.

;             R16 lagrar det skiftade talet under tiden skiftningen pågår.
;             R17 lagrar antalet genomförda skiftningar. När angivet antal
;             skiftningar har genomförts (R17 == R24) avslutas skiftningen.
;             Det skiftade talet returneras via R24. 
;
;             - R24: Antalet skiftningar som ska genomföras.
;********************************************************************************
shift_left:
   LDI R16, (1 << 0)
   LDI R17, 0x00
shift_left_loop:
   CP R17, R24
   BREQ shift_left_end
   LSL R16
   INC R17
   RJMP shift_left_loop
shift_left_end:
   MOV R24, R16
   RET

;********************************************************************************
; button_is_pressed: Indikerar ifall en given tryckknapp är nedtryckt genom
;                    att returnera 1 (sant) eller (0) via R24.
;
;                    Pekaren till tryckknappens pinregister kopieras till
;                    pekarregister Z för avläsning. Tryckknappens pin kopieras
;                    över till R24 och skiftas åt vänster inför avläsningen. 
;
;                    Pinregistrets innehåll hämtas via Z och lagras i R16.
;                    Samtliga bitar förutom tryckknappens maskeras för att 
;                    enbart kontrollera tryckknappens insignal via bitvis AND. 
;                    Om tryckknappen är nedtryckt returneras 1, annars 0.
;
;                    - [R24:R25]: Pekare till tryckknappens pinregister.
;                    - R22      : Tryckknappens pin-nummer.
;********************************************************************************
button_is_pressed:
   MOVW Z, R24   
   MOV R24, R22
   RCALL shift_left
   LD R16, Z
   AND R16, R24
   BRNE button_is_pressed_return_1 
button_is_pressed_return_0:
   CLR R24
   RET
button_is_pressed_return_1:
   LDI R24, 0x01
   RET

;********************************************************************************
; led_on: Tänder lysdiod ansluten till specificerad pin utan att påverka
;         övriga pinnar på aktuell I/O-port.
;
;         Pekaren till lysdiodens portregistret kopieras till pekarregister Z
;         för läsning och skrivning. Lysdiodens pin kopieras över till R24 
;         och skiftas åt vänster inför avläsningen. 
;
;         Portregistrets innehåll hämtas via Z och lagras i R16. 
;         Lysdiodens pin ettställs via bitvis OR. Det uppdaterade 
;         innehållet skrivs slutligen tillbaka till portregistret via Z.
;
;         - R24:R25: Pekare till portregistret.
;         - R22    : Lysdiodens pin-nummer.
;********************************************************************************
led_on:
   MOVW Z, R24
   MOV R24, R22
   RCALL shift_left
   LD R16, Z
   OR R16, R24
   ST Z, R16
   RET
   
;********************************************************************************
; led_off: Släcker lysdiod ansluten till specificerad pin utan att påverka
;          övriga pinnar på aktuell I/O-port.
;
;          Pekaren till lysdiodens portregistret kopieras till pekarregister Z
;          för läsning och skrivning. Lysdiodens pin kopieras över till R24 
;          och skiftas åt vänster inför avläsningen. Det shiftade värdet
;          inverteras via bitvis OR med värdet 0xFF, som lagras temporärt i R16.
;
;          Portregistrets innehåll hämtas via Z och lagras i R16. 
;          Lysdiodens pin nollställs via bitvis AND. Det uppdaterade 
;          innehållet skrivs slutligen tillbaka till portregistret via Z.
;
;         - R24:R25: Pekare till portregistret.
;         - R22    : Lysdiodens pin-nummer.
;********************************************************************************
led_off:
   MOVW Z, R24
   MOV R24, R22
   RCALL shift_left
   LDI R16, 0xFF
   EOR R24, R16
   LD R16, Z
   AND R16, R24
   ST Z, R16
   RET
  
;********************************************************************************
; main: Initierar systemet vid start. Programmet hålls sedan igång så länge
;       matningsspänning tillförs. Varje knapp kontrolleras kontinuerligt
;       och motsvarande lysdiod tänds vid nedtryckning.
;
;       - Notering: PINB ligger på adressen 0x23.
;                   PORTB ligger på adressen 0x25.
;********************************************************************************
main:
   LDI R16, (1 << LED1) | (1 << LED2) | (1 << LED3)
   OUT DDRB, R16
   LDI R17, (1 << BUTTON1) | (1 << BUTTON2) | (1 << BUTTON3)
   OUT PORTB, R17

;********************************************************************************
; main_loop: Pollar kontinuerligt tryckknapparna och tänder en given 
;            lysdioderna vid nedtryckning av motsvarande tryckknapp.
;********************************************************************************
main_loop:

;********************************************************************************
; check_button1: Genomför polling av BUTTON1. Vid nedtryckning tänds LED1, 
;                annars hålls LED1 släckt. Oavsett utfall pollas sedan BUTTON2.
;********************************************************************************
check_button1:
   LDI R24, 0x23
   LDI R25, 0x00
   LDI R22, BUTTON1
   RCALL button_is_pressed
   MOV R16, R24
   LDI R24, 0x25
   LDI R25, 0x00
   LDI R22, LED1
   CPI R16, 0x01
   BREQ led1_on_branch
led1_off_branch:
   RCALL led_off
   RJMP check_button2
led1_on_branch:
   RCALL led_on

;********************************************************************************
; check_button2: Genomför polling av BUTTON2. Vid nedtryckning tänds LED2, 
;                annars hålls LED2 släckt. Oavsett utfall pollas sedan BUTTON3.
;********************************************************************************
check_button2:
   LDI R24, 0x23
   LDI R25, 0x00
   LDI R22, BUTTON2
   RCALL button_is_pressed
   MOV R16, R24
   LDI R24, 0x25
   LDI R25, 0x00
   LDI R22, LED2
   CPI R16, 0x01
   BREQ led2_on_branch
led2_off_branch:
   RCALL led_off
   RJMP check_button3
led2_on_branch:
   RCALL led_on
 
;********************************************************************************
; check_button3: Genomför polling av BUTTON3. Vid nedtryckning tänds LED3, 
;                annars hålls LED3 släckt. Efter att LED3 har uppdaterats
;                återstartas loopen, vilket medför att BUTTON1 pollas härnäst.
;********************************************************************************  
check_button3:
   LDI R24, 0x23
   LDI R25, 0x00
   LDI R22, BUTTON3
   RCALL button_is_pressed
   MOV R16, R24
   LDI R24, 0x25
   LDI R25, 0x00
   LDI R22, LED3
   CPI R16, 0x01
   BREQ led3_on_branch
led3_off_branch:
   RCALL led_off
   RJMP main_loop
led3_on_branch:
   RCALL led_on
   RJMP main_loop
   
   
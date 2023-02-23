;********************************************************************************
; ptr.asm: Test av assemblerkod med pekare. Tre lysdioder LED1 - LED3 samt
;          tre tryckknappar BUTTON1 - BUTTON3 ansluts enligt f�ljande:
;
;          - LED1 ansluts till pin 8 (PORTB0).
;          - LED2 ansluts till pin 9 (PORTB1).
;          - LED3 ansluts till pin 10 (PORTB2).
;
;          - BUTTON1 ansluts till pin 11 (PORTB3).
;          - BUTTON2 ansluts till pin 12 (PORTB4).
;          - BUTTON3 ansluts till pin 13 (PORTB5).
;
;          Under programmets g�ng sker kontinuerligt polling (avl�sning) av
;          tryckknapparna, d�r lysdiodernas utsignaler uppdateras enligt nedan:
;
;          - Vid nedtryckning av BUTTON1 t�nds LED1, annars h�lls LED1 sl�ckt. 
;          - Vid nedtryckning av BUTTON2 t�nds LED2, annars h�lls LED2 sl�ckt. 
;          - Vid nedtryckning av BUTTON3 t�nds LED3, annars h�lls LED3 sl�ckt. 
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
; .CSEG: Kodsegmentet, lagringsplats f�r programkoden.
;********************************************************************************
.CSEG

;********************************************************************************
; RESET_vect: Programmets startpunkt. Programhopp sker till subrutinen main
;             f�r att starta programmet.
;********************************************************************************
.ORG 0x00
   RJMP main

;********************************************************************************
; shift_left: Skiftar heltalet (1 << 0) �t v�nster angivet antal g�nger. 
;             Som exempel, om fem skiftningar ska genomf�ras skiftas talet
;             fem g�nger, vilket medf�r att talet (1 << 5) returnereras.

;             R16 lagrar det skiftade talet under tiden skiftningen p�g�r.
;             R17 lagrar antalet genomf�rda skiftningar. N�r angivet antal
;             skiftningar har genomf�rts (R17 == R24) avslutas skiftningen.
;             Det skiftade talet returneras via R24. 
;
;             - R24: Antalet skiftningar som ska genomf�ras.
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
; button_is_pressed: Indikerar ifall en given tryckknapp �r nedtryckt genom
;                    att returnera 1 (sant) eller (0) via R24.
;
;                    Pekaren till tryckknappens pinregister kopieras till
;                    pekarregister Z f�r avl�sning. Tryckknappens pin kopieras
;                    �ver till R24 och skiftas �t v�nster inf�r avl�sningen. 
;
;                    Pinregistrets inneh�ll h�mtas via Z och lagras i R16.
;                    Samtliga bitar f�rutom tryckknappens maskeras f�r att 
;                    enbart kontrollera tryckknappens insignal via bitvis AND. 
;                    Om tryckknappen �r nedtryckt returneras 1, annars 0.
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
; led_on: T�nder lysdiod ansluten till specificerad pin utan att p�verka
;         �vriga pinnar p� aktuell I/O-port.
;
;         Pekaren till lysdiodens portregistret kopieras till pekarregister Z
;         f�r l�sning och skrivning. Lysdiodens pin kopieras �ver till R24 
;         och skiftas �t v�nster inf�r avl�sningen. 
;
;         Portregistrets inneh�ll h�mtas via Z och lagras i R16. 
;         Lysdiodens pin ettst�lls via bitvis OR. Det uppdaterade 
;         inneh�llet skrivs slutligen tillbaka till portregistret via Z.
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
; led_off: Sl�cker lysdiod ansluten till specificerad pin utan att p�verka
;          �vriga pinnar p� aktuell I/O-port.
;
;          Pekaren till lysdiodens portregistret kopieras till pekarregister Z
;          f�r l�sning och skrivning. Lysdiodens pin kopieras �ver till R24 
;          och skiftas �t v�nster inf�r avl�sningen. Det shiftade v�rdet
;          inverteras via bitvis OR med v�rdet 0xFF, som lagras tempor�rt i R16.
;
;          Portregistrets inneh�ll h�mtas via Z och lagras i R16. 
;          Lysdiodens pin nollst�lls via bitvis AND. Det uppdaterade 
;          inneh�llet skrivs slutligen tillbaka till portregistret via Z.
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
; main: Initierar systemet vid start. Programmet h�lls sedan ig�ng s� l�nge
;       matningssp�nning tillf�rs. Varje knapp kontrolleras kontinuerligt
;       och motsvarande lysdiod t�nds vid nedtryckning.
;
;       - Notering: PINB ligger p� adressen 0x23.
;                   PORTB ligger p� adressen 0x25.
;********************************************************************************
main:
   LDI R16, (1 << LED1) | (1 << LED2) | (1 << LED3)
   OUT DDRB, R16
   LDI R17, (1 << BUTTON1) | (1 << BUTTON2) | (1 << BUTTON3)
   OUT PORTB, R17

;********************************************************************************
; main_loop: Pollar kontinuerligt tryckknapparna och t�nder en given 
;            lysdioderna vid nedtryckning av motsvarande tryckknapp.
;********************************************************************************
main_loop:

;********************************************************************************
; check_button1: Genomf�r polling av BUTTON1. Vid nedtryckning t�nds LED1, 
;                annars h�lls LED1 sl�ckt. Oavsett utfall pollas sedan BUTTON2.
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
; check_button2: Genomf�r polling av BUTTON2. Vid nedtryckning t�nds LED2, 
;                annars h�lls LED2 sl�ckt. Oavsett utfall pollas sedan BUTTON3.
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
; check_button3: Genomf�r polling av BUTTON3. Vid nedtryckning t�nds LED3, 
;                annars h�lls LED3 sl�ckt. Efter att LED3 har uppdaterats
;                �terstartas loopen, vilket medf�r att BUTTON1 pollas h�rn�st.
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
   
   
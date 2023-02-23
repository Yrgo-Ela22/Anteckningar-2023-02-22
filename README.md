# Anteckningar 2022-02-22
Implementering av pekare i assembler samt implementering av pekare för CPU-emulator via instruktioner ST, LD, STIO samt LDIO.

Filen "ptr.asm" demonstrerar assemblerkoden innefattande pekare som skrevs som övningsuppgift.
Subrutiner led_on, led_off samt button_is_pressed implementerades. För samtliga subrutiner passerades
en pekare till ett register, antingen pinregister PINB eller portregister PORTB, samt ett pin-nummer.

Programmet testades genom att tre lysdioder LED1 - LED3 anslöts till pin 8 - 10 (PORTB0 - PORTB2) och
tre tryckknappar BUTTON1 - BUTTON3 anslöts till pin 11 - 13 (PORTB3 - PORTB5). 

Under programmets gång genomfördes kontinuerligt polling (avläsning) av tryckknapparna, 
där lysdiodernas utsignaler uppdaterades enligt nedan:

   - Vid nedtryckning av BUTTON1 tändes LED1, annars hölls LED1 släckt. 
   - Vid nedtryckning av BUTTON2 tändes LED2, annars hölls LED2 släckt. 
   - Vid nedtryckning av BUTTON3 tändes LED3, annars hölls LED3 släckt. 
   
Filen "ptr.c" innehåller motsvarande C-kod.

Övriga .c- och .h-filer utgörs av CPU-emulatorn med implementering av pekare.
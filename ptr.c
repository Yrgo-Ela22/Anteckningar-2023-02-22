/********************************************************************************
* ptr.c: Programkod innefattande pekare till I/O-register. Tre lysdioder 
*        LED1 - LED3 samt tre tryckknappar BUTTON1 - BUTTON3 ansluts enligt 
*        följande:
*
*        - LED1 ansluts till pin 8 (PORTB0).
*        - LED2 ansluts till pin 9 (PORTB1).
*        - LED3 ansluts till pin 10 (PORTB2).
*
*        - BUTTON1 ansluts till pin 11 (PORTB3).
*        - BUTTON2 ansluts till pin 12 (PORTB4).
*        - BUTTON3 ansluts till pin 13 (PORTB5).
*
*        Under programmets gång sker kontinuerligt polling (avläsning) av
*        tryckknapparna, där lysdiodernas utsignaler uppdateras enligt nedan:
*
*        - Vid nedtryckning av BUTTON1 tänds LED1, annars hålls LED1 släckt.
*        - Vid nedtryckning av BUTTON2 tänds LED2, annars hålls LED2 släckt.
*        - Vid nedtryckning av BUTTON3 tänds LED3, annars hålls LED3 släckt.
*
*        Inline-funktioner används, vilket medför att kompilatorn uppmuntras 
*        att klistra in funktionernas innehåll inline. Notera i den kompilator-
*        genererade koden att detta genomförs (se Disassebly under simulering).
*        Ta bort nyckelordet inline för att genomföra funktionsanrop med
*        adresser till I/O-register med mera.
********************************************************************************/
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>

/********************************************************************************
* Makrodefinitioner:
********************************************************************************/
#define LED1 PORTB0    /* Lysdiod 1 ansluten till pin 8 (PORTB0). */
#define LED2 PORTB1    /* Lysdiod 2 ansluten till pin 9 (PORTB1). */
#define LED3 PORTB2    /* Lysdiod 3 ansluten till pin 10 (PORTB2). */

#define BUTTON1 PORTB3 /* Tryckknapp 1 ansluten till pin 11 (PORTB3). */
#define BUTTON2 PORTB4 /* Tryckknapp 2 ansluten till pin 12 (PORTB4). */
#define BUTTON3 PORTB5 /* Tryckknapp 3 ansluten till pin 13 (PORTB5). */

/********************************************************************************
* led_on: Tänder lysdiod ansluten till angiven pin.
*
*         - reg: Referens till lysdiodens portregister.
*         - pin: Lysdiodens pin-nummer.
********************************************************************************/
static inline void led_on(volatile uint8_t* reg,
                          const uint8_t pin)
{
   *reg |= (1 << pin);
   return;
}

/********************************************************************************
* led_off: Släcker lysdiod ansluten till angiven pin.
*
*         - reg: Referens till lysdiodens portregister.
*         - pin: Lysdiodens pin-nummer.
********************************************************************************/
static void led_off(volatile uint8_t* reg,
                    const uint8_t pin)
{
   *reg &= ~(1 << pin);
   return;
}

/********************************************************************************
* button_is_pressed: Indikerar nedtryckning av angiven tryckknapp genom att
*                    returnera true eller false.
*
*                    - reg: Referens till tryckknappens pinregister.
*                    - pin: Tryckknappens pin-nummer.
********************************************************************************/
static bool button_is_pressed(const volatile uint8_t* reg,
                              const uint8_t pin)
{
   return (bool)(*reg & (1 << pin));
}

/********************************************************************************
* main: Initierar systemet vid start. Programmet hålls sedan igång så länge
*       matningsspänning tillförs. Varje knapp kontrolleras kontinuerligt
*       och motsvarande lysdiod tänds vid nedtryckning.
********************************************************************************/
int main(void)
{
   DDRB = (1 << LED1) | (1 << LED2) | (1 << LED3);
   PORTB = (1 << BUTTON1) | (1 << BUTTON2) | (1 << BUTTON3);

   while (1)
   {
      if (button_is_pressed(&PINB, BUTTON1)) led_on(&PORTB, LED1);
      else                                   led_off(&PORTB, LED1);

      if (button_is_pressed(&PINB, BUTTON2)) led_on(&PORTB, LED2);
      else                                   led_off(&PORTB, LED2);

      if (button_is_pressed(&PINB, BUTTON3)) led_on(&PORTB, LED3);
      else                                   led_off(&PORTB, LED3);
   }

   return 0;
}

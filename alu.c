/********************************************************************************
* alu.c: Contains function declarations for implementation of an ALU
*        (Aritmetic Logic Unit) for performing calculations and updating
*        status bits SNZVC as described below:
*
*        S (Signed)  : Set if result is negative with overflow considered*.
*        N (Negative): Set if result is negative, i.e. N = result[7].
*        Z (Zero)    : Set if result is zero, i.e. Z = result == 0 ? 1 : 0.
*        V (Overflow): Set if signed overflow occurs**.
*        C (Carry)   : Set if result contains a carry bit, i.e. C = result[8]***.
*
*        * Signed flag is set if result is negative (N = 1) while
*          overflow hasn't occured (V = 0) or result is positive (N = 0)
*          while overflow has occured (V = 1), i.e. S = N ^ V.
*
*          For instance, consider the subtracting two 8-bit numbers -100 and 50.
*          The result is -100 - 50 = -150, but since only 8 bits are used, the
*          result is implemented as -150 + 2^8 = -150 + 256 = 106, i.e.
*          0110 0110. Since most significant bit is cleared, the N-flag is
*          cleared and the number if positive. However, overflow occured,
*          since the two numbers -100 and 50 have different signs and the
*          result has the same sign as the subtrahend 50. Hence the V-flag
*          is set. Since N = 0 and V = 1, the S-flag is also set.
*          Therefore the number is correctly intepreted as negative.
*
*        ** Signed overflow occurs:
*
*           a) During addition (+) if the operands A and B are of the
*              same sign and the result is of the opposite sign, i.e.
*
*              V = (A[7] == B[7]) && (A[7] != result[7]) ? 1 : 0
*
*           b) During subtraction (-) if the operands A and B are of the
*              opposite sign and the result has the same sign as B, i.e.
*
*              V = (A[7] != B[7]) && (B[7] == result[7]) ? 1 : 0
*
*        *** One instance when the carry bit is set is when unsigned overflow
*            occurs, for instance when adding two numbers 255 and 1 into an
*            8 bit destination. The result is equal to 0 (with carry set),
*            since 1111 1111 + 1 = 1 0000 0000, which gets truncated to
*            0000 0000. Since result[8] == 1, the carry bit is set.
*            Unsigned overflow occurs for the timer circuits of microcontroller
*            ATmega328P when counting up in Normal Mode.
********************************************************************************/
#include "alu.h"

/********************************************************************************
* alu: Performs calculation with specified operands and returns the result.
*      The status flags SNZVC of the referenced status register are updated
*      in accordance with the result.
*
*      - operation: The operation to perform (OR, AND, XOR, ADD or SUB).
*      - a        : First operand.
*      - b        : Second operand.
*      - sr       : Reference to status register containing SNZVC flags.
********************************************************************************/
uint8_t alu(const uint8_t operation,
            const uint8_t a,
            const uint8_t b,
            uint8_t* sr)
{
   uint16_t result = 0x00;
   *sr &= ~((1 << S) | (1 << N) | (1 << Z) | (1 << V) | (1 << C));

   switch (operation)
   {
      case OR:
      {
         result = a | b; 
         break;
      }
      case AND:
      {
         result = a & b;
         break;
      }
      case XOR:
      {
         result = a ^ b;
         break;
      }
      case ADD:
      {
         result = a + b;

         if ((read(a, 7) == read(b, 7)) && (read(result, 7) != read(a, 7)))
         {
            set(*sr, V);
         }
         break;
      }
      case SUB:
      {
         result = a + (256 - b); /* 256 - b is the 2-complement representation of B. */

         if ((read(a, 7) == read(256 - b, 7)) && (read(result, 7) != read(a, 7)))
         {
            set(*sr, V);
         }
         break;
      }
   }

   if (read(result, 7) == 1)         set(*sr, N);
   if ((uint8_t)(result) == 0)       set(*sr, Z);
   if (read(result, 8) == 1)         set(*sr, C);
   if (read(*sr, N) != read(*sr, V)) set(*sr, S);

   return (uint8_t)(result);
}
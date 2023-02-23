/********************************************************************************
* control_unit.c: Contains static variables and function definitions for 
*                 implementation of an 8-bit control unit.
********************************************************************************/
#include "control_unit.h"

/* Static functions: */
static void monitor_interrupts(void);
static void check_for_irq(void);
static void generate_interrupt(const uint8_t interrupt_vector);

static inline void monitor_pcint0(void);
static inline void monitor_pcint1(void);
static inline void monitor_pcint2(void);

/* Static variables: */
static uint32_t ir; /* Instruction register, stores next instruction to execute. */
static uint8_t pc;  /* Program counter, stores address to next instruction to fetch. */
static uint8_t mar; /* Memory address register, stores address for current instruction. */
static uint8_t sr;  /* Status register, stores status bits ISNZVC. */

static uint8_t op_code; /* Stores OP-code, for example LDI, OUT, JMP etc. */
static uint8_t op1;     /* Stores first operand, most often a destination. */
static uint8_t op2;     /* Stores second operand, most often a value or read address. */

static enum cpu_state state;                    /* Stores current state. */
static uint8_t reg[CPU_REGISTER_ADDRESS_WIDTH]; /* CPU-registers R0 - R31. */

static uint8_t pinb_previous; /* Stores previous input values of PINB (for monitoring). */
static uint8_t pinc_previous; /* Stores previous input values of PINC (for monitoring). */
static uint8_t pind_previous; /* Stores previous input values of PIND (for monitoring). */

/********************************************************************************
* control_unit_reset: Resets control unit registers and corresponding program.
********************************************************************************/
void control_unit_reset(void)
{
   ir = 0x00;
   pc = 0x00;
   mar = 0x00;
   sr = 0x00;

   op_code = 0x00;
   op1 = 0x00;
   op2 = 0x00;

   state = CPU_STATE_FETCH;

   pinb_previous = 0x00;
   pinc_previous = 0x00;
   pind_previous = 0x00;

   for (uint8_t i = 0; i < CPU_REGISTER_ADDRESS_WIDTH; ++i)
   {
      reg[i] = 0x00;
   }

   
   data_memory_reset();
   stack_reset();
   program_memory_write();
   return;
}

/********************************************************************************
* control_unit_run_next_state: Runs next state in the CPU instruction cycle:
********************************************************************************/
void control_unit_run_next_state(void)
{
   switch (state)
   {
      case CPU_STATE_FETCH:
      {
         ir = program_memory_read(pc); /* Fetches next instruction. */
         mar = pc;                     /* Stores address of current instruction. */
         pc++;                         /* Program counter points to next instruction. */
         state = CPU_STATE_DECODE;     /* Decodes the instruction during next clock cycle. */
         break;
      }
      case CPU_STATE_DECODE:
      {        
         op_code = ir >> 16;           /* Bit 23 downto 16 consists of the OP code. */
         op1 = ir >> 8;                /* Bit 15 downto 8 consists of the first operand. */
         op2 = ir;                     /* Bit 7 downto 0 consists of the second operand. */
         state = CPU_STATE_EXECUTE;    /* Executes the instruction during next clock cycle. */
         break;
      }
      case CPU_STATE_EXECUTE:
      {
         switch (op_code) /* Checks the OP code.*/
         {
            case NOP: /* NOP => do nothing. */
            {
               break; 
            }
            case LDI: /* Loads constant into specified CPU register. */
            {
               reg[op1] = op2; 
               break;
            }
            case MOV: /* Copies value to specified CPU register. */
            {
               reg[op1] = reg[op2]; 
               break;
            }
            case OUT: /* Writes value to I/O location (address 0 - 255) in data memory. */
            {
               data_memory_write(op1, reg[op2]); 
               break;
            }
            case IN: /* Reads value from I/O location (address 0 - 255) in data memory. */
            {
               reg[op1] = data_memory_read(op2); 
               break;
            }
            case STS: /* Stores value to data memory (address 256 - 511, hence an offset of 256). */
            {
               data_memory_write(op1 + 256, reg[op2]); 
               break;
            }
            case LDS: /* Loads value from data memory (address 256 - 511, hence an offset of 256). */
            {
               reg[op1] = data_memory_read(op2 + 256); 
               break;
            }
            case CLR: /* Clears content of CPU register. */
            {
               reg[op1] = 0x00; 
               break;
            }
            case ORI: /* Performs bitwise OR with a constant. */
            {
               reg[op1] = alu(OR, reg[op1], op2, &sr);
               break;
            }
            case ANDI: /* Performs bitwise AND with a constant. */
            {
               reg[op1] = alu(AND, reg[op1], op2, &sr);
               break;
            }
            case XORI: /* Performs bitwise XOR with a constant. */
            {
               reg[op1] = alu(XOR, reg[op1], op2, &sr);
               break;
            }
            case OR: /* Performs bitwise OR with content in CPU register. */
            {
               reg[op1] = alu(OR, reg[op1], reg[op2], &sr);
               break;
            }
            case AND: /* Performs bitwise AND with content in CPU register. */
            {
               reg[op1] = alu(AND, reg[op1], reg[op2], &sr);
               break;
            }
            case XOR: /* Performs bitwise AND with content in CPU register. */
            {
               reg[op1] = alu(XOR, reg[op1], reg[op2], &sr);
               break;
            }
            case ADDI: /* Performs addition with a constant. */
            {
               reg[op1] = alu(ADD, reg[op1], op2, &sr);
               break;
            }
            case SUBI: /* Performs subtraction with a constant. */
            {
               reg[op1] = alu(SUB, reg[op1], op2, &sr);
               break;
            }
            case ADD: /* Performs addition with a CPU register. */
            {
               reg[op1] = alu(ADD, reg[op1], reg[op2], &sr);
               break;
            }
            case SUB: /* Performs subtraction with a CPU register. */
            {
               reg[op1] = alu(SUB, reg[op1], reg[op2], &sr);
               break;
            }
            case INC: /* Increments content of a CPU register. */
            {
               reg[op1] = alu(ADD, reg[op1], 1, &sr);
               break;
            }
            case DEC: /* Decrements content of a CPU register. */
            {
               reg[op1] = alu(SUB, reg[op1], 1, &sr);
               break;
            }
            case CPI: /* Compares content between CPU register with a constant. */
            {
               (void)alu(SUB, reg[op1], op2, &sr); /* Return value is not stored. */
               break;
            }
            case CP: /* Compares content between two CPU registers. */
            {
               (void)alu(SUB, reg[op1], reg[op2], &sr); /* Return value is not stored. */
               break;
            }
            case JMP: /* Jumps to specified address. */
            {
               pc = op1;
               break;
            }
            case BREQ: /* Branches to specified address i Z flag is set. */
            {
               if (read(sr, Z)) pc = op1;
               break;
            }
            case BRNE: /* Branches to specified address if Z flag is cleared. */
            {
               if (!read(sr, Z)) pc = op1;
               break;
            }
            case BRGE: /* Branches to specified address if S flag is cleared. */
            {
               if (!read(sr, S)) pc = op1;
               break;
            }
            case BRGT: /* Branches to specified address if both S and Z flags are cleared. */
            {
               if (!read(sr, S) && !read(sr, Z)) pc = op1;
               break;
            }
            case BRLE: /* Branches to specified address if S or Z flag is set. */
            {
               if (read(sr, S) || read(sr, Z)) pc = op1;
               break;
            }
            case BRLT: /* Branches to specified address if S flag is set. */
            {
               if (read(sr, S)) pc = op1;
               break;
            }
            case CALL: /* Stores the return address on the stack and jumps to specified address. */
            {
               stack_push(pc);
               pc = op1;
               break;
            }
            case RET: /* Jumps to return address stored on the stack. */
            {
               pc = stack_pop();
               break;
            }
            case RETI: /* Pops the return address from the stack and sets the global interrupt flag. */
            {
               pc = stack_pop();
               set(sr, I);
               break;
            }
            case PUSH: /* Stores content of specified CPU register on the stack. */
            {
               stack_push(reg[op1]);
               break;
            }
            case POP: /* Loads value from the stack to a CPU-register. */
            {
               reg[op1] = stack_pop();
               break;
            }
            case LSL: /* Shifts content of CPU register on step to the left. */
            {
               reg[op1] = reg[op1] << 1;
               break;
            }
            case LSR: /* Shifts content of CPU register on step to the right. */
            {
               reg[op1] = reg[op1] >> 1;
               break;
            }
            case SEI: /* Sets the global interrupt flag in the status register. */
            {
               set(sr, I);
               break;
            }
            case CLI: /* Clears the global interrupt flag in the status register. */
            {
               clr(sr, I);
               break;
            }
            case STIO:  /* Stores value to referenced I/O location (no offset). */
            {
               const uint16_t address = reg[op1] | (reg[op1 + 1] << 8);
               data_memory_write(address, reg[op2]);
               break;
            }
            case LDIO: /* Loads value from referenced I/O location (no offset). */
            {
               const uint16_t address = reg[op2] | (reg[op2 + 1] << 8);
               reg[op1] = data_memory_read(address);
               break;
            }
            case ST: /* Stores value to referenced data location (offset = 256). */
            {
               const uint16_t address = reg[op1] | (reg[op1 + 1] << 8);
               data_memory_write(address + 256, reg[op2]);
               break;
            }
            case LD: /* Loads value from referenced data location (offset = 256). */
            {
               const uint16_t address = reg[op2] | (reg[op2 + 1] << 8);
               reg[op1] = data_memory_read(address + 256);
               break;
            }
            default:
            {
               control_unit_reset(); /* System reset if error occurs. */
               break;
            }
         }

         state = CPU_STATE_FETCH;    /* Fetches next instruction during next clock cycle. */
         check_for_irq();            /* Checks for interrupt request after each execute cycle. */
         break;
      }
      default:                       /* System reset if error occurs. */
      {
         control_unit_reset();
         break;
      }
   }

   monitor_interrupts();            /* Monitors interrupts each clock cycle. */
   return;
}

/********************************************************************************
* control_unit_run_next_state: Runs next CPU instruction cycle, i.e. fetches
*                              a new instruction from program memory, decodes
*                              and executes it.
********************************************************************************/
void control_unit_run_next_instruction_cycle(void)
{
   do
   {
      control_unit_run_next_state();
   } while (state != CPU_STATE_EXECUTE);
   return;
}

/********************************************************************************
* control_unit_print: Prints information about the processor, for instance
*                     current subroutine, instruction, state, content in
*                     CPU-registers and I/O registers DDRB, PORTB and PINB.
********************************************************************************/
void control_unit_print(void)
{
   printf("--------------------------------------------------------------------------------\n");
   printf("Current subroutine:\t\t\t\t%s\n", program_memory_subroutine_name(mar));
   printf("Current instruction:\t\t\t\t%s\n", cpu_instruction_name(op_code));
   printf("Current state:\t\t\t\t\t%s\n", cpu_state_name(state));

   printf("Program counter:\t\t\t\t%hu\n", pc);
   printf("Stack pointer:\t\t\t\t\t%hu\n", stack_pointer());
   printf("Last added value to the stack:\t\t\t%hu\n\n", stack_last_added_value());

   printf("Instruction register:\t\t\t\t%s ", get_binary((ir >> 16) & 0xFF, 8));
   printf("%s ", get_binary((ir >> 8) & 0xFF, 8));
   printf("%s\n", get_binary(ir & 0xFF, 8));

   printf("Status register (ISNZVC):\t\t\t%s\n\n", get_binary(sr, 6));

   printf("Content in CPU register R16:\t\t\t%s\n", get_binary(reg[R16], 8));
   printf("Content in CPU register R17:\t\t\t%s\n", get_binary(reg[R17], 8));
   printf("Content in CPU register R18:\t\t\t%s\n", get_binary(reg[R18], 8));
   printf("Content in CPU register R24:\t\t\t%s\n\n", get_binary(reg[R24], 8));

   printf("Address in X register:\t\t\t\t%u\n", reg[XL] | (reg[XH] << 8));
   printf("Address in Y register:\t\t\t\t%u\n\n", reg[YL] | (reg[YH] << 8));

   printf("Content in data direction register DDRB:\t%s\n", get_binary(data_memory_read(DDRB), 8));
   printf("Content in data register PORTB:\t\t\t%s\n", get_binary(data_memory_read(PORTB), 8));
   printf("Content in pin input register PINB:\t\t%s\n\n", get_binary(data_memory_read(PINB), 8));

   printf("Content in PCICR:\t\t\t\t%s\n", get_binary(data_memory_read(PCICR + 256), 8));
   printf("Content in PCMSK0:\t\t\t\t%s\n", get_binary(data_memory_read(PCMSK0 + 256), 8));
   printf("Content in PCIFR:\t\t\t\t%s\n", get_binary(data_memory_read(PCIFR + 256), 8));

   printf("--------------------------------------------------------------------------------\n\n");
   return;
}

/********************************************************************************
* monitor_interrupts: Monitors all interrupt sources in the system.
********************************************************************************/
static void monitor_interrupts(void)
{
   monitor_pcint0();
   monitor_pcint1();
   monitor_pcint2();
   return;
}


/********************************************************************************
* check_for_irq: Checks for interrupt requests and generates an interrupt if
*                the I flag in status register i set, a specific interrupt flag,
*                such as PCIF0 in PCIFR, is set and the corresponding interrupt 
*                enable bit, such as PCIE0 in PCICR, i set. Before an interrupt
*                is generated, the corresponding flag bit is cleared to
*                terminate the interrupt request. Otherwise the interrupt
*                will be generated again and again). A jump is made to the
*                corresponding interrupt vector, such as PCINT0_vect.
********************************************************************************/
static void check_for_irq(void)
{
   if (read(sr, I)) 
   {
      const uint8_t pcifr = data_memory_read(PCIFR + 256);
      const uint8_t pcicr = data_memory_read(PCICR + 256);

      if (read(pcifr, PCIF0) && read(pcicr, PCIE0))
      {
         data_memory_clear_bit(PCIFR + 256, PCIF0); 
         generate_interrupt(PCINT0_vect);          
      }
      else if (read(pcifr, PCIF1) && read(pcicr, PCIE1))
      {
         data_memory_clear_bit(PCIFR + 256, PCIF1); 
         generate_interrupt(PCINT1_vect);          
      }
      else if (read(pcifr, PCIF2) && read(pcicr, PCIE2))
      {
         data_memory_clear_bit(PCIFR + 256, PCIF2); 
         generate_interrupt(PCINT2_vect);          
      }
   }
   return;
}

/********************************************************************************
* generate_interrupt: Generates and interrupt by jumping to specified interrupt
*                     vector. Before the jump, the return address is stored
*                     on the stack and the I-flag in the status register is
*                     cleared so that no new interrupts are generated while
*                     the current interrupt is executed.
* 
*                     - interrupt_vector: Jump address for generating interrupt.
********************************************************************************/
static void generate_interrupt(const uint8_t interrupt_vector)
{
   stack_push(pc);        
   clr(sr, I);            
   pc = interrupt_vector;
   return;
}

/********************************************************************************
* monitor_pcint0: Monitors pin change interrupts on I/O port B. All pins where
*                 pin change monitoring is enabled (corresponding mask bit in
*                 PCMSK0 i set) is monitored by comparing current input signal
*                 with the previous one. If they don't match, the corresponding
*                 interrupt flag PCIF0 in the PCIFR register i set to generate
*                 an interrupt request (IRQ).
********************************************************************************/
static inline void monitor_pcint0(void)
{
   const uint8_t pinb_current = data_memory_read(PINB);
   const uint8_t pcmsk0 = data_memory_read(PCMSK0 + 256);

   for (uint8_t i = 0; i < DATA_MEMORY_DATA_WIDTH; ++i)
   {
      if (read(pcmsk0, i))
      {
         if (read(pinb_current, i) != read(pinb_previous, i))
         {
            data_memory_set_bit(PCIFR + 256, PCIF0);
            break;
         }
      }
   }

   pinb_previous = pinb_current;
   return;
}

/********************************************************************************
* monitor_pcint1: Monitors pin change interrupts on I/O port C. All pins where
*                 pin change monitoring is enabled (corresponding mask bit in
*                 PCMSK1 i set) is monitored by comparing current input signal
*                 with the previous one. If they don't match, the corresponding
*                 interrupt flag PCIF1 in the PCIFR register i set to generate
*                 an interrupt request (IRQ).
********************************************************************************/
static inline void monitor_pcint1(void)
{
   const uint8_t pinc_current = data_memory_read(PINC);
   const uint8_t pcmsk1 = data_memory_read(PCMSK1 + 256);

   for (uint8_t i = 0; i < DATA_MEMORY_DATA_WIDTH; ++i)
   {
      if (read(pcmsk1, i))
      {
         if (read(pinc_current, i) != read(pinc_previous, i))
         {
            data_memory_set_bit(PCIFR + 256, PCIF1);
            break;
         }
      }
   }

   pinc_previous = pinc_current;
   return;
}

/********************************************************************************
* monitor_pcint2: Monitors pin change interrupts on I/O port d. All pins where
*                 pin change monitoring is enabled (corresponding mask bit in
*                 PCMSK2 i set) is monitored by comparing current input signal
*                 with the previous one. If they don't match, the corresponding
*                 interrupt flag PCIF2 in the PCIFR register i set to generate
*                 an interrupt request (IRQ).
********************************************************************************/
static inline void monitor_pcint2(void)
{
   const uint8_t pind_current = data_memory_read(PIND);
   const uint8_t pcmsk2 = data_memory_read(PCMSK2 + 256);

   for (uint8_t i = 0; i < DATA_MEMORY_DATA_WIDTH; ++i)
   {
      if (read(pcmsk2, i))
      {
         if (read(pind_current, i) != read(pind_previous, i))
         {
            data_memory_set_bit(PCIFR + 256, PCIF2);
            break;
         }
      }
   }

   pind_previous = pind_current;
   return;
}



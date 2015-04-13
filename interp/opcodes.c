#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <limits.h>

#include "syscall.h"
#include "opcodes.h"
#include "opcodefunc.h"
#include "machine.h"

#define WORD_SIZE 4
#define GET_INTVAL(reg, off) (Get_Word(cpu-> reg + OFFSET(off)))
#define SET_INTVAL(reg, off, val) (Set_Word(cpu-> reg + OFFSET(off), val))
#define INC(reg, amount)    cpu-> reg += amount*WORD_SIZE;
#define OFFSET(v)           ((v)*WORD_SIZE)

int Do_Debug = 0;
int Max_Instructions = INT_MAX;
int Timer_Interval = 0;

void Opcodes_Debug()
{
    Do_Debug = 1;
}

void Set_Timer_Interval(int instr)
{
    Timer_Interval = instr;
}

void Set_Max_Instr(int max)
{
    Max_Instructions = max;
}

// set DEBUG to "//" to turn off DEBUG
#define DEBUG(...) debug_print(cpu,__VA_ARGS__);
static void debug_print(Machine_State *cpu, const char *fmt, ...)
{
    if (!Do_Debug) return;

    va_list args;
    va_start(args, fmt);

    char format[200];
    sprintf(format, "%d %d %d %d %s\n", 
            cpu->BP, cpu->IP, cpu->SP, cpu->FP, fmt);
    vfprintf(stderr, format, args);
    va_end(args);
}

//***************************************
static void push(Machine_State *cpu, int value)
{
    Set_Word(cpu->SP, value);
    cpu->SP += WORD_SIZE;
}
//***************************************
static int pop(Machine_State *cpu)
{
    cpu->SP -= WORD_SIZE;
    return Get_Word(cpu->SP);
}
//***************************************
static void do_rti(Machine_State *cpu)
{
    int temp;

    if ( !(cpu->FLAG & FL_INT_MODE) )
    {
        Machine_Check("RTI while not in interrupt mode at %d", cpu->IP);
    }

    temp = pop(cpu);
    cpu->SSP = cpu->SP;
    cpu->SP = temp;

    cpu->FLAG = pop(cpu);
    cpu->FP = pop(cpu);
    cpu->IP = pop(cpu);
    cpu->LP = pop(cpu);
    cpu->BP = pop(cpu);
}
//***********************************************
static void interrupt(Machine_State *cpu, int vector)
{
    int temp;

    // turn off pending bit
    cpu->FLAG &= ~FL_INT_PENDING;

    push(cpu, cpu->BP);
    push(cpu, cpu->LP);
    push(cpu, cpu->IP);
    push(cpu, cpu->FP);
    push(cpu, cpu->FLAG);

    // go to system mode and interrupt mode
    cpu->FLAG &= ~FL_USER_MODE;
    cpu->FLAG |= FL_INT_MODE;

    temp = cpu->SP;
//    cpu->SP = cpu->SSP;
    // Switch FP and SP to absolute addresses
    cpu->FP += cpu->BP;
    cpu->SP += cpu->BP;

    // store SP on system stack
    push(cpu, temp);

    // ISR is at vector
    cpu->IP = Get_Word(vector*WORD_SIZE);
}
//***************************************
void Execute(Machine_State *cpu)
{
    int temp;
    static int num_instructions = 0;

    if (++num_instructions >= Max_Instructions) 
    {
        cpu->FLAG |= FL_HALTED;
        return;
    }

    if (Timer_Interval > 0 && (num_instructions % Timer_Interval) == 0)
    {
        cpu->FLAG |= FL_INT_PENDING;
    }

    if ((cpu->FLAG & FL_INT_PENDING) && !(cpu->FLAG & FL_INT_MODE))
    {
        interrupt(cpu, INTERRUPT_VECTOR);
        return;
    }

    int val = GET_INTVAL(IP, 0);
    
    switch (val)
    {
        case NOP:
            DEBUG("NOP");
            INC(IP, 1);
            break;
        case PLUS_OP:
            DEBUG("PLUS");
            SET_INTVAL(SP, -2, (GET_INTVAL(SP, -2) + GET_INTVAL(SP, -1)));
            INC(SP, -1);
            INC(IP, 1);
            break;
        case MINUS_OP:
            DEBUG("MINUS");
            SET_INTVAL(SP, -2, (GET_INTVAL(SP, -2) - GET_INTVAL(SP, -1)));
            INC(SP, -1);
            INC(IP, 1);
            break;
        case TIMES_OP:
            DEBUG("TIMES");
            SET_INTVAL(SP, -2, (GET_INTVAL(SP, -2) * GET_INTVAL(SP, -1)));
            INC(SP, -1);
            INC(IP, 1);
            break;
        case DIVIDE_OP:
            DEBUG("DIVIDE");
            SET_INTVAL(SP, -2, (GET_INTVAL(SP, -2) / GET_INTVAL(SP, -1)));
            INC(SP, -1);
            INC(IP, 1);
            break;
        case MOD_OP:
            DEBUG("MOD");
            SET_INTVAL(SP, -2, (GET_INTVAL(SP, -2) % GET_INTVAL(SP, -1)));
            INC(SP, -1);
            INC(IP, 1);
            break;
        case NEG_OP:
            DEBUG("NEG");
            SET_INTVAL(SP, -1, -GET_INTVAL(SP, -1));
            INC(IP, 1);
            break;
        case EQ_OP:
            DEBUG("EQ");
            SET_INTVAL(SP, -2, (GET_INTVAL(SP, -2) == GET_INTVAL(SP, -1)));
            INC(SP, -1);
            INC(IP, 1);
            break;
        case NE_OP:
            DEBUG("NE");
            SET_INTVAL(SP, -2, (GET_INTVAL(SP, -2) != GET_INTVAL(SP, -1)));
            INC(SP, -1);
            INC(IP, 1);
            break;
        case GT_OP:
            DEBUG("GT");
            SET_INTVAL(SP, -2, (GET_INTVAL(SP, -2) > GET_INTVAL(SP, -1)));
            INC(SP, -1);
            INC(IP, 1);
            break;
        case LT_OP:
            DEBUG("LT");
            SET_INTVAL(SP, -2, (GET_INTVAL(SP, -2) < GET_INTVAL(SP, -1)));
            INC(SP, -1);
            INC(IP, 1);
            break;
        case GE_OP:
            DEBUG("GE");
            SET_INTVAL(SP, -2, (GET_INTVAL(SP, -2) >= GET_INTVAL(SP, -1)));
            INC(SP, -1);
            INC(IP, 1);
            break;
        case LE_OP:
            DEBUG("LE");
            SET_INTVAL(SP, -2, (GET_INTVAL(SP, -2) <= GET_INTVAL(SP, -1)));
            INC(SP, -1);
            INC(IP, 1);
            break;
        case AND_OP:
            DEBUG("AND");
            SET_INTVAL(SP, -2, (GET_INTVAL(SP, -2) && GET_INTVAL(SP, -1)));
            INC(SP, -1);
            INC(IP, 1);
            break;
        case OR_OP:
            DEBUG("OR");
            SET_INTVAL(SP, -2, (GET_INTVAL(SP, -2) || GET_INTVAL(SP, -1)));
            INC(SP, -1);
            INC(IP, 1);
            break;
        case HALT_OP:
            DEBUG("HALT");
            cpu->FLAG |= FL_HALTED;
            break;
        case POP_OP:
            DEBUG("POP");
            INC(SP, -1);
            INC(IP, 1);
            break;
        case CALL_OP:
            DEBUG("CALL %d", GET_INTVAL(IP, 1));
            temp = GET_INTVAL(IP, 1);
            SET_INTVAL(SP, 0, (cpu->IP + OFFSET(2)));
            SET_INTVAL(SP, 1, (cpu->FP));
            INC(SP, 2);
            cpu->FP = cpu->SP;
            cpu->IP = temp;
            break;
        case CALLI_OP:
            DEBUG("CALLI %d", GET_INTVAL(SP, -1));
            INC(SP, -1);
            temp = GET_INTVAL(SP, 0);
            SET_INTVAL(SP, 0, (cpu->IP + OFFSET(2)));
            SET_INTVAL(SP, 1, (cpu->FP));
            INC(SP, 2);
            cpu->FP = cpu->SP;
            cpu->IP = temp;
            break;
        case RETURN_OP:
            DEBUG("RETURN");
            cpu->SP = cpu->FP - OFFSET(2);
            cpu->IP = GET_INTVAL(FP, -2);
            cpu->FP = GET_INTVAL(FP, -1);
            break;
        case RETURNV_OP:
            DEBUG("RETURNV");
            temp = GET_INTVAL(SP, -1);
            cpu->SP = cpu->FP - OFFSET(1);
            cpu->IP = GET_INTVAL(FP, -2);
            cpu->FP = GET_INTVAL(FP, -1);
            SET_INTVAL(SP, -1, temp);
            break;
        case PUSH_OP:
            DEBUG("PUSH %d", GET_INTVAL(IP, 1));
            INC(IP, 1);
            SET_INTVAL(SP, 0, GET_INTVAL(IP, 0));
            INC(SP, 1);
            INC(IP, 1);
            break;
        case PUSHFP_OP:
            DEBUG("PUSHFP %d", cpu->FP);
            SET_INTVAL(SP, 0, cpu->FP);
            INC(SP, 1);
            INC(IP, 1);
            break;
        case OUTS_OP:
            DEBUG("OUTS %d", GET_INTVAL(SP,-1));
            if (cpu->FLAG & FL_USER_MODE)
            {
                Machine_Check("OUTS instruction while not in system mode");
            }
            temp = pop(cpu);
            printf("%s", (char *)Get_Addr(temp));
            INC(IP, 1);
            break;
        case TRAPTOC_OP:
            DEBUG("TRAPTOC_OP %d %d", GET_INTVAL(FP,-3), GET_INTVAL(FP,-4));
            {
                int args[20];
                int ii;
                args[0] = GET_INTVAL(FP, -3);
                for (ii=1; ii<args[0]; ii++)
                {
                    args[ii] = GET_INTVAL(FP, -ii-3);
                }
                //INC(SP, -args[0]);
                INC(IP, 1);
                temp = syscall(args);
                SET_INTVAL(SP, 0, temp);
                INC(SP, 1);
            }
            break;
        case TRAP_OP:
            DEBUG("TRAP %d %d", GET_INTVAL(FP,-3), GET_INTVAL(FP,-4));
            INC(IP, 1);
            interrupt(cpu, TRAP_VECTOR);
            break;
        case RTI_OP:
            DEBUG("RTI");
            do_rti(cpu);
            break;
        case JUMP_OP:
            DEBUG("JUMP %d", GET_INTVAL(IP, 1));
            cpu->IP = GET_INTVAL(IP, 1);
            break;
        case JUMPE_OP:
            DEBUG("JUMPE %d %d", GET_INTVAL(SP, -1), GET_INTVAL(IP, 1));
            INC(IP, 1);
            INC(SP, -1);
            if (!GET_INTVAL(SP, 0))
                cpu->IP = GET_INTVAL(IP,0);
            else
                INC(IP, 1);
            break;
        case PUSHCVARIND_OP:
            temp = GET_INTVAL(SP, -1);
            DEBUG("PUSHCVARIND %d %d", GET_INTVAL(SP, -1), Get_Byte(temp));
            SET_INTVAL(SP,-1, Get_Byte(temp));
            INC(IP, 1);
            break;
        case POPCVARIND_OP:
            DEBUG("POPCVARIND %d %d", GET_INTVAL(SP, -2), GET_INTVAL(SP, -1));
            temp = GET_INTVAL(SP, -1);
            Set_Byte(temp, GET_INTVAL(SP, -2));
            INC(SP, -2);
            INC(IP, 1);
            break;
        case PUSHVAR_OP:
            temp = cpu->FP + GET_INTVAL(IP, 1);
            DEBUG("PUSHVAR %d %d", GET_INTVAL(IP, 1), Get_Word(temp));
            INC(IP, 1);
            SET_INTVAL(SP,0, Get_Word(temp));
            INC(SP, 1);
            INC(IP, 1);
            break;
        case POPVAR_OP:
            DEBUG("POPVAR %d", GET_INTVAL(IP,1));
            INC(IP, 1);
            temp = cpu->FP + GET_INTVAL(IP, 0);
            INC(SP, -1);
            Set_Word(temp, GET_INTVAL(SP,0));
            INC(IP,1);
            break;
        case PUSHCVAR_OP:
            temp = cpu->FP + GET_INTVAL(IP, 1);
            DEBUG("PUSHCVAR %d %d", GET_INTVAL(IP, 1), Get_Byte(temp));
            Set_Byte(cpu->SP, Get_Byte(temp));
            INC(SP, 1);
            INC(IP, 2);
            break;
        case POPCVAR_OP:
            DEBUG("POPVVAR %d", GET_INTVAL(IP,1));
            temp = cpu->FP + GET_INTVAL(IP, 1);
            INC(SP, -1);
            Set_Byte(temp, Get_Byte(cpu->SP));
            INC(IP,2);
            break;
        case ADJSP_OP:
            DEBUG("ADJSP %d", GET_INTVAL(IP,1));
            INC(IP,1);
            cpu->SP += GET_INTVAL(IP,0);
            INC(IP,1);
            break;
        case SWAP_OP:
            DEBUG("SWAP %d %d", GET_INTVAL(SP, -1), GET_INTVAL(SP, -2));
            temp = GET_INTVAL(SP, -1);
            SET_INTVAL( SP, -1, GET_INTVAL(SP, -2));
            SET_INTVAL(SP, -2, temp);
            INC(IP,1);
            break;
        case DUP_OP:
            DEBUG("DUP");
            SET_INTVAL(SP, 0, GET_INTVAL(SP, -1));
            INC(SP, 1);
            INC(IP,1);
            break;
        default:
            Machine_Check("Illegal instruction '%i' at %d\n", val, cpu->IP);
            break;
    }
}

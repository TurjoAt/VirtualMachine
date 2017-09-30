/*
 * File:    vm.c
 * Virtual Machine
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_STACK_HEIGHT 2000
#define CODE_LENGTH 500
#define MAX_LEXI_LEVELS 3

// Required structures
typedef struct {
		int op; // Opcode
     	int r; 	// Reg
    	int l;  // L
     	int m;  // M
 } instruction;

// Declare opcodes
const char *opcodes[] = {"", "lit", "rtn", "lod", "sto", "cal", "inc", "jmp", "jpc", "sio",
						"neg", "add", "sub", "mul", "div", "odd", "mod", "eql", "neq", "lss",
						"leq", "gtr", "geq"};

// Declare functions
void vm();
void executionCycle(instruction* theInstruction, int* regFile, int* stack, int* sp, int* bp, int* pc, int*halt);
void printActivationRecords(int *stack, int SP, int BP, FILE *out);
int base(int* bp, instruction* theInstruction, int* stack);

void vm()
{
	// Read & Write
    FILE *in = fopen("input.txt", "r");
    FILE *out = fopen("output.txt", "w");

    int i = 0;

	// Set the stack & registers to 0
    int stack[MAX_STACK_HEIGHT] = {0};
	stack[2] = 1; 
    int regFile[8] = {0};

    //Assigning values from homework specifications
    int SP = 0, BP = 1, PC = 0, IR = 0;

    //structArr - array of instruction structs, contains op, r, l, and m.
    instruction structArr[CODE_LENGTH] = {0};
	//theInstruction is goinf to be used as a single reference to one of the instruction structArr
    instruction *theInstruction;

    fprintf(out, "Factorial Op Printout:\n");

	//Fetch Cycle(a) - storing the instructions int the instruction structArr
	//Scan the information from input file and print out to the output file
    while (fscanf(in, "%d %d %d %d", &structArr[i].op, &structArr[i].r, &structArr[i].l, &structArr[i].m) != EOF)
    {
        fprintf(out, "%d\t%s\t%d\t%d\t%d\n", i, opcodes[structArr[i].op], structArr[i].r, structArr[i].l, structArr[i].m);
        i++;
    }

    fprintf(out, "\n\n");
	fprintf(out, "Factorial Stack Trace:\n");
    fprintf(out, "Initial Values\t\t\t\tpc \tbp \tsp\n");

		//halt = 0, end of the program
    int halt = 0;
    while (halt != 1)
    {
        //Fetch Cycle(b)- loading an instruction from the instruction structArr at position PC onto theInstruction
		//and printing its values
        theInstruction = &structArr[PC];
        fprintf(out, "%d\t%s\t%d\t%d\t%d",PC, opcodes[theInstruction->op], theInstruction->r, theInstruction->l, theInstruction->m);
        ++PC;

        //Execution Cycle - the current instructions as well as the contents of the activtion record get passed
		//into the executionCycle() function with implements a long switch case.
        executionCycle(theInstruction, regFile, stack, &SP, &BP, &PC, &halt);
        fprintf(out, "\t%d\t%d\t%d\t", PC, BP, SP);

        //Calls function that prints out the Activtion Record and separates each one with a bar "|"
        printActivationRecords(stack, SP, BP, out);
		fprintf(out, "\n");
		
		//Prints the contents of all registers
		fprintf(out, "RF: " );
		for(i = 0; i < 8; i++)
		{
			fprintf(out, "%d", regFile[i]);
			(i+1<8) ? fprintf(out, ", ") : fprintf(out, "\n");
		}

	}

	
	fprintf(out, "\nFactorial Output:\n");
	fprintf(out, "%d\n", regFile[theInstruction->r]);

    fclose(in);
    fclose(out);

}

//Execute Cycle Function: mimics execute, and contains code for each instruction in ISA, implemented in a switch.
void executionCycle(instruction* theInstruction, int* regFile, int* stack, int* sp, int* bp, int* pc, int*halt) {

    switch(theInstruction->op)
    {
        case 1: //LIT
            regFile[theInstruction -> r] = theInstruction -> m;
            break;
        case 2: //RTN
            *sp = *bp - 1;
            *bp = stack[*sp + 3];
            *pc = stack[*sp + 4];
            break;
        case 3: //LOD
            regFile[theInstruction -> r] = stack[ base(bp, theInstruction, stack) + theInstruction -> m];
            break;
        case 4: //STO
            stack[base(bp, theInstruction, stack) + theInstruction -> m] = regFile[theInstruction -> r];
            break;
        case 5: //CAL
            stack[*sp + 1] = 0; //space to return value
            stack[*sp + 2] = base(bp, theInstruction, stack); //static link (SL)
            stack[*sp + 3] = *bp; //dynamic link (DL)
            stack[*sp + 4] = *pc; //return address (RA)
            *bp = *sp + 1;
            *pc = theInstruction -> m;
            break;
        case 6: //INC
            *sp += theInstruction->m;
            break;
        case 7: //JMP
            *pc = theInstruction -> m;
            break;
        case 8: //JPC
            if(regFile[theInstruction -> r] == 0)
             *pc = theInstruction -> m;
            break;
        case 9: //SIO
            if(theInstruction ->m == 1)
                printf("%d\n", regFile[theInstruction -> r]);
            else if(theInstruction -> m == 2)
                scanf("%d", &regFile[theInstruction -> r]);
            else if(theInstruction -> m == 3)
            {
                *halt = 1;
                *sp = 0;
                *pc = 0;
            }
            break;
        case 10: //NEG
            regFile[theInstruction -> r] = -(regFile[theInstruction -> l]);
            break;
        case 11: //ADD
            regFile[theInstruction -> r] = regFile[theInstruction -> l] + regFile[theInstruction -> m];
            break;
        case 12: //SUB
            regFile[theInstruction -> r] = regFile[theInstruction -> l] - regFile[theInstruction -> m];
            break;
        case 13: //MUL
            regFile[theInstruction -> r] = regFile[theInstruction -> l] * regFile[theInstruction -> m];
            break;
        case 14: //MUL
            regFile[theInstruction -> r] = regFile[theInstruction -> l] / regFile[theInstruction -> m];
            break;
        case 15: //ODD
            regFile[theInstruction -> r] = regFile[theInstruction -> r] % 2;
            break;
        case 16: // MOD
            regFile[theInstruction -> r] = regFile[theInstruction -> l] % regFile[theInstruction -> m];
        case 17: // EQL
            if ( regFile[theInstruction -> l] == regFile[theInstruction -> m] )
                regFile[theInstruction -> r] = 1;
            else
                regFile[theInstruction -> r] = 0;
            break;
        case 18: // NEQ
            if ( regFile[theInstruction -> l] != regFile[theInstruction -> m] )
                regFile[theInstruction -> r] = 1;
            else
                regFile[theInstruction -> r] = 0;
            break;
        case 19: // LSS
            if( regFile[theInstruction -> l] < regFile[theInstruction -> m] )
                regFile[theInstruction -> r] = 1;
            else
                regFile[theInstruction -> r] = 0;
            break;
        case 20: // LEQ
            if( regFile[theInstruction -> l] <= regFile[theInstruction -> m] )
                regFile[theInstruction -> r] = 1;
            else
                regFile[theInstruction -> r] = 0;
            break;
        case 21: // GTR
            if ( regFile[theInstruction -> l] > regFile[theInstruction -> m] )
                regFile[theInstruction -> r] = 1;
            else
                regFile[theInstruction -> r] = 0;
            break;
        case 22: // GEQ
            if( regFile[theInstruction -> l] > regFile[theInstruction -> m] )
                regFile[theInstruction -> r] = 1;
            else
                regFile[theInstruction -> r] = 0;
            break;

        default:
            printf("Illegal OPR!\n");
    }

}

//Recursive function that prints each stack frame using | in between
void printActivationRecords(int *stack, int SP, int BP, FILE* out) {

    int i = 0;

    //Base Case #1: if BP is 0, the program has finished. No stack frames are left to print out
    if (BP == 0)
        return;
    //Base Case #2: if BP is 1, then it is in the main stack frame, and we print out the stack from
	//BP to SP, with BP pointing to the bottom of the main stack frame, and SP pointing to the top of the stack
    else if (BP == 1)
    {
        for(i = 1; i <= SP; i++)
			fprintf(out, "%d ",stack[i]);
		if(SP == 0)
           fprintf(out, "0");
        return;
    }
    //Recursive Case: Prints out each new stack frame, separating them with |
    else
    {
        printActivationRecords(stack, BP-1, stack[BP+2], out);

        //Covers one case, where CAL instruction is just called, meaning a new Stack Frame is created, but SP is still less than BP
        if (SP + 1< BP)
        {
            fprintf(out, "| ");

            for (i = 0; i < 4; i++)
                fprintf(out, "%d ", stack[BP+i]);
        }	
        //For SP being greater than BP, aka most cases
        else
        {   
			if (BP < SP)
				fprintf(out, "| ");
			for (i = BP; i <= SP; i++){
			
                fprintf(out, "%d ", stack[i]);
				
			}
		}
        return;
    }
}


//Base function: function used to find base L levels down
//l stans for L in the instruction format, passed through the theInstruction
int base(int* bp, instruction *theInstruction, int *stack)
{
    int l = theInstruction -> l;
    int b1; //find base L levels down
    b1 = *bp;
    while (l > 0)
		{
        b1 = stack[b1 + 1];
        l--;
    }

    return b1;
}


int main(int argc, char** argv)
{
    vm();
    return 0;
}
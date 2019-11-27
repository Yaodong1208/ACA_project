/*
 * Copyright 2002-2019 Intel Corporation.
 * 
 * This software is provided to you as Sample Source Code as defined in the accompanying
 * End User License Agreement for the Intel(R) Software Development Products ("Agreement")
 * section 1.L.
 * 
 * This software and the related documents are provided as is, with no express or implied
 * warranties, other than those that are expressly stated in the License.
 */

#include <stdio.h>
#include "pin.H"
#include <set>
#include <string>
#include "../../../extras/xed-intel64/include/xed-iclass-enum.h" //for xed_iclass_enum_t
using namespace std;
FILE * trace;
//contains all instruction types we met in this app
//set<OPCODE> ins_types; 
set<LEVEL_BASE::OPCODE> ins_types; 

// Pin calls this function every time a new instruction is encountered
VOID Instruction(INS ins, VOID *v)
{
    //insert instruction type to ins_type
    ins_types.insert(LEVEL_CORE::INS_Opcode(ins));
}

// This function is called when the application exits
VOID Fini(INT32 code, VOID *v)
{
    //out put all kinds of instruction type to trace
    for( set<LEVEL_BASE::OPCODE>::iterator it = ins_types.begin(); it != ins_types.end(); it++)
        fprintf(trace,"%d\n",(*it));
    fprintf(trace, "#eof\n");
    fclose(trace);
}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    PIN_ERROR("This Pintool prints the IPs of every instruction executed\n" 
              + KNOB_BASE::StringKnobSummary() + "\n");
    return -1;
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, char * argv[])
{
    trace = fopen("ins_type.out", "w");
    
    // Initialize pin
    if (PIN_Init(argc, argv)) return Usage();

    // Register Instruction to be called to instrument instructions
    INS_AddInstrumentFunction(Instruction, 0);

    // Register Fini to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);
    
    // Start the program, never returns
    PIN_StartProgram();
    
    return 0;
}

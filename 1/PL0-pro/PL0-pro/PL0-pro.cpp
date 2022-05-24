// PL0-pro.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "PL0.h"
#include "grammar.h"

int main(int argc, char** argv)
{
    uint32_t stage = 0;
    
    PL0 compiler("const CONSTSYM var VARSYM procedure PROCEDURESYM begin BEGINSYM odd ODD "
        "end ENDSYM if IFSYM then THENSYM call CALLSYM while WHILESYM do DOSYM read READSYM write WRITESYM",
        "= EQUAL := ASSIGN + ADD - SUB * MUL / DIV # NOTEQL < LT <= LTOREQ > GT >= GTOREQ",
        ". END , COM ; SEMI ( LPARE ) RPARE", "NUMBER 0 1 2 3 4 5 6 7 8 9", "IDENT", Grammar_0); 

    /*PL0 compiler("a abcA b abcB",
        "* MUL + ADD @ AND",
        "( LPARE ) RPARE", "NUMBER 0 1 2 3 4 5 6 7 8 9", "IDENT", Grammar_0);*/

    /*PL0 compiler("i ID",
        "* MUL + ADD",
        "( LPARE ) RPARE", "NUMBER 0 1 2 3 4 5 6 7 8 9", "IDENT", Grammar_0);*/

    if (argc <= 1) {
        compiler.run(stage);
    } else {
        stage = atoi(argv[1]);
        compiler.run(stage);
    }

    return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file

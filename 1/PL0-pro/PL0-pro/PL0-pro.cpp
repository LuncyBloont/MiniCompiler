// PL0-pro.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <string>
#include <fstream>
#include "PL0.h"

int main(int argc, char** argv)
{
    uint32_t stage = 0;

    std::ifstream fkeys("pl0\\keys.txt");
    std::string keys((std::istreambuf_iterator<char>(fkeys)), std::istreambuf_iterator<char>());
    std::ifstream fops("pl0\\ops.txt");
    std::string ops((std::istreambuf_iterator<char>(fops)), std::istreambuf_iterator<char>());
    std::ifstream fsign("pl0\\sign.txt");
    std::string sign((std::istreambuf_iterator<char>(fsign)), std::istreambuf_iterator<char>());
    std::ifstream fnum("pl0\\numbers.txt");
    std::string numbers((std::istreambuf_iterator<char>(fnum)), std::istreambuf_iterator<char>());
    std::ifstream fidname("pl0\\idname.txt");
    std::string idname((std::istreambuf_iterator<char>(fidname)), std::istreambuf_iterator<char>());
    std::ifstream fgrammar("pl0\\Grammar.txt");
    std::string grammer((std::istreambuf_iterator<char>(fgrammar)), std::istreambuf_iterator<char>());
    PL0 compiler(keys, ops, sign, numbers, idname, grammer);

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

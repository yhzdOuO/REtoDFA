#include <iostream>
#include <fstream>
#include "nfa.h"
#include "dfa.h"
using namespace std;

int main()
{
    DFA d;

    ifstream inputFile("input.txt");
    ofstream outputFile("output.txt");

    if (!inputFile) {
        cerr << "无法打开输入文件!" << endl;
        return 1;
    }

    if (!outputFile) {
        cerr << "无法打开输出文件!" << endl;
        return 1;
    }

    string line;
    if (getline(inputFile, line)) {
        /* 生成DFA */
        d.REtoDFA(line);
        d.printStr();
        d.printNFA();
        d.printDFA();

        /* 最小化DFA */
        d.Minimize();
        d.printDFA();
    }
    while (getline(inputFile, line)) {
        if (!line.empty()) {
            string ans = d.eval(line) ? "true" : "false";
            outputFile << ans << endl;
            cout << ans << endl;
        }
    }

    inputFile.close();
    outputFile.close();

    return 0;
}
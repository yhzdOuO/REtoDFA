#pragma once
#include <iostream>
#include <algorithm>
#include <vector>
#include <map>
#include <string>
#include <stack>
#include <set>
using namespace std;

#define charCat '+'
#define charEpsilon ' '

struct Node {
    Node(int id, int nextId) {
        this->id = id;
        this->nextId = nextId;
    }
    int id, nextId;
};

struct Edge {
    Edge(int id, int nextId, char input = charEpsilon) {
        this->id = id;
        this->input = input;
        this->nextId = nextId;
    }
    int id, nextId;
    char input;
};

class NFA {
public:
    NFA();
    void REtoNFA(string& input);
    void printNFA();
    void printStr();

protected:
    int idNumNFA, idBeginNFA, idEndNFA;
    vector<char> vocabulary;
    vector<vector<Edge*>> nfa;

private:
    /* Thompson构造法，从regular expression 生成对应的 Epsilon-NFA */
    void ThompsonsConstruction();
    /* 处理输入的字符串 */
    void processStr(string& input);
    void clear();

    Node* convertUnion(vector<Edge*>& edgeList, Node* node1, Node* node2);
    Node* convertConcate(vector<Edge*>& edgeList, Node* node1, Node* node2);
    Node* convertStar(vector<Edge*>& edgeList, Node* node);
    Node* convertCharacter(vector<Edge*>& edgeList, char c);

    string str;
    string origin;
};
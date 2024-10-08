#pragma once
#include "nfa.h"

class DFA: public NFA{
public:
	DFA();
	void REtoDFA(string& input);
	void Minimize();
	void printDFA();
	bool eval(const string& input);

protected:
	vector<vector<Edge*>> dfa;
	int idNumDFA;
	int idBeginDFA;
	set<int> idEndDFA;

private:
	/* 在DFA中从当前节点nowId开始向input走一步，返回目标节点id */
	int go(int nowId, char input);

	void move(set<int>& t, char a, set<int>& m);
	void closure(set<int>& t);

	/* 子集构造法，将NFA转为DFA */
	void powersetConstruction();
	/* hopcroft构造法，最小化DFA*/
	void hopcroft();

	void clear();

	bool hasDFA, minimized;
};
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
	/* ��DFA�дӵ�ǰ�ڵ�nowId��ʼ��input��һ��������Ŀ��ڵ�id */
	int go(int nowId, char input);

	void move(set<int>& t, char a, set<int>& m);
	void closure(set<int>& t);

	/* �Ӽ����취����NFAתΪDFA */
	void powersetConstruction();
	/* hopcroft���취����С��DFA*/
	void hopcroft();

	void clear();

	bool hasDFA, minimized;
};
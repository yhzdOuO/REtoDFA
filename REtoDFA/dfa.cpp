#include "dfa.h"
#include <queue>

DFA::DFA() {
	NFA();
	hasDFA = false; 
	minimized = false;
	idNumDFA = 0;
	idBeginDFA = -1;
}

void DFA::REtoDFA(string& input) {
	REtoNFA(input);
	clear();
	powersetConstruction();
}

void DFA::Minimize() {
	if (hasDFA && !minimized) {
		hopcroft();
	}
}

void DFA::printDFA() {
	if (!hasDFA) {
		return;
	}

	string d = "+++++++++++++++++++++++++++++++";
	cout << d << endl;
	cout << "++++          DFA          ++++" << endl;
	cout << d << endl;

	cout << "vocabulary: [ ";
	for (int vi = 0; vi < vocabulary.size(); vi++) {
		cout << vocabulary[vi] << " ";
	}
	cout << "]" << endl;
	cout << d << endl;

	cout << "  num id: [ " << idNumDFA << " ]" << endl;
	cout << "start id: [ " << idBeginDFA << " ]" << endl;
	cout << "  end id: [ ";
	for (set<int>::iterator it = idEndDFA.begin(); it != idEndDFA.end(); it++) {
		cout << (*it) << " ";
	}
	cout << "]" << endl;
	cout << d << endl;

	cout << "DFA:" << endl;
	for (vector<Edge*> vertex : dfa)
	{
		for (Edge* edge : vertex) {
			cout << "  " << edge->id << "--[" << edge->input << "]-->" << edge->nextId << endl;
		}
	}
	cout << d << endl << endl;
}

bool DFA::eval(const string& input) {
	if (!hasDFA) {
		return false;
	}

	int status = idBeginDFA;
	for (char c : input) {
		status = go(status, c);
		if (status == -1) {
			return false;
		}
	}
	return idEndDFA.find(status) != idEndDFA.end();
}

int DFA::go(int nowId, char input) {
	/* ��ǰΪnowId�Ľڵ㣬��input����һ���������ߵ���Ŀ��ڵ�Id*/
	int i = 0;
	while (i < dfa[nowId].size() && dfa[nowId][i]->input != input) {
		i++;
	}
	if (i == dfa[nowId].size()) {
		return -1;
	}
	else {
		return dfa[nowId][i]->nextId;
	}
}

void DFA::move(set<int>& t, char a, set<int>& m) {
	/* ����t�е����нڵ㣬��NFA����a����һ���������п��Ե����Ŀ��ڵ���뼯��m */
	for (set<int>::iterator it = t.begin(); it != t.end(); it++) {
		for (Edge* edge : nfa[*it]) {
			if (edge->input == a) {
				m.insert(edge->nextId);
			}
		}
	}
}

void DFA::closure(set<int>& t) {
	/* ����t�е����нڵ㣬��NFA�а���bfs�ķ�ʽ����epsilon��һֱ�ߣ������п��Ե����Ŀ��ڵ�Ż�ԭ���� */
	vector<bool> visited(idNumNFA, false);
	queue<int> q;
	for (set<int>::iterator it = t.begin(); it != t.end(); it++) {
		q.push(*it);
	}
	while (!q.empty()) {
		int id = q.front();
		q.pop();
		for (Edge* edge : nfa[id]) {
			if (edge->input == charEpsilon) {
				int nextId = edge->nextId;
				if (visited[nextId]) {
					continue;
				}
				t.insert(nextId);
				q.push(nextId);
				visited[nextId] = true;
			}
		}
	}
}

void DFA::powersetConstruction() {
	vector<Edge*> dfaEdgeList;   /* ��¼DFA�ıߣ����б� */
	vector<set<int>> statusList; /* ��¼���й�������Ӽ���Ҳ����move��epsilon closure��ļ��ϣ���ΪDFA�Ľڵ� */
	vector<bool> visitedList;    /* ��¼��������Լ��Ƿ񱻷��ʹ������Ƿ񾭹�move��closure���� */
	/* ��ʼ���ϵ�epsilon�հ� */
	set<int> init = { idBeginNFA };
	closure(init);
	statusList.push_back(init);
	visitedList.push_back(false);
	idBeginDFA = 0;
	while (true) {
		/* �ҵ���һ��δ�����ʹ����Ӽ� */
		int id = 0;
		while (id < visitedList.size() && visitedList[id]) {
			id++;
		}
		if (id == visitedList.size()) {
			break;
		}
		set<int> nowset = statusList[id];
		/* �Ըü��Ϻ���ĸ����������ĸ��move��epsilon�հ� */
		for (char a : vocabulary) {
			set<int> nextset;
			move(nowset, a, nextset);
			closure(nextset);
			if (nextset.empty()) {
				continue;
			}
			/* �������õ��ļ��ϲ���ԭ�����Ӽ��б��� */
			/* ����ԭ�����Ӽ��б�û���ظ�����루������ʵҲ������setʵ�֣� */
			int nextId = 0;
			while (nextId < statusList.size() && (statusList[nextId] != nextset)) {
				nextId++;
			}
			if (nextId == statusList.size()) {
				statusList.push_back(nextset);
				visitedList.push_back(false);
				/* �ü���������nfa�Ľ���״̬������ΪDFA�Ľ���״̬*/
				if (nextset.find(idEndNFA) != nextset.end()) {
					idEndDFA.insert(nextId);
				}
			}
			/* �����Ƿ���빹������µ�DFA�ڵ㣨�Ӽ�����������ת�ƹ�ϵ��¼��DFA�ı��б��� */
			dfaEdgeList.push_back(new Edge(id, nextId, a));
		}
		visitedList[id] = true;
	}

	/* �����б���ݽڵ�˳��תΪ�ڽӱ�洢 */
	idNumDFA = statusList.size();
	dfa = vector<vector<Edge*>>(idNumDFA);
	for (Edge* edge : dfaEdgeList) {
		dfa[edge->id].push_back(edge);
	}
	hasDFA = true;
}

void DFA::hopcroft() {
	vector<vector<int>> idSetList(2);	/* ��¼��ǰ�Ļ��ֵ�DFA״̬���ϣ������б� */
	vector<int> idToSetId(idNumDFA, 0);	/* ��¼ÿ��DFA״̬Id��Ӧ�ļ���Id*/
	/* ��ʼ������DFA�еĽڵ㻮��Ϊ ��ֹ״̬�� �� ����ֹ״̬�� */
	for (int id = 0; id < idNumDFA; id++) {
		if (idEndDFA.find(id) == idEndDFA.end()) {
			idSetList[0].push_back(id);
			idToSetId[id] = 0;
		}
		else {
			idSetList[1].push_back(id);
			idToSetId[id] = 1;
		}
	}
	/* �������Ͻ��л��֣�ֱ�����ܼ������֣��������б��Ȳ��ٱ仯 */
	int lastLen = 0;
	while (idSetList.size() != lastLen) {
		lastLen = idSetList.size();
		for (int i = 0; i < idSetList.size(); i++) {
			if (idSetList[i].size() == 1) {
				continue;
			}
			for (int vi = 0; vi < vocabulary.size(); vi++) {
				/* �Ե�ǰ����idSetList[i]����һ�λ��� */
				/* ��鼯��������״̬��nextIdSet����c��һ����Ŀ��״̬���ڼ��ϵ�id�� */
				char c = vocabulary[vi];
				int nowSetId = idSetList.size();
				vector<vector<int>> divideIdSet; /* ��¼������nextIdSet��ͬ״̬���ɵļ��� */
				vector<int> divideNextIdSet;     /* ��¼���϶�Ӧ��nextIdSet */
				
				for (int j = 0; j < idSetList[i].size(); j++) {
					/* ���nextIdSet */
					int nowId = idSetList[i][j];
					int nextId = go(nowId, c); /* ����Ϊ-1�������ڸñߣ����Ϊ�ռ� */
					int nextIdSet = nextId;
					if (nextId >= 0) {
						nextIdSet = idToSetId[nextId];
					}
					/* �ж�nextIdSet�Ƿ���ͬ��������ͬ����л��� */
					int k = 0;
					while (k < divideNextIdSet.size() && divideNextIdSet[k] != nextIdSet) {
						k++;
					}
					if (k == divideNextIdSet.size()) {
						divideNextIdSet.push_back(nextIdSet);
						divideIdSet.push_back(vector<int>());
					}
					/* ����Ԫ�ؼ��뵽��ӦnextIdSet�ļ����� */
					divideIdSet[k].push_back(nowId);
				}

				/* ��һ�λ��ֺ�Ľ���ϲ���ԭ���ļ����б� */
				idSetList[i] = divideIdSet[0];
				for (int j = 1; j < divideIdSet.size(); j++) {
					int divideNewSetId = idSetList.size();
					idSetList.push_back(divideIdSet[j]);
					for (int k = 0; k < divideIdSet[j].size(); k++) {
						idToSetId[divideIdSet[j][k]] = divideNewSetId;
					}
				}
			}
		}
	}

	/* ͨ���ϲ����״̬���б�������С����DFA��������Ϊ�ڽӱ� */
	vector<vector<Edge*>> miniDFA(idSetList.size());
	for (int i = 0; i < idSetList.size(); i++) {
		for (int vi = 0; vi < vocabulary.size(); vi++) {
			char input = vocabulary[vi];
			int nowId = idSetList[i][0];	/* �ȼ��࣬��������Ԫ�صȼۣ����ֻ��Ҫȡ��һ�� */
			int nextId = go(nowId, input); 
			if (nextId < 0) {
				continue;
			}
			int nowIdSet = idToSetId[nowId];
			int nextIdSet = idToSetId[nextId];
			miniDFA[nowIdSet].push_back(new Edge(nowIdSet, nextIdSet, input));
		}
	}

	/* ������ʼ״̬����ֹ״̬ */
	int idBegin = idToSetId[idBeginDFA];
	set<int> idEnd;
	for (set<int>::iterator it = idEndDFA.begin(); it != idEndDFA.end(); it++) {
		idEnd.insert(idToSetId[*it]);
	}
	
	clear();
	
	idNumDFA = miniDFA.size();
	idBeginDFA = idBegin;
	idEndDFA = idEnd;
	dfa = miniDFA;
	hasDFA = true;
	minimized = true;
}

void DFA::clear() {
	idNumDFA = 0;
	idBeginDFA = -1;
	idEndDFA.clear();
	for (vector<Edge*> edgeList : dfa) {
		for (Edge* edge : edgeList) {
			delete edge;
		}
		edgeList.clear();
	}
	dfa.clear();
	hasDFA = false;
	minimized = false;
}
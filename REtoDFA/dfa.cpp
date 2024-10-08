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
	/* 当前为nowId的节点，向input边走一步，返回走到的目标节点Id*/
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
	/* 集合t中的所有节点，在NFA中向a边走一步，将所有可以到达的目标节点放入集合m */
	for (set<int>::iterator it = t.begin(); it != t.end(); it++) {
		for (Edge* edge : nfa[*it]) {
			if (edge->input == a) {
				m.insert(edge->nextId);
			}
		}
	}
}

void DFA::closure(set<int>& t) {
	/* 集合t中的所有节点，在NFA中按照bfs的方式沿着epsilon边一直走，将所有可以到达的目标节点放回原集合 */
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
	vector<Edge*> dfaEdgeList;   /* 记录DFA的边，边列表 */
	vector<set<int>> statusList; /* 记录所有构造出的子集，也就是move和epsilon closure后的集合，作为DFA的节点 */
	vector<bool> visitedList;    /* 记录构造出的自己是否被访问过，即是否经过move和closure操作 */
	/* 开始集合的epsilon闭包 */
	set<int> init = { idBeginNFA };
	closure(init);
	statusList.push_back(init);
	visitedList.push_back(false);
	idBeginDFA = 0;
	while (true) {
		/* 找到第一个未被访问过的子集 */
		int id = 0;
		while (id < visitedList.size() && visitedList[id]) {
			id++;
		}
		if (id == visitedList.size()) {
			break;
		}
		set<int> nowset = statusList[id];
		/* 对该集合和字母表中所有字母求move和epsilon闭包 */
		for (char a : vocabulary) {
			set<int> nextset;
			move(nowset, a, nextset);
			closure(nextset);
			if (nextset.empty()) {
				continue;
			}
			/* 将构造后得到的集合插入原来的子集列表中 */
			/* 查找原来的子集列表，没有重复则插入（这里其实也可以用set实现） */
			int nextId = 0;
			while (nextId < statusList.size() && (statusList[nextId] != nextset)) {
				nextId++;
			}
			if (nextId == statusList.size()) {
				statusList.push_back(nextset);
				visitedList.push_back(false);
				/* 该集合若包含nfa的结束状态，则作为DFA的结束状态*/
				if (nextset.find(idEndNFA) != nextset.end()) {
					idEndDFA.insert(nextId);
				}
			}
			/* 无论是否插入构造出了新的DFA节点（子集），都将该转移关系记录到DFA的边列表中 */
			dfaEdgeList.push_back(new Edge(id, nextId, a));
		}
		visitedList[id] = true;
	}

	/* 将边列表根据节点顺序转为邻接表存储 */
	idNumDFA = statusList.size();
	dfa = vector<vector<Edge*>>(idNumDFA);
	for (Edge* edge : dfaEdgeList) {
		dfa[edge->id].push_back(edge);
	}
	hasDFA = true;
}

void DFA::hopcroft() {
	vector<vector<int>> idSetList(2);	/* 记录当前的划分的DFA状态集合，集合列表 */
	vector<int> idToSetId(idNumDFA, 0);	/* 记录每个DFA状态Id对应的集合Id*/
	/* 初始化，将DFA中的节点划分为 终止状态集 和 非终止状态集 */
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
	/* 遍历集合进行划分，直到不能继续划分，即集合列表长度不再变化 */
	int lastLen = 0;
	while (idSetList.size() != lastLen) {
		lastLen = idSetList.size();
		for (int i = 0; i < idSetList.size(); i++) {
			if (idSetList[i].size() == 1) {
				continue;
			}
			for (int vi = 0; vi < vocabulary.size(); vi++) {
				/* 对当前集合idSetList[i]进行一次划分 */
				/* 检查集合中所有状态的nextIdSet（向c走一步后目标状态所在集合的id） */
				char c = vocabulary[vi];
				int nowSetId = idSetList.size();
				vector<vector<int>> divideIdSet; /* 记录所有由nextIdSet相同状态构成的集合 */
				vector<int> divideNextIdSet;     /* 记录集合对应的nextIdSet */
				
				for (int j = 0; j < idSetList[i].size(); j++) {
					/* 获得nextIdSet */
					int nowId = idSetList[i][j];
					int nextId = go(nowId, c); /* 可能为-1，不存在该边，结果为空集 */
					int nextIdSet = nextId;
					if (nextId >= 0) {
						nextIdSet = idToSetId[nextId];
					}
					/* 判断nextIdSet是否相同，若不相同则进行划分 */
					int k = 0;
					while (k < divideNextIdSet.size() && divideNextIdSet[k] != nextIdSet) {
						k++;
					}
					if (k == divideNextIdSet.size()) {
						divideNextIdSet.push_back(nextIdSet);
						divideIdSet.push_back(vector<int>());
					}
					/* 将该元素加入到对应nextIdSet的集合中 */
					divideIdSet[k].push_back(nowId);
				}

				/* 将一次划分后的结果合并入原来的集合列表 */
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

	/* 通过合并后的状态集列表，构建最小化的DFA，并保存为邻接表 */
	vector<vector<Edge*>> miniDFA(idSetList.size());
	for (int i = 0; i < idSetList.size(); i++) {
		for (int vi = 0; vi < vocabulary.size(); vi++) {
			char input = vocabulary[vi];
			int nowId = idSetList[i][0];	/* 等价类，其中任意元素等价，因此只需要取第一个 */
			int nextId = go(nowId, input); 
			if (nextId < 0) {
				continue;
			}
			int nowIdSet = idToSetId[nowId];
			int nextIdSet = idToSetId[nextId];
			miniDFA[nowIdSet].push_back(new Edge(nowIdSet, nextIdSet, input));
		}
	}

	/* 更新起始状态和终止状态 */
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
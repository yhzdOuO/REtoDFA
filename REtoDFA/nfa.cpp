#include "nfa.h"

NFA::NFA() {
    idNumNFA = 0;
    idBeginNFA = -1;
    idEndNFA = -1;
}

void NFA::REtoNFA(string& input) {
    clear();
    processStr(input);
    ThompsonsConstruction();
}

void NFA::processStr(string& input) {
    origin = input.c_str();
    char* charList = new char[origin.size() * 2 + 2];
    char* pc = charList;
    /* 将原式用括号包裹，最后弹出所有字符的操作可以转化为从右括号一直弹出到左括号的操作 */
    *(pc++) = '(';
    bool notFirst = false;
    /* 添加缺省的concatenate符号，便于中缀表达式计算 */
    for (int i = 0; i < input.length(); i++) {
        char tc = input[i];
        if (tc != ')' && tc != '*' && tc != '|') {
            if (notFirst) {
                *(pc++) = charCat;
            }
            else {
                notFirst = true;
            }
        }
        if (tc == '|' || tc == '(') {
            notFirst = false;
        }
        *(pc++) = tc;
    }
    *(pc++) = ')';
    *(pc++) = '\0';
    str = string(charList);
    delete[] charList;
}

void NFA::clear() {
    idNumNFA = 0;
    idBeginNFA = -1;
    idEndNFA = -1;
    vocabulary.clear();
    for (vector<Edge*> edgeList : nfa) {
        for (Edge* edge : edgeList) {
            delete edge;
        }
        edgeList.clear();
    }
    nfa.clear();
}

Node* NFA::convertUnion(vector<Edge*>& edgeList, Node* node1, Node* node2) {
    Node* newNode = new Node(idNumNFA, idNumNFA + 1);
    idNumNFA += 2;
    edgeList.push_back(new Edge(newNode->id, node1->id));
    edgeList.push_back(new Edge(newNode->id, node2->id));
    edgeList.push_back(new Edge(node1->nextId, newNode->nextId));
    edgeList.push_back(new Edge(node2->nextId, newNode->nextId));
    delete node1, node2;
    return newNode;
}

Node* NFA::convertConcate(vector<Edge*>& edgeList, Node* node1, Node* node2) {
    Node* newNode = new Node(node1->id, node2->nextId);
    edgeList.push_back(new Edge(node1->nextId, node2->id));
    delete node1, node2;
    return newNode;
}

Node* NFA::convertStar(vector<Edge*>& edgeList, Node* node) {
    Node* newNode = new Node(idNumNFA, idNumNFA + 1);
    idNumNFA += 2;
    edgeList.push_back(new Edge(newNode->id, node->id));
    edgeList.push_back(new Edge(newNode->id, newNode->nextId));
    edgeList.push_back(new Edge(node->nextId, newNode->nextId));
    edgeList.push_back(new Edge(node->nextId, node->id));
    delete node;
    return newNode;
}

Node* NFA::convertCharacter(vector<Edge*>& edgeList, char c) {
    Node* newNode = new Node(idNumNFA, idNumNFA + 1);
    idNumNFA += 2;
    edgeList.push_back(new Edge(newNode->id, newNode->nextId, c));
    return newNode;
}

void NFA::printNFA() {
    string d = "+++++++++++++++++++++++++++++++";
    cout << d << endl;
    cout << "++++          NFA          ++++" << endl;
    cout << d << endl;

    cout << "vocabulary: [ ";
    for (int vi = 0; vi < vocabulary.size(); vi++) {
        cout << vocabulary[vi] << " ";
    }
    cout << "]" << endl;
    cout << d << endl;

    cout << "  num id: [ " << idNumNFA << " ]" << endl;
    cout << "start id: [ " << idBeginNFA << " ]" << endl;
    cout << "  end id: [ " << idEndNFA << " ]" << endl;
    cout << d << endl;

    cout << "NFA:" << endl;
    for (vector<Edge*> vertex : nfa)
    {
        for (Edge* edge : vertex) {
            cout << "  " << edge->id << "--[" << edge->input << "]-->" << edge->nextId << endl;
        }
    }
    cout << d << endl << endl;
}

void NFA::printStr() {
    string d = "+++++++++++++++++++++++++++++++";
    cout << d << endl;
    cout << "++++          STR          ++++" << endl;
    cout << d << endl;

    cout << "ori：" << "\t" << origin << endl;
    cout << "str：" << "\t" << str << endl;
    cout << d << endl << endl;
}

void NFA::ThompsonsConstruction() {
    vector<Edge*> edgeList;

    /* 操作符栈 和 操作数栈 */
    stack<char> stackOp; // * ( ) | +
    stack<Node*> stackNum;
    set<int> voca;
    map<char, int> prior;
    prior[charCat] = 3;
    prior['|'] = 2;
    /*prior['('] = 1;
    prior[')'] = 1;
    prior['$'] = 0;*/

    /* 计算中缀表达式 */
    for (int i = 0; i < str.length(); i ++) {
        char c = str[i];
        if (c == '*') {
            Node* node = stackNum.top();
            stackNum.pop();

            Node* newNode = convertStar(edgeList, node);
            stackNum.push(newNode);
        }
        else if (c == '(') {
            stackOp.push(c);
        }
        else if (c == '|' || c == charCat || c == ')') {
            while (!stackOp.empty()) {
                char tc = stackOp.top();
                if (c == ')') {
                    if (tc == '(') {
                        stackOp.pop();
                        break;
                    }
                }
                else {
                    if (tc == '(' || prior[tc] < prior[c]) {
                        stackOp.push(c);
                        break;
                    }
                }
                stackOp.pop();
                Node* node2 = stackNum.top();
                stackNum.pop();
                Node* node1 = stackNum.top();
                stackNum.pop();

                Node* newNode = NULL;
                if (tc == charCat) {
                    newNode = convertConcate(edgeList, node1, node2);
                }
                else if (tc == '|') {
                    newNode = convertUnion(edgeList, node1, node2);
                }
                stackNum.push(newNode);
            }
        }
        else {
            voca.insert(c);
            stackNum.push(convertCharacter(edgeList, c));
        }
    }

    /* 构建Epsilon-NFA */
    /* idBeginNFA idEndNFA idNumNFA*/
    Node* lastNode = stackNum.top();
    stackNum.pop();
    idBeginNFA = lastNode->id;
    idEndNFA = lastNode->nextId;

    while (!stackNum.empty()) {
        lastNode = stackNum.top();
        stackNum.pop();
        cout << lastNode->id << " " << lastNode->nextId << endl;
    }
    /* vocabulary */
    for (set<int>::iterator it = voca.begin(); it != voca.end(); it++) {
        vocabulary.push_back(*it);
    }

    /* Epsilon-NFA */
    nfa = vector<vector<Edge*>>(idNumNFA);
    for (Edge* edge : edgeList) {
        int i = edge->id;
        nfa[i].push_back(edge);
    }
}

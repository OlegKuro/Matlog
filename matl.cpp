#include <algorithm>
#include <vector>
#include <stdio.h>
#include <iostream>
#include <Windows.h>
#include <string>
#include <map>

#define ll long long
#define ui unsigned int
#define mp make_pair
#define N 5000

using namespace std;

ll tree[N * N];
bool approved[N * N];
struct Node {
	int verts;
	string s;
	ll hc; //hash-code for equals (inspired by java)
	Node * left_child;
	Node * right_child;

	Node() : hc(0), left_child(NULL), right_child(NULL) {}

	Node(string s, Node * left_child, Node * right_child) : s(s), left_child(left_child), right_child(right_child) {
		int leftRang = 0;
		int rightRang = 0;
		verts = 1; //itself

		if (left_child) {
			leftRang = left_child->verts;
			verts += leftRang;
		}

		if (right_child) {
			rightRang = right_child->verts;
			verts += rightRang;
		}

		hc = 0;
		if (left_child)
			hc += left_child->hc+78;
		hc *= tree[1];
		hc += s[0] + 17;
		if (right_child) {
			hc *= tree[rightRang];
			hc += right_child->hc+85;
		}
	}
	~Node() {
		if (right_child) delete right_child;
		if (left_child) delete left_child;
	}
};


Node *formulas[N*N];
Node *axioms[10];
//string is var??
bool strVar(const string& s) {
	return (s[0] >= 'A' && s[0] <= 'Z'&& !s.empty());
}

bool structEquals(Node * a, Node * b) {
	if (!a && !b) return true;
	if (!a || !b) return false;
	if (a == b) return true;
	if (b->hc != a->hc) return false;
	if (a->s != b->s) return false;
	if (structEquals(a->left_child, b->left_child)) return false;
	if (structEquals(a->right_child, b->right_child)) return false;
	return true;
}


Node * logNot(const string &s, int &it) {
	char c = s[it];
	if (c >= 'A' && c <= 'Z') {
		string name;
		name += c;
		it++;
		while (s[it] >= '0' && s[it] <= '9' && it < s.length()) {
			name += s[it];
			it++;
		}
		return new Node(name, NULL, NULL);
	}
	else if (c == '!') {
		it++;
		Node * expr = logNot(s, it);
		return new Node("!", NULL, expr);
	}
	else if (c == '(') {
		it++;
		Node * expr = implication(s, it);
		if (it >= s.length() || s[it++] != ')') {
			throw "missing closing parenthisis";
		}
		return expr;
	}
	throw "check your input";
}


Node * logAnd(const string &s, int &it) {
	Node * expr = logNot(s, it);
	while (it < s.length() && s[it] == '&') {
		it++;
		Node * expr2 = logNot(s, it);
		expr = new Node("&", expr, expr2);
	}
	return expr;
}

Node * logOr(const string &s, int &it) {
	Node * expr = logAnd(s, it);
	while (it < s.length() && s[it] == '|') {
		it++;
		Node * expr2 = logAnd(s, it);
		expr = new Node("|", expr, expr2);
	}
	return expr;
}

Node * implication(const string &s, int &it) {
	Node * expr1 = logOr(s, it);
	if (it < s.length() && s[it] == '-' && s[++it] == '>') {
		it++;
		Node * expr2 = implication(s, it);
		return new Node("->", expr1, expr2);
	}
	return expr1;
}

Node * parseStringToFormula(const string &s) {
	int it = 0;
	return implication(s, it);
}

//tree output in correct form (?) :D
//UPD: huh... seems like it's correct, which means that tree is correct too
void Print(Node * v) {
	if (v->left_child) {
		cout << "(";
		Print(v->left_child);
		cout << ")";
	}
	cout << v->s;
	if (v->right_child) {
		cout << "(";
		Print(v->right_child);
		cout << ")";
	}
}

//check attentively
bool mapInsert(Node * formula, Node * template_, map<string, vector<Node *> > &variableMap) {
	if (formula == NULL && template_ == NULL) {
		return true;
	}
	if (formula == NULL || template_ == NULL) {
		return false;
	}
	if (formula == template_) {
		return true;
	}
	const string &temp = template_->s;
	if (strVar(temp)) {
		variableMap[temp].push_back(formula);
		return true;
	}
	else {
		if (temp != formula->s) {
			return false;
		}
		return (mapInsert(formula->left_child, template_->left_child, variableMap) &&
			mapInsert(formula->right_child, template_->right_child, variableMap))? true : false;
	}
}


bool templateEquivalence(Node * formula, Node * template_) {
	map<string, vector<Node*> > variableMap;
	if (mapInsert(formula, template_, variableMap)) {
		for (auto& it : variableMap) {
			vector<Node*> &nodes = it.second;
			for (Node* node : nodes) {
				if (!structEquals(node, *nodes.begin())) {
					return false;
				}
			}
		}
		return true;
	}
	return false;
}


int isAxiom(Node * v) {
	for (int i = 0; i < 10; i++) 
		if (templateEquivalence(v, axioms[i])) 
			return i + 1;
	return -1;
}

pair<int, int> isMP(int id) {
	for (int i = id - 1; i >= 0; i--) {
		if (!approved[i]) continue;
		Node * AB = formulas[i];
		if (AB && AB->s == "->" && AB->right_child && formulas[id] && structEquals(AB->right_child, formulas[id])) {
			for (int j = 0; j < id; j++) {
				if (!approved[j]) continue;
				Node * A = formulas[j];
				if (A && AB->left_child && structEquals(A, AB->left_child)) {
					return mp(j, i);
				}
			}
		}
	}
	return mp(-1, -1);
}


int main() {
	string s;
	freopen("in.txt", "right_child", stdin);
	freopen("out.txt", "w", stdout);	
	
	axioms[0] = parseStringToFormula("A->B->A");
	axioms[1] = parseStringToFormula("(A->B)->(A->B->C)->(A->C)");
	axioms[2] = parseStringToFormula("A->B->A&B");
	axioms[3] = parseStringToFormula("A&B->A");
	axioms[4] = parseStringToFormula("A&B->B");
	axioms[5] = parseStringToFormula("A->A|B");
	axioms[6] = parseStringToFormula("B->A|B");
	axioms[7] = parseStringToFormula("(A->C)->(B->C)->(A|B->C)");
	axioms[8] = parseStringToFormula("(A->B)->(A->!B)->!A");
	axioms[9] = parseStringToFormula("!!A->A");

	cin.sync_with_stdio(0);
	cout.sync_with_stdio(0);

	tree[0] = 1;
	tree[1] = 31;

	for (int i = 2; i < N * N; i++) 
		tree[i] = tree[i - 1] * 31;

	string temp;
	int step = 1;
	while (cin >> s) {
		temp = "";
		for (char i : s)
			if (!isspace(i))
				temp += i;
		s = temp; //deletes spaces
		if (s.length() == 0) break; 
		cout << "(" << step << ") " << s;
		try {Node * expr = parseStringToFormula(s);
			formulas[step - 1] = expr;
			int axiomNumber = isAxiom(expr);
			if (axiomNumber != -1) {
				cout << " (Axiom " << axiomNumber << ")" << endl;
				approved[step - 1] = true;
			}
			else {
				pair<int, int> mp = isMP(step - 1);
				if (mp.first != -1) {
					cout << " (M.P. " << mp.first + 1 << " and " << mp.second + 1 << ")" << endl;
					approved[step - 1] = true;
				}
				else {
					cout << " (Not approved)" << endl;
					return 0;
				}
			}
		}
		catch (char const * err) {
			cout << err << " in " << s << endl;
		}
		catch (...) {
			cout << "something wrong..." << endl;
		}
		step++;
	}
	//system("pause");
	return 0;
}
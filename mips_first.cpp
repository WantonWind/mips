#include<iostream>
#include<string>
#include<map>
#include<Functional>

const int INF = 0xffffffff;
typedef long long ll;

using namespace std;
//global variables for later use
char data[1 << 9];		//place to store data
int dtp; 				//point to data
vector<char> buffer;	//used to store temprary byte

//function for preprocess
int align(int a, int b){
	int c = 1 << b;
	int d = a & -a;
	while(d < c && a > 0){
		a -= d;
		d = a & -a;
	}
	a += c;
	return a;
}

void ascii_str(const string& s){
	for(int i = 0; i < s.size(); ++i) data[dtp++] = s[i];
}

void asciiz_str(const string& s){
	ascii_str(s);
	data[dtp++] = 0;
}

void byte(int n){
	for(int i = 0; i < n; ++i) data[dtp++] = vector[i];
}


//function for calculator and controler
pair<int,int> add(ll a, ll b){
	return pair<int,int>((a + b) & INF, 0);
}

pair<int,int> sub(ll a, ll b){
	return pair<int,int>((a - b) & INF, 0);
}

pair<int,int> mul(ll a, ll b){
	ll t = a * b;
	return pair<int,int>((t >> 32) & INF, t & INF);
}

pair<int,int> div(ll a, ll b){
	return pair<int,int>(a % b, a / b);
}

pair<int,int> xxor(ll a, ll b){
	return pair<int,int>(a ^ b, 0);
}

pair<int,int> neg(ll a, ll b){
	return pair<int,int>(-a, 0);
}

pair<int,int> seq(ll a, ll b){
	return pair<int,int>(a == b, 0);
}

pair<int,int> sge(ll a, ll b){
	return pair<int,int>(a >= b, 0);
}

pair<int,int> sgt(ll a, ll b){
	return pair<int,int>(a > b, 0);
}

pair<int,int> sle(ll a, ll b){
	return pair<int,int>(a <= b, 0);
}

pair<int,int> slt(ll a, ll b){
	return pair<int,int>(a < b, 0);
}

pair<int,int> sne(ll a, ll b){
	return pair<int,int>(a != b, 0);
}

//function for syscall
int print_int(int a, int b){
	cout << a;
	return 0;
}

int print_string(int a, int b){
	while(data[a]) cout << data[a++];
	return 0;
}

int get_int(int a, int b){
	cin >> a;
	return a;
}

int get_string(int a, int b){
	for(int i = 0; i < b; ++i) cin >> data[a + i];
	return 0;
	//throw something ??
}

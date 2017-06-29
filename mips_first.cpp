#include<iostream>
#include<string>
#include<map>
#include<vector>
#include<deque>
#include<functional>

const int INF = 0xffffffff;
const int SHF = 0x0000ffff;
const int CHF = 0x000000ff;
const int lo = 32;
const int hi = 33;
const int v0 = 2;
const int a0 = 4;
const int a1 = 5;
typedef long long ll;

using namespace std;

//declaration
class ins;

//global variables for later use
int reg[34];				//place to store register
char data[1 << 9];			//place to store data
int dtp; 					//point to data
int nxt;					//point to next instraction
vector<ins> instraction;	//place to store instraction
vector<char> buffer;		//used to store temprary byte

bool mem_occupied;			//mem is occupied
bool reg_occupied[34];		//reg is occupied
bool control_hazard;		//control hazard has appeared.

vector<function<pair<int,int>(ll,ll)>> op_to_func;	//mapping from op to function
map<int,function<int(int,int)>> v0_to_func;			//mapping from v0 to function
map<string,int> rn_to_rp;			//mapping from name of register to place of register
map<int,int> la_to_r;				//mapping from label to row of ins

//function for preprocess
void align(int n){
	int c = 1 << n;
	int d = dtp & -dtp;
	while(d < c && dtp > 0){
		dtp -= d;
		d = dtp & -dtp;
	}
	dtp += c;
}						//only for dtp

void ascii_str(const string& s){
	for(int i = 0; i < s.size(); ++i) data[dtp++] = s[i];
}

void asciiz_str(const string& s){
	ascii_str(s);
	data[dtp++] = 0;
}

void byte(int n){
	for(int i = 0; i < n; ++i) data[dtp++] = buffer[i] & CHF;
}

void half(int n){
	for(int i = 0; i < n; ++i){
		data[dtp++] = (buffer[i] >> 8) & CHF;
		data[dtp++] = buffer[i] & CHF;
	}
}

void word(int n){
	for(int i = 0; i < n; ++i){
		data[dtp++] = (buffer[i] >> 24) & CHF;
		data[dtp++] = (buffer[i] >> 16) & CHF;
		data[dtp++] = (buffer[i] >> 8) & CHF;
		data[dtp++] = buffer[i] & CHF;
	}
}

void space(int n){
	dtp += n;
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
	return pair<int,int>((a % b) & INF, (a / b) & INF);
}

pair<int,int> xxor(ll a, ll b){
	return pair<int,int>((a ^ b) & INF, 0);
}

pair<int,int> neg(ll a, ll b){
	return pair<int,int>((-a) & INF, 0);
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

//class ins used to store instruction briefly
struct ins{
	int op;		//操作类型+功能
	int rs;		//第一个操作数的寄存器地址 (不存在为-1 load/store)
	int rt;		//第二个操作数的寄存器地址（当第二个数不存在或者为立即数的时候默认为-1）
	int rd;		//结果寄存器地址（-1代表low和high）
	int sc;		//位移量/立即数   //??? why use ll
	int lp;		//label 序号

	ins(){}		//rs rs -1??
	ins(string ex){
		// to do
	}
	//copy constructor is implicitly called.
};

//class for processing in the pipelining. 
class regulator{
protected:
	ins ii;
	int clock;
public:
	regulator(ins ex):ii(ex),clock(2){}
	int stage(){ return clock; }
	virtual int console() = 0;
	virtual bool is_controlor() = 0;
};

class calculator : public regulator{
private:
	function<pair<int,int>(ll,ll)> calc;
	int A;
	int B;
	pair<int,int> ans;
	
	bool prepare(){
		if(!reg_occupied[ii.rs] && ii.rt == -1 || !reg_occupied[ii.rt]){
			calc = op_to_func[ii.op];
			A = reg[ii.rs];
			if(ii.rt == -1) B = ii.sc;
			else B = reg[ii.rt];
			if(ii.rd != -1) reg_occupied[ii.rd] = true;
			else{
				reg_occupied[lo] = true;
				reg_occupied[hi] = true;
			}
			return true;
		}
		else return false;
	}	
		
	void execute(){
		if(ii.op <= 8)	ans = calc(A,B);
		else ans = calc(unsigned(A),unsigned(B));
	}
	
	void write_back(){
		if(ii.rd == -1){
			reg[lo] = ans.first;
			reg[hi] = ans.second;
			reg_occupied[lo] = false;
			reg_occupied[hi] = false;
		}
		else{
			reg[ii.rd] = ans.first;
			reg_occupied[ii.rd] = false;
		}
	}
public:
	calculator(const ins& ex):regulator(ex){}
	int console(){
		int suc = true;
		if(stage() == 2) suc = prepare();
		if(stage() == 3) execute();
		if(stage() == 4) suc = -1;
		if(stage() == 5) write_back();
		if(suc) ++clock;
		return suc;
	}
	bool is_controlor(){ return false; }
};

class controlor : public regulator{
private:
	function<pair<int,int>(ll,ll)> calc;
	int A;
	int B;
	bool ans;
	
	bool prepare(){
		if(ii.op <= 36){
			if(!reg_occupied[ii.rs] && ii.rt == -1 || !reg_occupied[ii.rt]){
				calc = op_to_func[ii.op];
				A = reg[ii.rs];
				if(ii.rt == -1) B = 0;
				else B = reg[ii.rt];
				return true;
			}
			return false;				
		}
		else if(ii.op == 39 || ii.op == 41)	ii.lp = reg[ii.rs];
		A = B = true;
		return true;
	}		//different from that of calculator
	
	void execute(){
		ans = calc(A,B).first;
	}
	
	void write_back(){
		if(ans){
			if(ii.op == 40 || ii.op == 41) reg[31] = nxt;
			nxt = ii.lp;		
		}	
	}
public:
	controlor(const ins& ex):regulator(ex){}
	int console(){
		int suc = true;
		if(stage() == 2) suc = prepare();
		if(stage() == 3) execute();
		if(stage() == 4) suc = -1;
		if(stage() == 5) write_back();
		if(suc) ++clock;
		return suc;
	}
	bool is_controlor(){ return true; }
};

class load : public regulator{
private:
	int pa;		//original address / mem_buffer
	int ans;
	bool prepare(){
		if(ii.rs == -1){
			reg_occupied[ii.rd] = true;
			pa = ii.lp;
			return true;
		}
		if(!reg_occupied[ii.rs]){
			reg_occupied[ii.rd] = true;
			pa = reg[ii.rs];
			return true;
		}
		return false;
	}
	
	void execute(){
		if(ii.rs == -1){
			ans = pa;
		}
		else ans = pa + ii.sc;
	}
	
	void mem_access(){
		pa = 0;
		if(ii.op == 42) pa = ans;
		if(ii.op == 43) pa += data[ans];
		if(ii.op == 44) for(int i = 0; i < 2; ++i)	pa += int(data[ans+i]) << (8 - i * 8);
		if(ii.op == 45) for(int i = 0; i < 4; ++i)	pa += int(data[ans+i]) << (24 - i * 8);
	}
	
	void write_back(){
		reg[ii.rd] = pa;
		reg_occupied[ii.rd] = false;
	}
public:
	load(const ins& ex):regulator(ex){}
	int console(){
		int suc = true;
		if(stage() == 2) suc = prepare();
		if(stage() == 3) execute();
		if(stage() == 4) mem_access();
		if(stage() == 5) write_back();
		if(suc) ++clock;
		return suc;
	}
	bool is_controlor(){ return false; }
};

class store : public regulator{
private:
	int pa;		//original address / mem_buffer
	int C;		//store data
	int ans;
	bool prepare(){
		if(!reg_occupied[ii.rd]){
			C = reg[ii.rd];
			if(ii.rs == -1){
				pa = ii.lp;
				return true;
			}
			if(!reg_occupied[ii.rs]){
				pa = reg[ii.rs];
				return true;
			}			
		}
		return false;
	}
	
	void execute(){
		if(ii.rs == -1){
			ans = pa;
		}
		else ans = pa + ii.sc;
	}
	
	void mem_access(){
		if(ii.op == 46)	data[ans] = C & CHF;
		if(ii.op == 47)	for(int i = 0; i < 2; ++i) data[ans + i] = ( C >> (8 - i * 8) ) & CHF;
		if(ii.op == 48)	for(int i = 0; i < 4; ++i) data[ans + i] = ( C >> (24 - i * 8) ) & CHF;		
	}
public:
	store(const ins& ex):regulator(ex){}
	int console(){
		int suc = true;
		if(stage() == 2) suc = prepare();
		if(stage() == 3) execute();
		if(stage() == 4) mem_access();
		if(suc) ++clock;
		return suc;
	}
	bool is_controlor(){ return false; }	
};

class mover : public regulator{
private:
	int A;
	bool prepare(){
		if(ii.rs == -1){
			reg_occupied[ii.rd] = true;
			A = ii.sc;
			return true;
		}
		if(!reg_occupied[ii.rs]){
			reg_occupied[ii.rd] = true;
			A = reg[ii.rs];
			return true;
		}
		return false;
	}
	
	void write_back(){
		reg[ii.rd] = A;
		reg_occupied[ii.rd] = false;
	}
public:
	mover(const ins& ex):regulator(ex){}
	int console(){
		int suc = true;
		if(stage() == 2) suc = prepare();
		if(stage() == 4) suc = -1;
		if(stage() == 5) write_back();
		if(suc) ++clock;
		return suc;
	}
	bool is_controlor(){ return false; }
};

class nop : public regulator{
public:
	nop(const ins& ex):regulator(ex){}
	int console(){
		int suc = true;
		if(stage() == 4) suc = -1;
		++clock;
		return suc;
	}
	bool is_controlor(){ return false; }
};

class syscall : public regulator{
private:
	int v0_data;		//store the data of $v0
	int a0_data;		//store the data of $a0
	int a1_data;		//store the data of $a1
	string s;
	
	bool prepare(){
		if(!reg_occupied[a0] && !reg_occupied[a1]){
			reg_occupied[v0] = true;
			v0_data = reg[v0];
			a0_data = reg[a0];
			a1_data = reg[a1];
			return true;
		}
		return false;
	}
	
	void execute(){
		switch(v0_data){
			case 1:	
				cout << a0_data;
				break;
			case 4:	
				while(data[a0_data]) cout << data[a0_data++]; 
				cout << '\0';
				break;
			case 5:	
				cin >> v0_data;
				break;
			case 8:	
				//
				break;
			case 9:	
				//
				break;
			case 10:
				exit(0);
			case 17:
				exit(a0_data);
		}
	}
	
	void write_back(){
		reg[v0] = v0_data;
		reg_occupied[v0] = false;
	}
public:
	syscall(const ins& ex):regulator(ex){}
	int console(){
		int suc = true;
		if(stage() == 2) suc = prepare();
		if(stage() == 3) execute();
		if(stage() == 4) ; // need polish
		if(stage() == 5) write_back();
		if(suc) ++clock;
		return suc;
	}
	bool is_controlor(){ return true; }
};

//pipelining
class pipeline{
private:
	deque<regulator*> que;
	ins copy;
	bool copied;		//whether copy is newest
	
	void push_regulator(const ins& ex){
		regulator* rl;
		if(ex.op <= 24) rl = new calculator(ex);
		else if(ex.op <= 41) rl = new controlor(ex);
		else if(ex.op <= 45) rl = new load(ex);
		else if(ex.op <= 48) rl = new store(ex);
		else if(ex.op <= 52) rl = new mover(ex);
		else if(ex.op == 53) rl = new nop(ex);
		else rl = new syscall(ex);
		que.push_back(rl);
	}		//process this ins
	void pop_regulator(){
		delete que.front();
		que.pop_front();
	}		//pop finished instraction
	
	void ins_fecth(){
		if(nxt == instraction.size()) return;
		if(!copied){
			copy = instraction[nxt];  //copy an back-up
			copied = true;		
		}
		if(copied){
			if(!mem_occupied && !control_hazard){  //delete reg_occupied
				push_regulator(copy);
				++nxt;
				copied = false;
				if(25 <= copy.op && copy.op <= 41 || copy.op == 54) control_hazard = true;
			}
		}
	}		//instraction fecth stage
	
	void scan(){
		mem_occupied = false;
		for(deque<regulator*>::iterator it = que.begin(); it != que.end(); ++it){
			int rec = (*it)->console();
			if(!rec) return;
			if((*it)->stage() == 4 && rec != -1) mem_occupied = true;
		}
		if((*que.begin())->stage() == 6){
			pop_regulator();
			if(que.empty() || (*--que.end())->is_controlor() == 0) control_hazard = false; //*-- ??
		}
		ins_fecth();
	}
public:
	pipeline():copied(false){}
	void console(){
		while(nxt < instraction.size() || !que.empty()) scan();
	}
};
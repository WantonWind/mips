#ifndef REG_CLASS
#define REG_CLASS

class reg_class {
private:
	int reg[34];
public:
	reg_class() {
		for (int i = 0; i < 34; ++i) reg[i] = 0;
	//	reg[29] = 1 << 24;
	}
	reg_class(const reg_class& ex) {
		for (int i = 0; i < 34; ++i) reg[i] = ex.reg[i];
	}
	reg_class& operator=(const reg_class& ex) {
		for (int i = 0; i < 34; ++i) reg[i] = ex.reg[i];
		return *this;
	}
	int& operator[](const int& i) {
		return reg[i];
	}
	void reset() {
		for (int i = 0; i < 34; ++i) reg[i] = 0;
	}
};

#endif
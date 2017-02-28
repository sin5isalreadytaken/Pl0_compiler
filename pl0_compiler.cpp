#include<iostream>
#include<string>
#include<iomanip>
#include<vector>
#include<map>
#include<fstream>
using namespace std;

typedef struct{
	char name[11];
	string type;//标识符基本类型
	string kind;//标识符种类，包括常量变量函数过程和函数过程的参数
	int arrayFlag;
	int adrFlag;//0表示地址调用，1表示传值调用
	int index;
	int level;
	int address;
}tableStruct;
typedef struct{
	int fparaNum;
	string freturnVal;//返回类型
	tableStruct fpara[100];
}funcStruct;
typedef struct{
	int pparaNum;
	tableStruct ppara[100];
}procStruct;
typedef struct{
	string constType;
	int integer;
	char c;
	double real;
}constStruct;

FILE *fin;
const int norw = 22, al = 10, nmax = 14, txmax = 100, amax = 10000000000000, levmax = 3, cxmax = 10000;
const double typeInt = 0.00000001;

string wsym[22] = { "arraysym", "beginsym", "charsym", "constsym", "dosym", "downtosym", "elsesym", "endsym", "forsym", "funcsym", "ifsym", "intsym", "ofsym", "procsym", "readsym", "realsym", "repeatsym", "thensym", "tosym", "untilsym", "varsym", "writesym" };
string word[22] = { "array     ", "begin     ", "char      ", "const     ", "do        ", "downto    ", "else      ", "end       ", "for       ", "function  ", "if        ", "integer   ", "of        ", "procedure ", "read      ", "real      ", "repeat    ", "then      ", "to        ", "until     ", "var       ", "write     " };
string Error[] = { "未声明标识符", "标识符为数组", "缺少右中括号", "标识符为函数名", "缺少右括号", "该标识符不能为因子", "数值过大", "不是分程序的开始或后继", "不是因子后继", "不合法token",
"常量定义应用等号", "常量定义不是常量", "常量定义缺少等号", "不是常量标识符", "标识符错误", "类型不正确", "缺少冒号", "缺少左中括号", "不是语句后继", "缺少of",
"标识符为数组", "标识符不是数组", "缺少赋值号", "赋值语句错误", "语句错误", "缺少then", "缺少until", "缺少分号", "缺少end", "缺少左括号",
"for语句错误", "嵌套过深", "数组大小错误", "标识符已声明", "缺少do", "条件语句错误", "读入非变量", "缺少begin", "program incomplete!",
"不是常量声明后继", "不是变量声明后继", "不是过程声明后继", "不是函数声明后继", "函数赋值类型不一致", "变量赋值类型不一致", "参数个数不一致", "参数类型不匹配", "函数赋值语句位置错误" };

struct instruction{
	string f;
	int l;
	double a;
}code[10000];

int err = 0, cc = 0, cx = 0, ll = 0, kk = al, numofConst = 0, numofFunc = 0, numofProc = 0, funcFlag = 0, ln = 0;
double num = 0;

char  letter = ' ', ch = ' ', formerCh = ' ', line[81], a[11] = { "          " }, id[11] = { "          " };
string sym = "", str = "";

tableStruct table[20000];
funcStruct funcTable[500];
procStruct procTable[500];
constStruct constTable[10000];

vector<string> declbegsys = { "constsym", "varsym", "procsym", "funcsym" };//声明开始符号集合
vector<string> statbegsys = { "beginsym", "ifsym", "forsym", "repeatsym" };//语句开始
vector<string> facbegsys = { "ident", "int", "real", "lparen" };//因子开始符号集合
vector<string> f2 = { "" };

void error(int n){
	if (n == 0)
		return;
	cout << "line" << ln << ":error" << n << ": " << Error[n - 1] << endl;
	err = err + 1;
	return;
}

int domain(int k){
	int result = 1;
	for (int i = 0; i < k; i++){
		result *= 10;
	}
	return result;
}

void getch(){
	if (cc == ll){
		if ((ch = fgetc(fin)) == EOF){
			if (formerCh == '.'){
				ch = '\0';
				return;
			}
			else{
				cout << "                     error39: program incomplete!" << endl;
				ch = '\0';
				fclose(fin);
				return;
			}
		}
		ll = 0;
		cc = 0;
		while (ch != '\n'){
			ll = ll + 1;
			line[ll] = ch;
			formerCh = ch;
			if ((ch = fgetc(fin)) == EOF){
				break;
			};
		}
		ll = ll + 1;
		line[ll] = '\n';
		ln++;
	}
	cc = cc + 1;
	ch = line[cc];
	return;
}

int stringComp(string a, string b){
	for (int i = 1; i < 11; i++){
		if (a.at(i) > b.at(i - 1))
			return 1;
		else if (a.at(i) < b.at(i - 1))
			return -1;
	}
	return 0;
}

void getsym(){
	int i, j, k;
	string ssym[55];
	for (int x = 0; x < 55; x++){
		ssym[x] = "nul";
	}
	ssym['+' - '('] = "plus";        ssym['-' - '('] = "minus";
	ssym['*' - '('] = "times";       ssym['/' - '('] = "slash";
	ssym['(' - '('] = "lparen";      ssym[')' - '('] = "rparen";
	ssym['=' - '('] = "eql";         ssym[',' - '('] = "comma";
	ssym['.' - '('] = "period";		 ssym[';' - '('] = "semicolon";
	ssym['<' - '('] = "lss";         ssym['>' - '('] = "gtr";
	ssym['[' - '('] = "lmparen";	 ssym[']' - '('] = "rmparen";
	while (ch == ' ' || ch == '\n' || ch == '\t')
		getch();
	if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')){
		k = 0;
		while ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9')){
			k = k + 1;
			a[k] = ch;
			getch();
		}
		if (k >= kk){
			kk = k;
		}
		else{
			while (kk != k){
				a[kk] = ' ';
				kk = kk - 1;
			}
		}
		for (int x = 0; x <= 10; x++){
			id[x] = a[x];
		}
		i = 0;
		j = norw - 1;
		while (i <= j){
			k = (i + j) / 2;
			string id1(id);
			if (stringComp(id1, word[k]) == 1)
				i = k + 1;
			else if (stringComp(id1, word[k]) == -1)
				j = k - 1;
			else{
				i = k + 1;
				j = k - 1;
			}
		}
		if (i - 1 > j)
			sym = wsym[k];
		else
			sym = "ident";
		/*for (int x = 1; x < 11; x++){
		cout << id[x];
		}
		cout << " ";*/
	}
	else if (ch == '"'){
		str = "";
		getch();
		while (ch == 32 || ch == 33 || (ch >= 35 && ch <= 126)){
			str += ch;
			getch();
		}
		if (ch == '"' && str.length() > 0){
			sym = "string";
			//cout << "\"" << str << "\" ";
			getch();
		}
		else{
			sym = "nul";
			//cout << "\"" << str << " ";
		}
	}
	else if (ch == '\''){
		getch();
		if ((ch >= 'a'&&ch <= 'z') || (ch >= 'A' && ch <= 'Z')){
			letter = ch;
			getch();
			if (ch == '\''){
				sym = "char";
				//cout << "\'" << letter << "\' ";
				getch();
			}
			else{
				sym = "nul";
				//cout << "\'" << letter << " ";
			}
		}
		else{
			sym = "nul";
			//cout << "\' ";
		}
	}
	else if (ch >= '0' && ch <= '9'){
		k = 0;
		num = 0;
		sym = "int";
		while (ch >= '0' && ch <= '9'){
			num = 10 * num + ch - '0';
			k = k + 1;
			getch();
		}
		if (k > nmax){
			error(7);
		}
		if (ch == '.'){
			sym = "real";
			k = 0;
			getch();
		}
		while (ch >= '0' && ch <= '9'){
			num = num + (double)(ch - '0') / domain(k + 1);
			k = k + 1;
			getch();
		}
		if (k > nmax){
			error(7);
		}
		else if (k == 0){
			cc -= 1;
			ch = '.';
			sym = "int";
			//cout << std::left << setw(10)<<(int)num << " ";
		}
		else
			//cout << std::left << setw(10) << num << " "
			;
	}
	else if (ch == ':'){
		getch();
		if (ch == '='){
			sym = "becomes";
			//cout << std::left << setw(11) << ":=";
			getch();
		}
		else{
			sym = "colon";
			//cout << std::left << setw(11) << ":";
		}
	}
	else if (ch == '<'){
		getch();
		if (ch == '='){
			sym = "leq";
			//cout << std::left << setw(11) << "<=";
			getch();
		}
		else if (ch == '>'){
			sym = "neq";
			//cout << std::left << setw(11) << "<>";
			getch();
		}
		else{
			sym = "lss";
			//cout << std::left << setw(11) << "<";
		}
	}
	else if (ch == '>'){
		getch();
		if (ch == '='){
			sym = "geq";
			//cout << std::left << setw(11) << ">=";
			getch();
		}
		else{
			sym = "gtr";
			//cout << std::left << setw(11) << ">";
		}
	}
	else if (ch == '\0'){
		sym = "exit";
		exit(0);
	}
	else{
		if (ch - '(' < 0 || ch - '('> 54){
			sym = "nul";
		}
		else
			sym = ssym[ch - '('];
		//cout << std::left << setw(10) << ch << " ";
		getch();
	}
	//cout <<sym << endl;
}

int symIn(vector<string> f){
	for (int x = 0; x < f.size(); x++){
		if (f[x].compare(sym) == 0)
			return 1;
	}
	return 0;
}

void stringArrayCat(vector<string> &s1, vector<string> s2){
	for (int x = 0; x < s2.size(); x++){
		s1.push_back(s2[x]);
	}
	return;
}

void test(vector<string> &s1, vector<string> s2, int n){
	int i = s1.size();
	int j = s2.size();
	if (!symIn(s1)){
		error(n);
		stringArrayCat(s1, s2);
		while (!symIn(s1)){
			getsym();
		}
	}
	return;
}

int arrayPosition(int tx){
	int i = tx;
	string s;
	string s1;
	for (int x = 0; x < 10; x++){
		s1 += id[x];
	}
	while (i > 0){
		s = table[i].name;
		if (s.compare(s1) == 0)
			break;
		i--;
	}
	if (i > 0){
		while (i > 0 && s.compare(s1) == 0){
			i--;
			s = table[i].name;
		}
		i++;
	}
	return i;
}

int position(int tx){
	int i = tx;
	string s;
	string s1;
	for (int x = 0; x < 10; x++){
		s1 += id[x];
	}
	while (i > 0){
		s = table[i].name;
		if (s.compare(s1) == 0)
			break;
		i--;
	}
	return i;
}

void gen(string x, int y, double z){
	if (cx > cxmax){
		cout << "program too long" << endl;
		fclose(fin);
		exit(0);
	}
	code[cx].f = x;
	code[cx].l = y;
	code[cx].a = z;
	cx = cx + 1;
	return;
}

void enter(string t, string k, int aF, int adrF, int l, int i, int &dx, int &tx){
	tx = tx + 1;
	for (int x = 0; x < 10; x++){
		table[tx].name[x] = id[x];
	}
	table[tx].name[10] = '\0';
	table[tx].kind = k;
	table[tx].type = t;
	table[tx].arrayFlag = aF;
	table[tx].adrFlag = adrF;
	table[tx].index = i;
	if (k.compare("variable") == 0){
		table[tx].level = l;
		table[tx].address = dx;
		dx = dx + 1;
	}
	else if (k.compare("procedure") == 0){
		table[tx].level = l;
	}
	else if (k.compare("function") == 0){
		table[tx].level = l;
	}
	return;
}

void enterConst(string t){
	constTable[numofConst++].constType = t;
	if (t.compare("int") == 0){
		if (num > amax){
			error(7);//同上，数值过大
			num = 0;
		}
		constTable[numofConst - 1].integer = (int)num;
	}
	else if (t.compare("real") == 0){
		if (num > amax){
			error(7);//同上，数值过大
			num = 0;
		}
		constTable[numofConst - 1].real = num;
	}
	else{
		constTable[numofConst - 1].c = letter;
	}
	return;
}

void enterFunc(string t, int n){
	funcTable[numofFunc - 1].freturnVal = t;
	funcTable[numofFunc - 1].fparaNum = n;
}

void enterProc(int n){
	procTable[numofProc - 1].pparaNum = n;
}

void constDeclaration(int lev, int &tx, int &dx){
	int minusFlag = 0;
	char s[11];
	if (sym.compare("ident") == 0){
		for (int x = 0; x < 10; x++){
			s[x] = id[x];
		}
		getsym();
		if (sym.compare("eql") == 0 || sym.compare("becomes") == 0){
			if (sym.compare("becomes") == 0){
				error(11);//常量定义应该用等号
			}
			getsym();
			if (sym.compare("plus") == 0 || sym.compare("minus") == 0){
				if (sym.compare("minus") == 0){
					minusFlag = 1;
				}
				getsym();
				if (sym.compare("int") == 0 || sym.compare("real") == 0){
					if (minusFlag){
						num = -num;
					}
					for (int x = 0; x < 10; x++){
						id[x] = s[x];
					}
					int tmpi = position(tx);
					if (tmpi != 0 && table[tmpi].level == lev){
						error(34);//标识符已声明
					}
					else{
						enter(sym, "constant", 0, 0, lev, numofConst, dx, tx);
						enterConst(sym);
						//cout << "                     This is a constant declaration!" << endl;
					}
					getsym();
				}
				else{
					error(12);//常量定义不是常量
				}
			}
			else if (sym.compare("int") == 0 || sym.compare("real") == 0 || sym.compare("char") == 0){
				if (position(tx) != 0){
					error(34);//标识符已声明
				}
				else{
					enter(sym, "constant", 0, 0, lev, numofConst, dx, tx);
					enterConst(sym);
					//cout << "                     This is a constant declaration!" << endl;
				}
				getsym();
			}
			else{
				error(12);//同上，常量定义不是常量
			}
		}
		else{
			error(13);//常量定义找不到等号
		}
	}
	else{
		error(14);//不是常量标识符
	}
	vector<string> f = { "semicolon", "comma", "varsym", "procsym", "funcsym", "writesym", "readsym" };
	stringArrayCat(f, statbegsys);
	test(f, f2, 0);
	return;
}

void varDeclaration(int lev, int &tx, int &dx){
	char temp[1000][11];
	int numofIdent = 0, numofElement = 0;
	if (sym.compare("ident") == 0){
		for (int x = 0; x < 10; x++){
			temp[numofIdent][x] = id[x];
		}
		temp[numofIdent++][10] = '\0';
		getsym();
		while (sym.compare("comma") == 0){
			getsym();
			if (sym.compare("ident") == 0){
				for (int x = 0; x < 10; x++){
					temp[numofIdent][x] = id[x];
				}
				temp[numofIdent++][10] = '\0';
				getsym();
			}
			else{
				error(15);//变量标识符错误
			}
		}
		if (sym.compare("colon") == 0){
			getsym();
		}
		else{
			error(17);//缺少冒号
		}
		string f = "";
		if (sym.compare("intsym") == 0 || sym.compare("realsym") == 0 || sym.compare("charsym") == 0){
			if (sym.compare("intsym") == 0){
				f = "int";
			}
			else if (sym.compare("realsym") == 0){
				f = "real";
			}
			else if (sym.compare("charsym") == 0){
				f = "char";
			}
			for (int x = 0; x < numofIdent; x++){
				for (int y = 0; y < 11; y++){
					id[y] = temp[x][y];
				}
				if (position(tx) != 0 && table[position(tx)].level == lev){
					error(34);//同上，标识符已声明
				}
				else
					enter(f, "variable", 0, 0, lev, 0, dx, tx);
			}
			//cout << "                     This is a variable declaration!" << endl;
			getsym();
		}
		else if (sym.compare("arraysym") == 0){
			getsym();
			if (sym.compare("lmparen") == 0){
				getsym();
			}
			else{
				error(18);//缺少左中括号
			}
			if (sym.compare("int") == 0){
				numofElement = (int)num;
				getsym();
				if (sym.compare("rmparen") == 0){
					getsym();
				}
				else{
					error(3);//同上，缺少右中括号
				}
				if (sym.compare("ofsym") == 0){
					getsym();
				}
				else{
					error(20);//缺少of
				}
				string f = "";
				if (sym.compare("intsym") == 0 || sym.compare("realsym") == 0 || sym.compare("charsym") == 0){
					if (sym.compare("intsym") == 0){
						f = "int";
					}
					else if (sym.compare("realsym") == 0){
						f = "real";
					}
					else if (sym.compare("charsym") == 0){
						f = "char";
					}
					for (int x = 0; x < numofIdent; x++){
						for (int y = 0; y < 11; y++){
							id[y] = temp[x][y];
						}
						if (position(tx) != 0 && table[position(tx)].level == lev){
							error(34);//同上，标识符已声明
						}
						else{
							for (int x = 0; x < numofElement; x++){
								enter(f, "variable", numofElement, 0, lev, 0, dx, tx);
							}
						}
					}
					//cout << "                     This is a variable declaration!(array)" << endl;
					getsym();
				}
				else{
					error(16);//类型不正确
				}
			}
			else{
				error(33);//数组大小错误
			}
		}
		else{
			error(16);//类型不正确
		}
	}
	else{
		error(15);//不是标识符
	}
	return;
}

int expression(int lev, int tx, vector<string> fsys);

int argument(int flag, int i, int lev, int tx, vector<string> f){
	int p = tx + 1, txtemp = 0;
	if (flag){
		for (int x = 0; x < funcTable[table[i].index - 1].fparaNum; x++){
			table[++tx] = funcTable[table[i].index - 1].fpara[x];
			txtemp++;
		}
		if (funcTable[table[i].index - 1].fparaNum == 0)
			return 0;
		flag = 4;
	}
	else{
		for (int x = 0; x < procTable[table[i].index - 1].pparaNum; x++){
			table[++tx] = procTable[table[i].index - 1].ppara[x];
			txtemp++;
		}
		if (procTable[table[i].index - 1].pparaNum == 0)
			return 0;
		flag = 3;
	}
	tx -= txtemp;
	int numofA = 0, t = 0;
	string strType;
	int type;
	if (table[p].adrFlag == 0){
		vector<string> f1 = { "comma" };
		stringArrayCat(f1, f);
		type = expression(lev, tx, f1);
		if (type == 1)
			strType = "char";
		else if (type == 2)
			strType = "int";
		else if (type == 3)
			strType = "real";
		if (table[p].type.compare(strType) != 0){
			error(47);//参数类型不匹配
		}
		gen("stoPara", lev - table[p].level + 1, numofA + flag);
	}
	else{
		if (sym.compare("ident") != 0){
			error(48);//实参错误
		}
		else{
			int j;
			j = position(tx);
			if (j == 0){
				error(1);//未声明标识符
			}
			else{
				if (table[j].kind.compare("variable") != 0){
					error(48);
				}
				else{
					if (table[j].arrayFlag){
						j = arrayPosition(j);
						getsym();
						if (sym.compare("lmparen") != 0){
							error(18);
						}
						else{
							getsym();
							vector<string> f1 = { "rmparen" };
							stringArrayCat(f1, f);
							expression(lev, tx, f1);
							gen("template", 0, table[j].arrayFlag);
							if (sym.compare("rmparen") != 0){
								error(3);
							}
							if (table[j].adrFlag)
								gen("lod", lev - table[j].level, table[j].address);
							else
								gen("lodArrA", lev - table[j].level, table[j].address);
							gen("stoAddr", lev - table[p].level + 1, table[p].address);
						}
					}
					else{
						if (table[j].adrFlag)
							gen("lod", lev - table[j].level, table[j].address);
						else
							gen("lodAddr", lev - table[j].level, table[j].address);
						gen("stoAddr", lev - table[p].level + 1, table[p].address);
					}
				}
			}
			getsym();
		}
	}
	numofA++;
	while (sym.compare("comma") == 0){
		getsym();
		p++;
		if (table[p].adrFlag == 0){
			vector<string> f1 = { "comma" };
			stringArrayCat(f1, f);
			type = expression(lev, tx, f1);
			if (type == 1)
				strType = "char";
			else if (type == 2)
				strType = "int";
			else if (type == 3)
				strType = "real";
			if (table[p].type.compare(strType) != 0){
				error(47);//参数类型不匹配
			}
			gen("stoPara", lev - table[p].level + 1, numofA + flag);
		}
		else{
			if (sym.compare("ident") != 0){
				error(48);//实参错误
			}
			else{
				int j;
				j = position(tx);
				if (j == 0){
					error(1);//未声明标识符
				}
				else{
					if (table[j].kind.compare("variable") != 0){
						error(48);
					}
					else{
						if (table[j].arrayFlag){
							j = arrayPosition(j);
							getsym();
							if (sym.compare("lmparen") != 0){
								error(18);
							}
							else{
								getsym();
								vector<string> f1 = { "rmparen" };
								stringArrayCat(f1, f);
								expression(lev, tx, f1);
								gen("template", 0, table[j].arrayFlag);
								if (sym.compare("rmparen") != 0){
									error(3);
								}
								if (table[j].adrFlag)
									gen("lod", lev - table[j].level, table[j].address);
								else
									gen("lodArrA", lev - table[j].level, table[j].address);
								gen("stoAddr", lev - table[p].level + 1, table[p].address);
							}
						}
						else{
							if (table[j].adrFlag)
								gen("lod", lev - table[j].level, table[j].address);
							else
								gen("lodAddr", lev - table[j].level, table[j].address);
							gen("stoAddr", lev - table[p].level + 1, table[p].address);
						}
					}
				}
				getsym();
			}
		}
		numofA++;
	}
	return numofA;
}

int factor(int lev, int tx, vector<string> fsys){//因子处理
	int i; int type = 2;
	vector<string> f1 = facbegsys;
	test(f1, fsys, 10);//不合法token（不是因子？）
	if (symIn(f1)){//循环！！！
		if (sym.compare("ident") == 0){
			i = position(tx);
			if (i == 0){
				error(1);//未声明标识符
			}
			else{
				string s = table[i].kind;
				if (s.compare("constant") == 0){
					string s1 = table[i].type;
					if (s1.compare("int") == 0){
						type = 2;
						gen("lit", 0, constTable[table[i].index].integer);
					}
					else if (s1.compare("real") == 0){
						type = 3;
						gen("lit", 0, constTable[table[i].index].real);
					}
					else{
						type = 1;
						gen("lit", 0, (int)constTable[table[i].index].c);
					}
					getsym();
				}
				else if (s.compare("variable") == 0){
					if (table[i].type.compare("int") == 0){
						type = 2;
					}
					else if (table[i].type.compare("real") == 0){
						type = 3;
					}
					else{
						type = 1;
					}
					if (table[i].arrayFlag){
						for (int x = 0; x < 10; x++){
							id[x] = table[i].name[x];
						}
						i = arrayPosition(tx);
						getsym();
						if (sym.compare("lmparen") != 0){
							error(2);//标识符为数组
						}
						else{
							getsym();
							vector<string> f = { "rmparen" };
							stringArrayCat(f, fsys);
							expression(lev, tx, f);
							if (sym.compare("rmparen") != 0){
								error(3);//缺少右中括号
							}
							else{
								getsym();
							}
							gen("template", 0, table[i].arrayFlag);
							gen("lodArray", lev - table[i].level, table[i].address);//载入数组????????????????????????
						}
					}
					else{
						if (table[i].adrFlag){
							gen("lodA", lev - table[i].level, table[i].address);
						}
						else
							gen("lod", lev - table[i].level, table[i].address);
						getsym();
					}
				}
				else if (s.compare("function") == 0){
					if (table[i].type.compare("int") == 0){
						type = 2;
					}
					else if (table[i].type.compare("real") == 0){
						type = 3;
					}
					else{
						type = 1;
					}
					getsym();
					if (sym.compare("lparen") != 0){
						error(4);//标识符为函数名
					}
					else{
						getsym();
						int numofArgu = 0;
						if (sym.compare("rparen") != 0){
							vector<string> f1 = { "rparen" };
							stringArrayCat(f1, fsys);
							numofArgu = argument(1, i, lev, tx, f1);//实参处理函数
							if (sym.compare("rparen") != 0){
								error(5);//缺少右括号
							}
							else{
								getsym();
							}
							if (funcTable[table[i].index - 1].fparaNum != numofArgu){
								error(46);
							}
							gen("cal", lev - table[i].level, table[i].address);//????????????????
							//cout << "                     This is a function call statement!" << endl;
						}
						else{
							if (funcTable[table[i].index - 1].fparaNum != numofArgu){
								error(46);
							}
							gen("cal", lev - table[i].level, table[i].address);//??????????????????????????
							getsym();
						}
					}
				}
				else{
					error(6);//该标识符不能作为因子
				}
			}
		}
		else if (sym.compare("lparen") == 0){
			getsym();
			vector<string> f = { "rparen" };
			stringArrayCat(f, fsys);
			type = expression(lev, tx, f);
			if (sym.compare("rparen") != 0){
				error(5);//同上,缺少右括号
			}
			else{
				getsym();
			}
		}
		else if (sym.compare("int") == 0 || sym.compare("real") == 0){
			if (num > amax){
				error(7);//数值过大
				num = 0;
			}
			if (sym.compare("int") == 0){
				type = 2;
			}
			else{
				type = 3;
			}
			gen("lit", 0, num);
			getsym();
		}
		vector<string> f1 = fsys;
		test(f1, f2, 9);//一个因子处理完毕，遇到的token应在fsys集合中
	}
	return type;
}

int term(int lev, int tx, vector<string> fsys){
	int type = 2;
	vector<string> f = { "times", "slash" };
	stringArrayCat(f, fsys);
	type = factor(lev, tx, f);
	while (sym.compare("times") == 0 || sym.compare("slash") == 0){
		string mulop = sym;
		getsym();
		int i = factor(lev, tx, f);
		type = i > type ? i : type;
		if (mulop.compare("times") == 0){
			gen("opr", 0, 4);//乘法指令
		}
		else{
			if (type == 2){
				gen("opr", 0, 6);//整数除法
			}
			else{
				gen("opr", 0, 5);//实数除法
			}
		}
	}
	return type;
}

int expression(int lev, int tx, vector<string> fsys){
	int type = 1;
	string var, addop;
	vector<string> f = { "plus", "minus" };
	if (symIn(f)){
		addop = sym;
		getsym();
		stringArrayCat(f, fsys);
		type = term(lev, tx, f);
		if (addop.compare("minus") == 0){
			gen("opr", 0, 1);//取反运算
		}
	}
	else{
		stringArrayCat(f, fsys);
		type = term(lev, tx, f);
	}
	while (sym.compare("plus") == 0 || sym.compare("minus") == 0){
		addop = sym;
		getsym();
		int i = term(lev, tx, f);
		type = type > i ? type : i;
		if (addop.compare("plus") == 0){
			gen("opr", 0, 2);//加法运算
		}
		else{
			gen("opr", 0, 3);//减法运算
		}
	}
	return type;
}

void condition(int lev, int tx, vector<string> fsys){
	string relop;
	vector<string> f = { "eql", "neq", "lss", "gtr", "leq", "geq" };
	vector<string> f1 = { "eql", "neq", "lss", "gtr", "leq", "geq" };
	stringArrayCat(f, fsys);
	expression(lev, tx, f);
	if (symIn(f1) == 0){
		error(36);//条件语句错误
	}
	else{
		relop = sym;
		getsym();
		expression(lev, tx, fsys);
		if (relop.compare("eql") == 0){
			gen("opr", 0, 8);//判等运算
		}
		else if (relop.compare("neq") == 0){
			gen("opr", 0, 9);//判不等
		}
		else if (relop.compare("lss") == 0){
			gen("opr", 0, 10);//判小
		}
		else if (relop.compare("geq") == 0){
			gen("opr", 0, 11);//判不小于
		}
		else if (relop.compare("gtr") == 0){
			gen("opr", 0, 12);//判大
		}
		else if (relop.compare("leq") == 0){
			gen("opr", 0, 13);//判不大于
		}
	}
	return;
}

void statement(int lev, int tx, vector<string> fsys){
	int i, cx1, cx2;
	if (sym.compare("ident") == 0){
		string tmpstr = "";
		for (int x = 0; x < 10; x++){
			tmpstr += id[x];
		}
		i = position(tx);
		if (i == 0){
			error(1);//同上，未声明标识符
		}
		else if (table[i].kind.compare("variable") == 0){
			getsym();
			if (sym.compare("becomes") == 0){
				if (table[i].arrayFlag){
					error(21);//标识符为数组
					getsym();
				}
				else{
					getsym();
					int type = expression(lev, tx, fsys);
					//if (table[i].type.compare("int") == 0 && type == 2 || table[i].type.compare("real") == 0 && type == 3 || table[i].type.compare("char") == 0 && type == 1){
					if (table[i].adrFlag){
						gen("stoA", lev - table[i].level, table[i].address);
					}
					else
						gen("sto", lev - table[i].level, table[i].address);//把表达式的值写往指定内存
					//cout << "                     This is a assign statement!(ident := expression)" << endl;
					//}
				}
			}
			else if (sym.compare("lmparen") == 0){
				for (int x = 0; x < 10; x++){
					id[x] = tmpstr.at(x);
				}
				i = arrayPosition(tx);
				if (table[i].arrayFlag == 0){
					error(22);//标识符不是数组
					getsym();
				}
				else{
					getsym();
					vector<string> f = { "rmparen" };
					stringArrayCat(f, fsys);
					expression(lev, tx, f);
					if (sym.compare("rmparen") != 0){
						error(3);//同上，缺少右中括号
					}
					else{
						gen("template", 0, table[i].arrayFlag);//数组下标已保存???????????????????????????
						getsym();
					}
					if (sym.compare("becomes") != 0){
						error(23);//不是赋值号
					}
					else{
						getsym();
					}
					int type = expression(lev, tx, fsys);
					//if (table[i].type.compare("int") == 0 && type == 2 || table[i].type.compare("real") == 0 && type == 3 || table[i].type.compare("char") == 0 && type == 1){
					gen("stoArray", lev - table[i].level, table[i].address);//把表达式的值载入数组???????????????????
					//cout << "                     This is a assign statement!(ident[expression] := expression)" << endl;
					//}
					//else{
					//	error(45);//变量赋值类型不一致
					//}
				}
			}
			else{
				error(24);//赋值语句错误
			}
		}
		else if (table[i].kind.compare("function") == 0){
			if (funcFlag == 0 || table[i].level + 1 != lev){
				error(49);//函数赋值语句位置错误
			}
			else{
				getsym();
				if (sym.compare("becomes") != 0){
					error(23);//同上，不是赋值号
				}
				else{
					getsym();
				}
				int type = expression(lev, tx, fsys);
				string s = funcTable[table[i].index - 1].freturnVal;
				if (s.compare("int") == 0 && type == 2 || s.compare("real") == 0 && type == 3 || s.compare("char") == 0 && type == 1){
					gen("sto", lev - table[i].level - 1, 3);
				}
				else{
					error(44);//函数赋值类型不一致
				}
				//cout << "                     This is a assign statement!(function ident := expression)" << endl;
			}
		}
		else if (table[i].kind.compare("procedure") == 0){
			getsym();
			if (sym.compare("lparen") != 0){
				error(25);//标识符为过程名
			}
			else{
				getsym();
				int numofArgu = 0;
				if (sym.compare("rparen") != 0){
					vector<string> f = { "rparen" };
					stringArrayCat(f, fsys);
					numofArgu = argument(0, i, lev, tx, f);
					if (procTable[table[i].index - 1].pparaNum != numofArgu){
						error(46);//参数个数不一致
					}
					if (sym.compare("rparen") != 0){
						error(5);//同上，缺少右括号
					}
					else{
						getsym();
					}
					gen("cal", lev - table[i].level, table[i].address);
					//cout << "                     This is a procedure call statement!" << endl;

				}
				else{
					if (procTable[table[i].index - 1].pparaNum != numofArgu){
						error(46);//参数个数不一致
					}
					gen("cal", lev - table[i].level, table[i].address);
					getsym();
					//cout << "                     This is a procedure call statement!" << endl;
				}
			}
		}
		else{
			error(25);//语句错误
		}
	}
	else if (sym.compare("ifsym") == 0){
		getsym();
		vector<string> f = { "thensym" };
		stringArrayCat(f, fsys);
		condition(lev, tx, f);
		if (sym.compare("thensym") == 0){
			getsym();
		}
		else{
			error(26);//缺少then
		}
		cx1 = cx;
		gen("jpc", 0, 0);//条件跳转指令，跳转位置暂时写0，分析完语句后再填写
		vector<string> f1 = { "elsesym" };
		stringArrayCat(f1, fsys);
		statement(lev, tx, f1);
		cx2 = cx;
		gen("jmp", 0, cx2 + 1);
		code[cx1].a = cx;//上一行指令（cx1所指的）跳转位置应为当前cx所指的位置
		if (sym.compare("elsesym") == 0){
			getsym();
			cx1 = cx;
			statement(lev, tx, fsys);
			code[cx2].a = cx;
		}
		//cout << "                     This is a if statement!" << endl;
	}
	else if (sym.compare("repeatsym") == 0){
		cx1 = cx;
		getsym();
		vector<string> f = { "untilsym" };
		stringArrayCat(f, fsys);
		statement(lev, tx, f);
		if (sym.compare("untilsym") != 0){
			error(27);//没有until
		}
		else{
			getsym();
			condition(lev, tx, fsys);
			gen("jpc", 0, cx1);
			//cout << "                     This is a repeat statement!" << endl;
		}
	}
	else if (sym.compare("beginsym") == 0){
		getsym();
		vector<string> f = { "semicolon", "endsym" };
		vector<string> f1 = { "semicolon" };
		stringArrayCat(f, fsys);
		stringArrayCat(f1, statbegsys);
		statement(lev, tx, f);
		while (symIn(f1)){
			if (sym.compare("semicolon") == 0){
				getsym();
			}
			else{
				error(28);//缺少分号
			}
			statement(lev, tx, f);
		}
		if (sym.compare("endsym") == 0){
			getsym();
			//cout << "                     This is a compound statement!" << endl;
		}
		else{
			error(29);//缺少end
		}
	}
	else if (sym.compare("readsym") == 0){
		getsym();
		if (sym.compare("lparen") == 0){
			do{
				getsym();
				if (sym.compare("ident") == 0){
					i = position(tx);
					if (i == 0){
						error(1);//同上，未声明标识符
					}
					else{
						if (table[i].kind.compare("variable") != 0 || (table[i].kind.compare("variable") == 0 && table[i].arrayFlag)){
							error(37);//读入非变量
							i = 0;
						}
						else{
							string type = "";
							if (table[i].type.compare("int") == 0){
								type = "redI";
							}
							else if (table[i].type.compare("real") == 0){
								type = "redR";
							}
							else{
								type = "redC";
							}
							if (table[i].adrFlag){
								gen(type + "A", lev - table[i].level, table[i].address);
							}
							else
								gen(type, lev - table[i].level, table[i].address);//生成sto指令，把读入的值存入指定变量所在的空间
							//cout << "                     This is a read statement!" << endl;
						}
					}
				}
				else{
					error(14);//非标识符
				}
				getsym();
			} while (sym.compare("comma") == 0);
		}
		else{
			error(30);//缺少左括号
		}
		if (sym.compare("rparen") != 0){
			error(5);//同上，缺少右括号
		}
		else
			getsym();
	}
	else if (sym.compare("writesym") == 0){
		getsym();
		if (sym.compare("lparen") != 0){
			error(30);//同上，缺少左括号
		}
		else{
			getsym();
		}
		if (sym.compare("string") == 0){
			for (int i = 0; i < str.length(); i++){
				gen("wrtS", 0, str.at(i));
			}
			getsym();
			if (sym.compare("comma") == 0){
				getsym();
				vector<string> f = { "rparen" };
				stringArrayCat(f, fsys);
				int type = expression(lev, tx, f);
				if (type == 1){
					gen("wrtC", 0, 0);
				}
				else if (type == 2){
					gen("wrtI", 0, 0);
				}
				else{
					gen("wrtR", 0, 0);
				}
				//cout << "                     This is a write statement!" << endl;
				if (sym.compare("rparen") != 0){
					error(5);//同上，缺少右括号
				}
				else
					getsym();
			}
			else if (sym.compare("rparen") != 0){
				error(5);//同上，缺少右括号
			}
			else{
				getsym();
			}
		}
		else{
			vector<string> f = { "rparen" };
			stringArrayCat(f, fsys);
			int type = expression(lev, tx, f);
			if (type == 1){
				gen("wrtC", 0, 0);
			}
			else if (type == 2){
				gen("wrtI", 0, 0);
			}
			else{
				gen("wrtR", 0, 0);
			}
			//cout << "                     This is a write statement!" << endl;
			if (sym.compare("rparen") != 0){
				error(5);//同上，缺少右括号
			}
			else
				getsym();
		}
	}
	else if (sym.compare("forsym") == 0){
		int i;
		getsym();
		if (sym.compare("ident") != 0){
			error(15);//for语句控制条件不是标识符
		}
		else{
			i = position(tx);
			if (i == 0){
				error(1);//同上，未声明标识符
			}
			else{
				if (table[i].kind.compare("variable") == 0 && table[i].arrayFlag == 0){
					getsym();
					if (sym.compare("becomes") != 0){
						error(23);//缺少赋值号
					}
					else{
						getsym();
					}
					vector<string> f = { "tosym", "downtosym" };
					stringArrayCat(f, fsys);
					gen("int", 0, 2);
					expression(lev, tx, f);
					gen("stox", 0, 0);
					/*if (table[i].adrFlag)
					gen("stoA", lev - table[i].level, table[i].address);
					else
					gen("sto", lev - table[i].level, table[i].address);
					cx1 = cx;
					if (table[i].adrFlag)
					gen("lodA", lev - table[i].level, table[i].address);
					else
					gen("lod", lev - table[i].level, table[i].address);*/
					//cout << "                     This is a for statement!" << endl;
					int buchang = 1;
					if (sym.compare("tosym") == 0 || sym.compare("downtosym") == 0){
						if (sym.compare("downtosym") == 0)
							buchang = -1;
						getsym();
						vector<string> f = { "dosym" };
						stringArrayCat(f, fsys);
						expression(lev, tx, f);
						gen("stox", 0, 0);
						if (buchang > 0)
							gen("opr", 0, 13);
						else
							gen("opr", 0, 11);
						cx2 = cx;
						gen("jpc", 1, 0);
						gen("lodx", 0, 0);
						if (table[i].adrFlag)
							gen("stoA", lev - table[i].level, table[i].address);
						else
							gen("sto", lev - table[i].level, table[i].address);
						cx1 = cx;
						if (table[i].adrFlag)
							gen("lodA", lev - table[i].level, table[i].address);
						else
							gen("lod", lev - table[i].level, table[i].address);
						gen("lodx", 0, 0);
						if (buchang > 0)
							gen("opr", 0, 13);
						else
							gen("opr", 0, 11);
						int cx3 = cx;
						gen("jpc", 1, 0);
						if (sym.compare("dosym") != 0){
							error(35);//缺少do
						}
						else{
							getsym();
							statement(lev, tx, fsys);
							if (table[i].adrFlag)
								gen("lodA", lev - table[i].level, table[i].address);
							else
								gen("lod", lev - table[i].level, table[i].address);
							gen("lit", 0, buchang);
							gen("opr", 0, 2);
							if (table[i].adrFlag){
								gen("stoA", lev - table[i].level, table[i].address);
							}
							else
								gen("sto", lev - table[i].level, table[i].address);
							gen("jmp", 0, cx1);
							code[cx2].a = cx;
							code[cx3].a = cx;
						}
					}
					else{
						error(31);//for语句格式错误
					}
				}
				else{
					error(15);//同上，变量标识符错误
				}
			}
		}
	}
	vector<string> f1 = fsys;
	test(f1, f2, 19);//至此一个语句处理完成，一定会遇到fsys中的符号
	return;
}

void listcode(int cx0, int cx){
	cout << "\n目标代码:" << endl;
	int i;
	for (i = cx0; i <= cx - 1; i++){
		cout << setw(4) << i << ": " << setw(10) << code[i].f << setw(3) << code[i].l << setw(5) << code[i].a << endl;
	}
	return;
}

void parameter(int &numofparameter, int flag, int lev, int &tx, int &dx, vector<string> fsys){
	int varFlag = 0;
	char temp[10][11];
	int numofIdent = 0;
	if (sym.compare("varsym") == 0){
		varFlag = 1;
		getsym();
	}
	if (sym.compare("ident") != 0){
		error(15);//同上，标识符错误
		vector<string> f = { "rparen" };
		vector<string> f1 = fsys;
		test(f1, f, 0);
	}
	else{
		for (int x = 0; x < 10; x++){
			temp[numofIdent][x] = id[x];
		}
		temp[numofIdent++][10] = '\0';
		getsym();
		while (sym.compare("comma") == 0){
			getsym();
			if (sym.compare("ident") == 0){
				for (int x = 0; x < 10; x++){
					temp[numofIdent][x] = id[x];
				}
				temp[numofIdent++][10] = '\0';
				getsym();
			}
			else{
				error(15);//同上，标识符错误
			}
		}
		if (sym.compare("colon") == 0){
			getsym();
		}
		else{
			error(17);//同上，缺少冒号
		}
		string f = "";
		if (sym.compare("intsym") == 0 || sym.compare("realsym") == 0 || sym.compare("charsym") == 0){
			if (sym.compare("intsym") == 0){
				f = "int";
			}
			else if (sym.compare("realsym") == 0){
				f = "real";
			}
			else if (sym.compare("charsym") == 0){
				f = "char";
			}
			for (int x = 0; x < numofIdent; x++){
				for (int y = 0; y < 11; y++){
					id[y] = temp[x][y];
				}
				enter(f, "variable", 0, varFlag, lev, 1, dx, tx);
				if (flag){
					procTable[numofProc - 1].ppara[numofparameter] = table[tx];
				}
				else{
					funcTable[numofFunc - 1].fpara[numofparameter] = table[tx];
				}
				numofparameter++;
			}
			getsym();
			if (sym.compare("semicolon") == 0){
				getsym();
				parameter(numofparameter, flag, lev, tx, dx, fsys);
			}
			//cout << "                     This is a parameter declaration!" << endl;
		}
		else{
			error(16);//同上，类型不正确
		}
	}
	return;
}

void block(int numofEle, int dx, int lev, int tx, vector<string> fsys){
	int tx0, cx0;
	tx0 = tx - numofEle;
	table[tx0].address = cx;//?????????????????????????????????????
	gen("jmp", 0, 0);//???????????????????????????????????
	if (lev > levmax){
		error(32);//嵌套过深
	}
	if (sym.compare("constsym") == 0){
		getsym();
		constDeclaration(lev, tx, dx);
		while (sym.compare("comma") == 0){
			getsym();
			constDeclaration(lev, tx, dx);
		}
		if (sym.compare("semicolon") == 0){
			getsym();
		}
		else{
			error(28);//同上，缺少分号
		}
		vector<string> f = { "varsym", "procsym", "funcsym", "writesym", "readsym" };
		stringArrayCat(f, statbegsys);
		test(f, f2, 40);
	}
	if (sym.compare("varsym") == 0){
		getsym();
		varDeclaration(lev, tx, dx);
		string formerSym = sym;
		while (sym.compare("semicolon") == 0){
			getsym();
			if (sym.compare("ident") == 0){
				varDeclaration(lev, tx, dx);
				formerSym = sym;
			}
		}
		if (formerSym.compare("semicolon") != 0){
			error(28);//同上，缺少分号
		}
		vector<string> f = { "procsym", "funcsym", "writesym", "readsym" };
		stringArrayCat(f, statbegsys);
		test(f, f2, 41);
	}
	vector<string> f4 = { "procsym", "funcsym", "writesym", "readsym" };
	stringArrayCat(f4, statbegsys);
	test(f4, f2, 0);
	while (sym.compare("procsym") == 0 || sym.compare("funcsym") == 0){
		if (sym.compare("procsym") == 0){
			getsym();
			if (sym.compare("ident") == 0){
				int tmpi = position(tx);
				if (tmpi != 0 && table[tmpi].level == lev){
					error(34);//同上，标识符已声明
				}
				else{
					int tmpdx = 3, numofpara = 0;
					numofProc++;
					enter("", "procedure", 0, 0, lev, numofProc, dx, tx);
					getsym();
					if (sym.compare("lparen") != 0){
						error(30);//同上，缺少左括号
					}
					else{
						getsym();
					}
					if (sym.compare("rparen") != 0){
						parameter(numofpara, 1, lev + 1, tx, tmpdx, fsys);
					}
					if (sym.compare("rparen") != 0){
						error(5);//同上，缺少右括号
					}
					else{
						getsym();
					}
					if (sym.compare("semicolon") != 0){
						error(28);//同上，缺少分号
					}
					else{
						getsym();
					}
					enterProc(numofpara);
					//cout << "                     This is a procedure declaration!" << endl;
					vector<string> f1 = fsys;
					test(f1, f2, 8);
					vector<string> f = { "semicolon" };
					stringArrayCat(f, fsys);
					int tempFuncFlag = 0;
					if (funcFlag){
						funcFlag = 0;
						tempFuncFlag = 1;
					}
					block(numofpara, tmpdx, lev + 1, tx, f);
					funcFlag = tempFuncFlag;
					tx -= numofpara;
					if (sym.compare("semicolon") == 0){
						getsym();
					}
					else{
						error(28);//同上，缺少分号
					}
				}
			}
			else{
				error(15);//同上，标识符错误
			}
			vector<string> f = { "procsym", "funcsym", "writesym", "readsym" };
			stringArrayCat(f, statbegsys);
			test(f, f2, 42);
		}
		else{
			getsym();
			if (sym.compare("ident") == 0){
				int tmpi = position(tx);
				if (tmpi != 0 && table[tmpi].level == lev){
					error(34);//同上，标识符已声明
				}
				else{
					int tmpdx = 4, numofpara = 0;
					numofFunc++;
					enter("", "function", 0, 0, lev, numofFunc, dx, tx);
					getsym();
					if (sym.compare("lparen") != 0){
						error(30);//同上，缺少左括号
					}
					else{
						getsym();
					}
					if (sym.compare("rparen") != 0){
						parameter(numofpara, 0, lev + 1, tx, tmpdx, fsys);
					}
					if (sym.compare("rparen") != 0){
						error(5);//同上，缺少右括号
					}
					else{
						getsym();
					}
					if (sym.compare("colon") != 0){
						error(17);//同上，缺少冒号
					}
					else{
						getsym();
						if (sym.compare("intsym") != 0 && sym.compare("realsym") != 0 && sym.compare("charsym") != 0){
							error(16);//同上，类型不正确
						}
						else{
							if (sym.compare("intsym") == 0){
								table[tx - numofpara].type = "int";
							}
							else if (sym.compare("realsym") == 0){
								table[tx - numofpara].type = "real";
							}
							else{
								table[tx - numofpara].type = "char";
							}
							getsym();
							if (sym.compare("semicolon") != 0){
								error(28);//同上，缺少分号
							}
							else{
								getsym();
							}
							enterFunc(table[tx - numofpara].type, numofpara);
							vector<string> f1 = fsys;
							test(f1, f2, 8);
							//cout << "                     This is a function declaration!" << endl;
							vector<string> f = { "semicolon" };
							stringArrayCat(f, fsys);
							int tempFuncFlag = 0;
							if (funcFlag)
								tempFuncFlag = 1;
							funcFlag = 1;
							block(numofpara, tmpdx, lev + 1, tx, f);
							tx -= numofpara;
							funcFlag = tempFuncFlag;
							if (sym.compare("semicolon") == 0){
								getsym();
							}
							else{
								error(28);//同上，缺少分号
							}
						}
					}
				}
			}
			else{
				error(15);//同上，标识符错误
			}
			vector<string> f = { "procsym", "funcsym", "writesym", "readsym" };
			stringArrayCat(f, statbegsys);
			test(f, f2, 43);
		}
	}
	vector<string> f3 = { "ident", "writesym", "readsym" };
	stringArrayCat(f3, statbegsys);
	test(f3, f2, 0);
	code[table[tx0].address].a = cx;//把前面生成的跳转语句的跳转位置改成当前位置
	table[tx0].address = cx;//符号表中记录地址为当前代码分配地址
	cx0 = cx;//记下当前代码分配位置
	gen("int", 0, dx);//生成分配空间指令，分配dx个空间
	if (sym.compare("beginsym") != 0){
		error(38);//缺少begin
		sym = "beginsym";
	}
	vector<string> f = { "semicolon" };
	stringArrayCat(f, fsys);
	statement(lev, tx, f);
	if (funcFlag)
		gen("opr", 0, 7);
	else
		gen("opr", 0, 0);//生成从子程序返回操作指令
	vector<string> f1 = { "procsym", "funcsym", "writesym", "readsym", "period", "ident", "semicolon" };
	stringArrayCat(f1, statbegsys);
	test(f1, f2, 8);//用fsys检查当前状态?????????????????
	//listcode(cx0, cx);
	if (funcFlag)
		gen("lod", 0, 3);
	return;
}

double s[1000] = { 0 };

int base(int l, int b){
	if (l < 0){
		return -l;
	}
	int b1;
	b1 = b;
	while (l > 0){
		b1 = s[b1];
		l--;
	}
	return b1;
}

void interpret(){
	int p = 0, b = 1, t = 0;
	struct instruction i;
	s[1] = 0;
	s[2] = 0;
	s[3] = 0;
	do{
		i = code[p];
		p++;
		if (i.f.compare("lit") == 0){
			t++;
			s[t] = i.a;
		}
		else if (i.f.compare("opr") == 0){
			switch ((int)i.a){
			case 0:{
				t = b - 1;
				p = s[t + 3];
				b = s[t + 2];
				break;
			}
			case 1:{
				s[t] = -s[t];
				break;
			}
			case 2:{
				t--;
				s[t] = s[t] + s[t + 1];
				break;
			}
			case 3:{
				t--;
				s[t] = s[t] - s[t + 1];
				break;
			}
			case 4:{
				t--;
				s[t] *= s[t + 1];//整数乘法和实数乘法
				break;
			}
			case 5:{
				t--;
				s[t] /= s[t + 1];//实数除法
				break;
			}
			case 6:{
				t--;
				s[t] = (int)((int)s[t] / (int)s[t + 1]);
				break;
			}
			case 7:{
				t = b - 1;
				p = s[t + 3];
				b = s[t + 2];
				s[++t] = s[t + 3];
				break;
			}
			case 8:{
				t--;
				s[t] = ((int)(s[t] + typeInt) == (int)(s[t + 1] + typeInt)) ? 1 : 0;
				break;
			}
			case 9:{
				t--;
				s[t] = ((int)(s[t] + typeInt) != (int)(s[t + 1] + typeInt)) ? 1 : 0;
				break;
			}
			case 10:{
				t--;
				s[t] = (s[t] < s[t + 1]) ? 1 : 0;
				break;
			}
			case 11:{
				t--;
				s[t] = ((int)(s[t] + typeInt) >= (int)(s[t + 1] + typeInt)) ? 1 : 0;
				break;
			}
			case 12:{
				t--;
				s[t] = (s[t] > s[t + 1]) ? 1 : 0;
				break;
			}
			case 13:{
				t--;
				s[t] = ((int)(s[t] + typeInt) <= (int)(s[t + 1] + typeInt)) ? 1 : 0;
				break;
			}
			default:break;
			}
		}
		else if (i.f.compare("lod") == 0){
			t++;
			s[t] = s[base((int)i.l, b) + (int)i.a];
		}
		else if (i.f.compare("sto") == 0){
			s[base((int)i.l, b) + (int)i.a] = s[t];
			t--;
		}
		else if (i.f.compare("cal") == 0){
			s[t + 1] = base(i.l, b);
			s[t + 2] = b;
			s[t + 3] = p;
			b = t + 1;
			p = (int)i.a;
		}
		else if (i.f.compare("int") == 0){
			t += (int)i.a;
		}
		else if (i.f.compare("jmp") == 0){
			p = (int)i.a;
		}
		else if (i.f.compare("jpc") == 0){
			if (s[t] == 0){
				p = (int)i.a;
				if ((int)i.l){
					t -= 2;
				}
			}
			t--;
		}
		else if (i.f.compare("redI") == 0){
			//cout << "输入int：" << endl;
			long int x;
			cin >> x;
			//scanf("%d", &x);
			s[base(i.l, b) + (int)i.a] = x;
		}
		else if (i.f.compare("redR") == 0){
			//cout << "输入real：" << endl;
			double x;
			cin >> x;
			//scanf("%f", &x);
			s[base(i.l, b) + (int)i.a] = x;
		}
		else if (i.f.compare("redC") == 0){
			//cout << "输入char：" << endl;
			char x;
			cin >> x;
			//scanf("%f", &x);
			s[base(i.l, b) + (int)i.a] = x;
		}
		else if (i.f.compare("redIA") == 0){
			//cout << "输入int：" << endl;
			long int x;
			cin >> x;
			//scanf("%f", &x);
			s[(int)s[base(i.l, b) + (int)i.a]] = x;
		}
		else if (i.f.compare("redRA") == 0){
			//cout << "输入real：" << endl;
			double x;
			cin >> x;
			//scanf("%f", &x);
			s[(int)s[base(i.l, b) + (int)i.a]] = x;
		}
		else if (i.f.compare("redCA") == 0){
			//cout << "输入char：" << endl;
			char x;
			cin >> x;
			//scanf("%f", &x);
			s[(int)s[base(i.l, b) + (int)i.a]] = x;
		}
		else if (i.f.compare("wrtI") == 0){
			cout << (long int)s[t];
			t--;
		}
		else if (i.f.compare("wrtR") == 0){
			cout << s[t];
			t--;
		}
		else if (i.f.compare("wrtC") == 0){
			char c = (int)s[t];
			cout << c;
			t--;
		}
		else if (i.f.compare("wrtS") == 0){
			cout << char((int)i.a);
		}
		else if (i.f.compare("template") == 0){
			if (s[t] >= (int)i.a || s[t] < 0){
				cout << "RuntimeError:数组下标越界!" << endl;
				return;
			}
		}
		else if (i.f.compare("stoArray") == 0){
			s[base(i.l, b) + (int)i.a + (int)s[t - 1]] = s[t];
			t -= 2;
		}
		else if (i.f.compare("lodArray") == 0){
			s[t] = s[base(i.l, b) + (int)i.a + (int)s[t]];
		}
		else if (i.f.compare("stoPara") == 0){
			s[t + (int)i.a] = s[t--];
		}
		else if (i.f.compare("lodAddr") == 0){
			t++;
			s[t] = base(i.l, b) + (int)i.a;
		}
		else if (i.f.compare("lodArrA") == 0){
			s[t] = base(i.l, b) + (int)i.a + s[t];
		}
		else if (i.f.compare("stoAddr") == 0){
			s[t + (int)i.a] = s[t];
			t--;
		}
		else if (i.f.compare("stoA") == 0){
			s[(int)s[base(i.l, b) + (int)i.a]] = s[t--];
		}
		else if (i.f.compare("lodA") == 0){
			s[++t] = s[(int)s[base(i.l, b) + (int)i.a]];
		}
		else if (i.f == "stox"){
			s[t - 2] = s[t];
		}
		else if (i.f == "lodx"){
			t++;
			s[t] = s[t - 2];
		}
	} while (p != 0);
	return;
}

int main(){
	char file[1000];
	cout << "文件路径：";
	scanf("%s", file);
	if ((fin = fopen(file, "r")) == NULL){
		cout << "文件路径?" << endl;
		return 0;
	};
	vector<string> fsys = { "period" };
	stringArrayCat(fsys, declbegsys);
	stringArrayCat(fsys, statbegsys);
	getsym();
	block(0, 3, 0, 0, fsys);
	if (sym.compare("period") != 0 && formerCh != '.'){
		error(39); //program incomplete
	}
	fclose(fin);
	//listcode(0, cx);
	if (err == 0){
		cout << "编译通过。" << endl;
		interpret();
	}
	else{
		cout << "?" << endl;
	}
	return 0;
}
/*Written by Suvrayan Bandyopadhyay
	Roll Number 2301CS89
*/

#include<iostream>
#include<vector>
#include<map>
#include<string>
#include<fstream>
using namespace std;

//We define some global variables here which our assembler will use

map<string, pair<string, int>> opTable;//Our lookup table for operations
vector<string> errorTable;//Our lookup table of errors
vector<string>errorList; //List of all the errors encountered so far

map<string, int>symbolTable;//Our symbol table
vector<vector<string>>tokens;//Tokens from our assembly code
vector<string> mcode;//Machine code
vector<pair<string,bool>>lcode;//listing file code. The bool flag is used to increment pctr



//Initializing our operation table
void initOpTable()
{
	//We have defined our opTable as follows, the command, the opcode (last 2 digits is hex) and the type
	//Type 0 - No offset/value needed
	//Type 1 - Offset/value is required
	//Type 2 - Special instructions (data and SET)

	opTable = { {"data",{"",2}},{"ldc",{"00",1}},{"adc",{"01",1}},{"ldl",{"02",1}},{"stl",{"03",1}},{"ldnl",{"04",1}},
				{"stnl",{"05",1}},{"add",{"06",0}},{"sub",{"07",0}},{"shl",{"08",0}},{"shr",{"09",0}},{"adj",{"0A",1}},
				{"a2sp",{"0B",0}},{"sp2a",{"0C",0}},{"call",{"0D",1}},{"return",{"0E",0}},{"brz",{"0F",1}},{"brlz",{"10",1}},
				{"br",{"11",1}},{"HALT",{"12",0}},{"SET",{"",2}} };


}
//Get file name without extention 
string getFileName(string f)
{
	string ret;
	for (int i = 0; i < f.size(); i++)
	{
		if (f[i] == '.')
		{
			break;
		}
		ret += f[i];

	}
	return ret;
}

//Stores a vector of errors
void initErrorList() 
{
	errorTable.push_back("1: Duplicate label definition");
	errorTable.push_back("2: No such label");
	errorTable.push_back("3: Not a number");
	errorTable.push_back("4: Missing operand");
	errorTable.push_back("5: Unexpected Operand");
	errorTable.push_back("6: Extra text in end of line");
	errorTable.push_back("7: Bogus label name");
	errorTable.push_back("8: Bogus mnemonic");
	errorTable.push_back("9: SET used without label");
	errorTable.push_back("10: SET can't find label in symbol table");
}

//Check if string is a decimal number or not
bool isDec(string n) 
{
	if (n.substr(0, 1) == "0")
	{
		return false;//It is octal
	}
	for (int i = 0; i < n.size(); i++) 
	{
		if (i == 0 && (n[i] == '+' || n[i] == '-')) 
		{
			continue;
		}

		if (!isdigit(n[i])) 
		{
			
			return false;
		}
	}
	return true;
}

//Check if a string is hex or not
bool isHex(string h) 
{
	//Check if first part is hex or not
	if (h.substr(0, 2) != "0x") 
	{
		return false;
	}

	//Check the remaining part
	for (int i = 3; i < h.size(); i++)
	{
		

		if (!(('A'<=h[i]&&h[i]<='F')|| ('a' <= h[i] && h[i] <= 'f')|| ('0' <= h[i] && h[i] <= '9')))
		{
			
			return false;
		}
	}
	return true;
}

//Check if the number is octal or not
bool isOct(string o)
{
	//Check if first part is oct or not
	if (o.substr(0, 1) != "0")
	{
		return false;
	}
	//Check the remaining part
	for (int i = 1; i < o.size(); i++)
	{
		if (i == 2 && (o[i] == '+' || o[i] == '-'))
		{
			continue;
		}
		if (!('0' <= o[i] && o[i] <= '7'))
		{
			return false;
		}
	}
	return true;
}

//Check if the string is a number or not
bool isNum(string n) 
{
	return isDec(n) + isHex(n) + isOct(n);
}

//Convert to int
int toInt(string a)
{
	int ret = 0;
	if (isHex(a))
	{
		int mult = 1;
		for (int i = a.size() - 1; i > 1; i--)
		{
			ret += mult * (int)(a[i] - '0');
			mult *= 16;
		}
	}

	else if (isDec(a))
	{
		ret = stoi(a);
	}
	else if (isOct(a))
	{
		int mult = 1;
		for (int i = a.size() - 1; i > 0; i--)
		{
			ret += mult * (int)(a[i] - '0');
			mult *= 8;
		}
	}
	else 
	{
		cout << "INVALID NUMBER GIVEN" << endl;
	}
	return ret;
}

//Function to split a string into its constituent tokens
vector<string>tokenize(string line) 
{
	vector<string>tokens;//Vector of tokens which can be easily analyzed
	string curr="";
	for (int i = 0; i < line.size(); i++) 
	{
		if (line[i] == ' '||line[i]==','||line[i]=='\t')//If space or , encountered;
		{
			if(curr!="")
				tokens.push_back(curr);
			curr = "";
		}

		else if (line[i] == ':') //Label encountered
		{
			curr += line[i];
			tokens.push_back(curr);
			curr = "";
		}
		else if (line[i] == ';')//Comments
		{
			break;//Line has becom a comment. Everything after # is a comment
		}
		else 
		{
			curr += line[i];
		}
		
	}
	//Add the last bit of text if any
	if (curr != "")
		tokens.push_back(curr);

	//Return the tokens
	return tokens;
}
//Is Mnemonic
bool isMnemonic(string m) 
{
	if (opTable.find(m.substr(0, m.size() - 1)) == opTable.end())
	{
		return false;
	}
	return true;
}



//Check if label name is valid
bool validLabelName(string l) 
{
	//Check if first character is a char
	if (!isalpha(l[0])) 
	{

		return false;
	}

	//Check if name is not a reserved command
	if (isMnemonic(l)) 
	{

		return false;
	}
	return true;
}






//The first pass. We generate the symbol table for our program and identify errors
void analyze(string fname) 
{
	//Open the file
	fstream file(fname);
	//program counter
	int pctr = 0;
	//Line counter
	int lctr = 0;

	//If file does not exist
	if (!file) 
	{
		cout << "ERROR:: FILE DOES NOT EXIST" << endl;
		return;
	}

	//While file is still open
	while (!file.eof())
	{
		lctr++;

		string line;//The current line
		getline(file, line);
	

		//Tokenize the line
		vector<string>t = tokenize(line);

		//If empty line
		if (t.size() == 0)
		{
			continue;
		}

		//Current token
		int i = 0;
		//Check if label on current line
		string label = "";

		//Analyzing first token
		if (t[0][t[0].size() - 1] == ':')
		{
			//Label has been found
			string nlabel = t[0].substr(0,t[0].size() - 1);
			//check if it is valid
			if (!validLabelName(t[0]))
			{
				errorList.push_back("LINE: " + to_string(lctr) + " ERROR::" + errorTable[6]);//Bogus label name
			}
			else if (symbolTable.find(nlabel) != symbolTable.end()&& symbolTable[nlabel]!=-1)
			{
				//Already defined
				
				errorList.push_back("LINE: " + to_string(lctr) + " ERROR::" + errorTable[0]);//Duplicate label defintion
			}

			else
			{
				symbolTable[nlabel] = pctr;//Set address of label to program counter
				label = nlabel;
			}

			//Since the first token has been analzyed
			t.erase(t.begin());
			
		}
		
		//Now that the label has been dealt with if it is present, we can check the mnemonics
		
		if (i == t.size()) //Only label present in this line
		{
			lcode.push_back(pair<string, bool>(line, false));
			continue;
		}
		
		//If mnemonic is present
		else 
		{
			string mnemonic = t[i];

			//Bogus mnemonic
			if (opTable.find(mnemonic) == opTable.end()) 
			{
				
				errorList.push_back("LINE: " + to_string(lctr) + " ERROR::" + errorTable[7]);
				continue;
			}

			//Else we analyze the mnemonic furthur
			int type = opTable[mnemonic].second;

			if (type == 0) //No operand needed
			{
				//Extra text
				if (i != t.size() - 1)
				{
					errorList.push_back("LINE: " + to_string(lctr) + " ERROR::" + errorTable[4]);
				}
				
			}

			//One operand needed
			else if (type == 1) 
			{
				
				//No operand found
				if (i == t.size() - 1) 
				{
					errorList.push_back("LINE: " + to_string(lctr) + " ERROR::" + errorTable[3]);
					continue;
				}
				//Checking the opearand
				string operand = t[i + 1];
				//Extra operand
				if (t.size()-i>2)
				{
					errorList.push_back("LINE: " + to_string(lctr) + " ERROR::" + errorTable[5]);
				
				}

				else if (!isNum(operand)&&!validLabelName(operand))
				{
					
					errorList.push_back("LINE: " + to_string(lctr) + " ERROR::" + errorTable[2]);
				}
				

				if (validLabelName(t[i+1])) 
				{

					//If it does not exist
					if (symbolTable.find(operand) == symbolTable.end()) 
					{
						symbolTable[operand] = -1;//Keep address -1 for now since label might be defined later
					}
				}
				
			}

			//Special set and data operands
			else if (type == 2)
			{
				//For SET instruction
				if (mnemonic == "SET") 
				{

					//If i == 1, it means that we have already encountered a label before in this line
					if (label=="")
					{
						errorList.push_back("LINE: " + to_string(lctr) + " ERROR::" + errorTable[8]);
					}
					else
					{
						//For some reason the label does not exist
						if (symbolTable.find(label) == symbolTable.end())
						{
							
							errorList.push_back("LINE: " + to_string(lctr) + " ERROR::" + errorTable[9]);
						}
						
						//No operand found
						else if (i == t.size() - 1)
						{
							errorList.push_back("LINE: " + to_string(lctr) + " ERROR::" + errorTable[3]);

						}
						//Extra operand
						else if (i > t.size() - 2)
						{
							errorList.push_back("LINE: " + to_string(lctr) + " ERROR::" + errorTable[4]);

						}
						else if (!isNum(t[i + 1])) 
						{
							errorList.push_back("LINE: " + to_string(lctr) + " ERROR::" + errorTable[2]);
						}
						else 
						{
							symbolTable[t[0].substr(0, t[0].size() - 1)] = toInt(t[i + 1]);
						}
						lcode.push_back(pair<string, bool>(line, false));
						continue; //Dont store this in the token list
					}
				}

				//For data
				else if (mnemonic == "data")
				{
					//No operand found
					if (i == t.size() - 1)
					{
						errorList.push_back("LINE: " + to_string(lctr) + " ERROR::" + errorTable[3]);

					}
					//Extra operand
					if (i > t.size() - 2)
					{
						errorList.push_back("LINE: " + to_string(lctr) + " ERROR::" + errorTable[4]);

					}

					//Checking the opearand

					if (!isNum(t[i + 1]) && !validLabelName(t[i + 1]))
					{
						errorList.push_back("LINE: " + to_string(lctr) + " ERROR::" + errorTable[2]);
					}

					if (validLabelName(t[i + 1]))
					{
						//If it does not exist
						if (symbolTable.find(t[i + 1]) == symbolTable.end())
						{
							symbolTable[t[i + 1]] = -1;//Keep address -1 for now since label might be defined later
						}
					}
				}
			}

			lcode.push_back(pair<string, bool>(line, true));
			pctr++;
		}

		tokens.push_back(t);//Create the tokens

	}

	for(auto x:symbolTable)
	{
		if (x.second == -1) 
		{
			errorList.push_back("ERROR:: " + errorTable[1] + ":: " + x.first);
		}
	}



}

//Generate a log file with errors and warnings
void generate_log(string fname) 
{
	string logname = getFileName(fname);
	logname += ".log";

	//Create new log file
	ofstream log(logname);

	for (int i = 0; i < errorList.size(); i++) 
	{
		log << errorList[i].c_str() << endl;
	}

	log.close();
}

//Resize hex string
string resizeHex(string a, int n)
{
	//Appending
	if (n > a.size())
	{

		int diff = n - a.size();
		for (int i = 0; i < diff; i++)
		{
			a = "0" + a;
		}

	}
	//Truncating
	else if (n < a.size())
	{
		int off = a.size() - n;
		a = a.substr(off, a.size() - off);
	}
	return a;
}



//Inverse hex
char inv(char a) 
{
	if (a == '0') 
	{
		return 'F';
	}
	if (a == '1')
	{
		return 'E';
	}
	if (a == '2')
	{
		return 'D';
	}
	if (a == '3')
	{
		return 'C';
	}
	if (a == '4')
	{
		return 'B';
	}
	if (a == '5')
	{
		return 'A';
	}
	if (a == '6')
	{
		return '9';
	}
	if (a == '7')
	{
		return '8';
	}
	if (a == '8')
	{
		return '7';
	}
	if (a == '9')
	{
		return '6';
	}
	if (a == 'A')
	{
		return '5';
	}
	if (a == 'B')
	{
		return '4';
	}
	if (a == 'C')
	{
		return '3';
	}
	if (a == 'D')
	{
		return '2';
	}
	if (a == 'E')
	{
		return '1';
	}
	if (a == 'F')
	{
		return '0';
	}

}

//Increment hex
char inc(char a) 
{
	if (a == '0')
	{
		return '1';
	}
	if (a == '1')
	{
		return '2';
	}
	if (a == '2')
	{
		return '3';
	}
	if (a == '3')
	{
		return '4';
	}
	if (a == '4')
	{
		return '5';
	}
	if (a == '5')
	{
		return '6';
	}
	if (a == '6')
	{
		return '7';
	}
	if (a == '7')
	{
		return '8';
	}
	if (a == '8')
	{
		return '9';
	}
	if (a == '9')
	{
		return 'A';
	}
	if (a == 'A')
	{
		return 'B';
	}
	if (a == 'B')
	{
		return 'C';
	}
	if (a == 'C')
	{
		return 'D';
	}
	if (a == 'D')
	{
		return 'E';
	}
	if (a == 'E')
	{
		return 'F';
	}
	if (a == 'F')
	{
		return '0';
	}
}

//2's complement of a hexadecimal number
string twocomplement(string h) 
{
	
	for (int i = 0; i < h.size(); i++) 
	{
		h[i] = inv(h[i]);
	}
	//Increment
	for (int i = h.size() - 1; i >= 0; i--) 
	{
		if (h[i] == 'F') 
		{
			//If carry
			h[i] = inc(h[i]);
			continue;
		}
		h[i] = inc(h[i]);
		break;

	}

	return h;
}


//Integer to hexstring
string int_to_hex(int a,int n) 
{
	string ret = "";
	
	bool neg = false;
	if (a < 0) 
	{
		a *= -1;
		neg = true;
	}

	while (a > 0) 
	{
		int d = a % 16;
		if (d < 10) 
		{
			ret += to_string(d);
		}
		else 
		{
			ret += (char)('A' + d - 10);
		}
		a /= 16;
	}
	reverse(ret.begin(), ret.end());

	ret = resizeHex(ret, n);
	//If negative
	if (neg) 
	{
		ret = twocomplement(ret);
	}

	
	return ret;
}





//The second pass, here we generate listing file and machine code;
void synthesize() 
{
	for (int i = 0; i < tokens.size(); i++) 
	{
		//Operation part
		string m1 = opTable[tokens[i][0]].first;

		string m2;
		//Operand part
		if (tokens[i].size() >= 2&&tokens[i][0]!="data")
		{
			if (validLabelName(tokens[i][1]))
			{
				
				
				int val = symbolTable[tokens[i][1]];
				//If offset
				if (tokens[i][0] == "call" || tokens[i][0] == "brz" || tokens[i][0] == "brlz" || tokens[i][0] == "br") 
				{
					val = val - i -1;//-1 because the pc increments before the offset is applied
					
				}

				m2 = int_to_hex(val,6);
				
			}
			else
			{
				m2 = int_to_hex(toInt(tokens[i][1]),6);
			}
		}
		else if (tokens[i][0] == "data") 
		{
			m1 = "";//Dont need the opcode here
			if (validLabelName(tokens[i][1]))
			{
				int val = symbolTable[tokens[i][1]];
				m2 = int_to_hex(val, 8);

			}
			else
			{
				m2 = int_to_hex(toInt(tokens[i][1]), 8);
			}
		}
		string m = m2 + m1;
		m = resizeHex(m, 8);
		mcode.push_back(m);
		
	
		
			
			
		
	}
}
//Write to file
void write(string fname) 
{
	
	string name = getFileName(fname);
	string oname = name+ ".o";
	string lname = name + ".L";

	//.o file
	ofstream ofile;
	
	ofile.open(oname, std::ios::binary);

	if (!ofile) 
	{
		cout << "Unable to create file" << endl;
		return;
	}

	//Write hexcode
	for (int i = 0; i < mcode.size(); i++) 
	{
		for (int j = 0; j < mcode[i].size(); j+=2) 
		{
			
			string bytestr = mcode[i].substr(j,2);
			char byte = static_cast<char>(stoi(bytestr, nullptr, 16));//Convert it to a byte string
			ofile.write(&byte, sizeof(byte));
		}
	}
	ofile.close();
	
	//Listing file
	fstream lfile;
	lfile.open(lname, std::ios::out);

	int p = 0;
	for (int i = 0; i < lcode.size(); i++)
	{
		if (lcode[i].second == false) 
		{
			lfile << int_to_hex(p, 8) << "          " << lcode[i].first << endl;
			
		}
		else 
		{
			lfile << int_to_hex(p, 8) << " "<<mcode[p]<<" " << lcode[i].first << endl;
			p++;
		}

	}


}

int main(int argc,char* argv[])
{
	//Initialize global variables
	initOpTable();
	initErrorList();
	
	string fname;
	if (argc == 1) 
	{
		cout << "Enter file name (.asm extention)" << endl;
		cin >> fname;
	}
	else 
	{
		fname = argv[1];
	}

	//Analyze text
	analyze(fname);

	//Generate log file
	generate_log(fname);

	//Print errors once
	for (int i = 0; i < errorList.size(); i++) 
	{
		cout << errorList[i] << endl;
	}
	//If errors
	if (errorList.size() != 0) 
	{
		cout << endl << endl;
		cout << "Errors present in program" << endl;
		cout << "Could not turn into machine code" << endl;
		cout << "Please check the log file for more information on errors" << endl;
		system("PAUSE");
		return 0;
	}

	//If no errors
	synthesize();
	cout << "\n";
	for (int i = 0; i < mcode.size(); i++) 
	{
		cout << mcode[i] << endl;
	}

	//Write to file
	write(fname);



	system("PAUSE");
}
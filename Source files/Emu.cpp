/*Name:Suvrayan Bandyopadhyay
Roll Number 2301CS89
*/

#include<iostream>
#include<fstream>
#include<vector>
#include<string.h>
#include<bitset>
using namespace std;

unsigned char* memory; // A string of the binary data in the



int a, b, pc, sp;
bool program_end = false;
long filesize;

//Read the binary data in our file
void readFile(string fname) 
{
	//Reading the file
	fstream file(fname, std::ios::binary|std::ios::in);

	if (!file) 
	{
		cout << "FILE DOES NOT EXIST" << endl;
		return;
	}

	//Get size of the file
	file.seekg(0, ios::end);
	filesize = file.tellg();

	char* temp = new char[1<<24];//For reading char data initially 1<<24 is the memory size

	file.seekg(0, ios::beg);//Go back to the beginning of the file

	file.read(temp, filesize);//Read binary data

	memory = reinterpret_cast<unsigned char*>(temp);

}
//bin to dec
//1001
int bin_to_dec(unsigned char binary[],int size) 
{
	int sh = 0;

	int32_t result = 0;
	

	for (int i = size - 1; i >= 0; i--)
	{

		result |=	static_cast<int32_t>(binary[i])<<sh;
		sh += 8;
	}
	//Check sign bit
	if (binary[0] & 0x80&&size<4)
	{
		int32_t mask = -((int32_t)1 << (size * 8));//Set all bits above n*8 as 1
	
		result |= mask;
		
	}

	return result;
}

//Store in memory
void store_in_mem(unsigned char *memloc, int val) 
{

	unsigned char* byte = reinterpret_cast<unsigned  char*>(&val);

	int j = 0;
	for (int i = sizeof(val) - 1; i >= 0; i--) 
	{
		memloc[j] = byte[i];//To account for endianess (Our memory is big endian)
	
		j++;
	
	}
}


//Runs one instruction
void run_step()
{
	try
	{
		unsigned char val[3] = { memory[pc * 4], memory[pc * 4 + 1], memory[pc * 4 + 2] };


		//The operation 
		int op = static_cast<int>(memory[pc * 4 + 3]);

		pc++;//Increment pc;


		//The value
		int value = bin_to_dec(val, 3);

		//Ldc
		if (op == 0)
		{
			b = a;
			a = value;
		}

		//adc
		else if (op == 1)
		{
			a = a + value;
		}
		//ldl
		else if (op == 2)
		{
			b = a;
			unsigned char memdata[] = { memory[(sp + value) * 4], memory[(sp + value) * 4 + 1], memory[(sp + value) * 4 + 2],memory[(sp + value) * 4 + 3] };
			a = bin_to_dec(memdata, 4);
		}
		//stl
		else if (op == 3)
		{

			store_in_mem(&memory[(sp + value) * 4], a);
			a = b;
		}

		//ldnl
		else if (op == 4)
		{
			unsigned char memdata[] = { memory[(a + value) * 4], memory[(a + value) * 4 + 1], memory[(a + value) * 4 + 2],memory[(a + value) * 4 + 3] };
			a = bin_to_dec(memdata, 4);
		}

		//stnl
		else if (op == 5)
		{
			//memory[a + value] = b;
			store_in_mem(&memory[(a + value)*4], b);
		}
		//add
		else if (op == 6)
		{

			a = b + a;

		}
		//sub
		else if (op == 7)
		{
			a = b - a;
		}
		//shl
		else if (op == 8)
		{
			a = b << a;
		}
		//shr
		else if (op == 9)
		{
			a = b >> a;
		}
		//adj
		else if (op == 10)
		{
			sp = sp + value;
		}
		//a2sp
		else if (op == 11)
		{
			sp = a;
			a = b;
		}
		//sp2a
		else if (op == 12)
		{
			b = a;
			a = sp;
		}
		//call
		else if (op == 13)
		{
			b = a;
			a = pc;
			pc = pc + value;
		}

		//return 
		else if (op == 14)
		{
			pc = a;
			a = b;
		}
		//brz
		else if (op == 15)
		{
			if (a == 0)
			{
				pc = pc + value;
			}
		}
		//brlz
		else if (op == 16)
		{
			if (a < 0)
			{
				pc = pc + value;
			}
		}
		//br
		else if (op == 17)
		{
			pc = pc + value;
		}
		//halt
		else if (op == 18)
		{
			program_end = true;
		}
		else 
		{
			cout << "Unrecognized operation" << endl;
			program_end = true;
		}


		if (pc >= (filesize / 4))

		{
			program_end = true;
		}
	}
	catch (exception& e) 
	{
		cout << "Exception detected... Program execution halted"<<endl;
		cout << "Exception: " << e.what() << endl;
		program_end = true;
	}

	
}

//Trace
void trace() 
{
	cout << "a: " << a << "\t" << "b: " << b << "\t" << "pc: " << pc << "\t" << "sp " << sp << endl;
}


int main(int argc, char* argv[])
{
	
	//Load file
	string name;
	
	if (argc < 2) 
	{
		cout << "Enter program name (.o extension)" << endl;
		cin >> name;
	}
	else 
	{
		name = argv[1];
	}
	readFile(name);
	if (filesize == 0) 
	{
		return 0;
	}
	cout << "File loaded successfully" << endl;
	//Set variables
	a = 0;
	b = 0;
	pc = 0;
	sp = 0;
	

		cout << "1: Trace" << endl;
		cout << "2: Run step" << endl;
		cout << "3: Before" << endl;
		cout << "4: After" << endl;
		cout << "5: Memory Dump (Before execution)" << endl;
		cout << "6: Memory Dump (After execution)" << endl;
		cout << "7: View Instruction Set" << endl;
		cout << "Enter choice" << endl;
		cout << endl;

		int ch;
		cin >> ch;

		if (ch == 1) 
		{
			pc = 0;//Start the program
			program_end = false;

			//While program is going on
			while (program_end==false) 
			{
				trace();
				run_step();
			}
			trace();
		}

		else if (ch == 2) 
		{
			pc = 0;//Start the program
			program_end = false;

			//While program is going on
			while (program_end == false)
			{
				trace();
				run_step();
				system("PAUSE");
			}
			trace();
	
		}

		else if (ch == 3) 
		{
			trace();
		}

		else if (ch == 4) 
		{
			pc = 0;//Start the program
			program_end = false;

			//While program is going on
			while (program_end == false)
			{
				run_step();
			}
			trace();
		}

		//Memory dump before
		else if (ch == 5)
		{
			cout << "How many bytes should be displayed" << endl;
			int n;
			cin >> n;
			for (int i = 0; i < n; i++)
			{
				if (i % 4 == 0)
				{
					cout << endl;
				}
				cout << hex << (int)memory[i] << " ";
			}

		}

		//Memory dump after
		else if (ch == 6) 
		{
			pc = 0;//Start the program
			program_end = false;

			//While program is going on
			while (program_end == false)
			{
				run_step();
			}
			cout << "How many bytes should be displayed" << endl;
			int n;
			cin >> n;
			for (int i = 0; i < n; i++) 
			{
				if (i % 4 == 0) 
				{
					cout << endl;
				}
				cout <<hex<<(int) memory[i] << " ";
			}

		}

		

		//Show ISA 
		else if (ch == 7) 
		{
			cout << "Instruction Set" << endl;
			cout << "NA : data" << endl;
			cout << "0: ldc" << endl;
			cout << "1: adc" << endl;
			cout << "2: ldl" << endl;
			cout << "3: stl" << endl;
			cout << "4: ldnl" << endl;
			cout << "5: stnl" << endl;
			cout << "6: add" << endl;
			cout << "7: sub" << endl;
			cout << "8: shl" << endl;
			cout << "9: shr" << endl;
			cout << "10: adj" << endl;
			cout << "11: adj" << endl;
			cout << "12: a2sp" << endl;
			cout << "13: call" << endl;
			cout << "14: return" << endl;
			cout << "15: brz" << endl;
			cout << "16: brlz" << endl;
			cout << "17: br" << endl;
			cout << "18: HALT" << endl;
			cout << "NA: SET" << endl;
			cout << endl;
		}
		
		system("PAUSE");
	
}
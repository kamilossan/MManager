#include "stdafx.h"
#include <Windows.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>

#define BUFFER_SIZE (sizeof DWORD)

using namespace std;


	

	ofstream mem_dump;
	int take_proc(LPCSTR name);
	int change_memory(LPVOID address, int new_value);
	int proc_attach();
	int read_process_memory(int search, int value);

	struct registry {
		vector<int> values;
		vector<int> registers;
	};
	int scont = 0;
	registry storage;
	
	//opened process
	HWND attached_process;
	//process handle with modification attributes
	HANDLE pHandler;

	LPVOID reg_start;
	unsigned int mem_reg = 0x0;
	LPVOID reg_end;
	int value;
	string proc_name;
	

	//this process handlers(to set debug mode)
	HANDLE current_proc;
	TOKEN_PRIVILEGES dbg;

	//get input with type mismatch handle
	LPVOID get_hex_input(int max);
	int get_int_input(int max);




	int main()
	{
		cout << "Enter process name:" << endl;
		cin >> proc_name;
		




		
		//adjust process privileges to debug mode (allows memory access and manipulation)
		if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &current_proc)) {
			cout << "Error adjusting process privileges" << endl;
		}
		if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &dbg.Privileges[0].Luid)) {
			cout << "Error setting new privilege" << endl;
		}
		dbg.PrivilegeCount = 1;
		//enable new privileges
		dbg.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
		if (!AdjustTokenPrivileges(current_proc, false, &dbg, sizeof(TOKEN_PRIVILEGES), (PTOKEN_PRIVILEGES)NULL, (PDWORD)0)) {
			cout << "Error enabling debug privileges: " << GetLastError() << endl;
			return false;
		}
		else {

			cout << "Entered debug mode successfully!" << endl;

			Sleep(1000);

			//find process and attach to it
			bool open = take_proc(proc_name.c_str());
			if (open) {
				proc_attach();
			}
			int decision = 0;
			
			while (true) {
				if (!open) {
					break;
				}
				cout << "What do you want to do?" << endl << "1. Read memory" << endl << "2. Edit register" <<endl<<"3. Dump search results to file"<< endl;
				decision = get_int_input(3);
				switch (decision) {
				case 1:
					cout << "Continued search? 1 - Yes 2 - No" << endl;
					scont = get_int_input(2);
					scont--;
					if (scont) {
						cout << endl << "Start register?" << endl;
						reg_start = get_hex_input(0);
						cout << endl << "End register?" << endl;
						reg_end = get_hex_input(0);

					}
					cout << endl << "Desired value(if empty it will scan all)?" << endl;
					value = get_int_input(0);

					read_process_memory(scont, value);
					break;
				case 2:
					LPVOID add;
					int val;
					cout << "Address register:" << endl;
					add =get_hex_input(0);

					cout << "Value to input: " << endl;
					val = get_int_input(0);


					change_memory(add, val);
					break;

				case 3:
					mem_dump.open("memory.dmp", ios::trunc);
					for (int x = 0; x < storage.registers.size();x++) {
						mem_dump << "Register: " << (LPCVOID)storage.registers[x] << " Value: " << storage.values[x]<<endl;
					}
					break;
				default:
					cout << "wrong input";
					break;
				
				}
			}
			return 0;

		}



		Sleep(2000);
		return 1;
	}
	int change_memory(LPVOID address, int new_value) {
		if (WriteProcessMemory(pHandler, address, &new_value, sizeof(new_value), NULL)) {
			cout << "Value at "<<address<<" successfully changed to "<< new_value<<"!" << endl;
			return 0;
		}
		else {
			cout << "Error by changing value: " << GetLastError() << endl<<"Check if the address is valid and accessible!";
			return 1;
		}
	}

	int read_process_memory(int cont, int value = NULL) {
		int temp = 0;
		mem_reg = (int)reg_start;
		//registry storage;
		while (true)



		{	
			if (!(bool)cont) {
				registry research;
				for (unsigned int x = 0; x < storage.registers.size(); x++) {
					if (!ReadProcessMemory(pHandler, (LPCVOID)storage.registers[x], &temp, BUFFER_SIZE, NULL)) {
						cout << (LPCVOID)mem_reg;
						cout << "error " << GetLastError() << endl;
						if (GetLastError() == 299) {
							cout << "register unavailable" << endl;
						}

					}
					//print value
					else {
						if (value == NULL | value == temp) {

							research.values.push_back(temp);
							research.registers.push_back(storage.registers[x]);
							cout << (LPVOID)storage.registers[x] << "  :  " << temp << endl;

						}

					}
				}
				storage = research;
				cout << "remaining values: " << storage.registers.size()<<endl;
				break;
			}
			else{
				if (mem_reg <= (unsigned int)reg_end) {
				//check for error, skip entire memory page if inaccessible
				if (!ReadProcessMemory(pHandler, (LPCVOID)mem_reg, &temp, BUFFER_SIZE, NULL)) {
					cout << (LPCVOID)mem_reg;
					cout << "error " << GetLastError() << endl;
					if (GetLastError() == 299) {
						cout << "register unavailable" << endl;
						mem_reg += 4096;
					}

				}
				//print value
				else {
					if (value == NULL | value == temp) {

						storage.values.push_back(temp);
						storage.registers.push_back(mem_reg);
						cout << (LPVOID)mem_reg << "  :  " << temp << endl;

					}

				}
				mem_reg++;
			}
			else {
				cout << reinterpret_cast<int>(reg_end);
				cout << "Read complete!" << endl;
				cout << "Found " << storage.registers.size() << " results matching " << value << " value." << endl << flush;
				Sleep(5000);
				break;
			}
			}

	
			//}



			//x += 1;
			//temp = 0;


		}
		return 0;
	}

	//more advanced process search, using callback function - used FindWindowA() due to less ambiguity and more reliable syntax

	//BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM null) {
	//	wchar_t window_name[MAX_PATH];
	//	GetWindowText(hwnd, (LPTSTR)&window_name, MAX_PATH);
	//	char title[MAX_PATH];
	//	for (int x = 0; window_name[x] != NULL; x++) {
	//		title[x] = (char)window_name[x];
	//	}
	//
	//	if ((int)strstr(title, proc_name) != NULL) {
	//		cout << "found process: " << title << endl;
	//		attached_process = hwnd;
	//		return false;
	//
	//	}
	//	else return true;
	//
	//};
	//
	//void find_process(string name) {
	//
	//
	//	EnumWindows(EnumWindowsProc, NULL);
	//
	//}

	int take_proc(LPCSTR name) {
		attached_process = FindWindowA(0, name);
		if (attached_process != NULL) {
			cout << "Attached to " << name << " successfully!";
			Sleep(1000);
			return 1;
		}
		else {
			cout << "Error finding process!";
			Sleep(1000);
			return 0;
		}
	}

	int proc_attach() {

		DWORD procID;
		GetWindowThreadProcessId(attached_process, &procID);
		pHandler = OpenProcess(PROCESS_VM_READ | PROCESS_VM_WRITE, false, procID);




		return 0;
	}

	LPVOID get_hex_input(int max) {
		
		LPVOID out;
		while(!(cin>>out)){
			cout << "Wrong input, try again!" << endl;
			cin.clear();
			cin.ignore(MAXINT, '\n');
			
					



		}


		if ((out > 0 && (int)out <= max) |max == 0) {
			return out;
		}
		else {
			cout << "Wrong input, try again!" << endl;
			return get_hex_input(max);
		}
	}

	int get_int_input(int max) {

		int out;
		while (!(cin >> out)) {
			cout << "Wrong input, try again!" << endl;
			cin.clear();
			cin.ignore(MAXINT, '\n');





		}


		if ((out > 0 && out <= max) | max == 0) {
			return out;
		}
		else {
			cout << "Wrong input, try again!" << endl;
			return get_int_input(max);
		}
	}

	
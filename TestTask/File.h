#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <Windows.h>
#include <vector>
#include <sstream>

using namespace std;

class File
{

public:

	string path;
	unsigned long hash;
	bool change;	
	
	File(char path_ch[]);
	~File();	

//private:
	void get_file_path(char path_ch[]);
	void hash_f(unsigned char* buf, unsigned long len);
	void get_file_inf();
	int get_file_size(ifstream& file);
	void write_data(ofstream& file);

};

void crc32_table_gen();

string char_to_string(char ch[]);
void utf8_to_rus(char path[]);
void del_space(char path[], int i);
bool file_eof(ifstream& paths);
void print();
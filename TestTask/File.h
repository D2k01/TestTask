#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include <string>
#include <Windows.h>
#include <vector>
#include <sstream>
#include "GOSTHash.h"
#include "Constants.h"


using namespace std;

class File
{

public:

	char* path; // ���� � �����
	int lOp;	// ������ ����
		
	Change change; // ���������� �� ���������
	Algoritm alg; // �������� �����������
	Hash* hash_file; // ���-����� ��� crc32
					 // ��� ����� ��� �������� �����������, ���� ������������� �������� GOST

	TGOSTHashContext* CTX; // ��������� ��� ���������� ���-����� �� ��������� GOST
							// �������� � ���� ���, ������ ���-����� � �.�.
	
	File(char path_ch[]);
	File(ifstream& data_r);
	~File();	

//private:
	void file_init();
	void set_file_path(char path_ch[]);	
	void get_file_hash();
	int get_file_size(ifstream& file);
	void hash_f(unsigned char* buf, unsigned long len);
	void was_changed();
	void write_data(ofstream& data_w);
	void read_data(ifstream& data_r);
};

void crc32_table_gen();

string char_to_string(char* path);
void utf8_to_rus(char path[]);
void del_space(char path[], int i);
void all_inf(File* f);
void print(vector<File*>& files);

istream& operator>>(std::istream& is, Algoritm& a);
istream& operator>>(std::istream& is, Point& p);
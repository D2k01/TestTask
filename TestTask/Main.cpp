#include <iostream>
#include <fstream>
#include <string>
#include <Windows.h>
#include <vector>
#include <sstream>

using namespace std;

unsigned long crc_table[256];
vector<unsigned long> files_hash;

void crc32_table_gen() {

	unsigned long crc;

	for (int i = 0; i < 256; i++) {
		crc = i;
		for (int j = 0; j < 8; j++) {
			crc = crc & 1 ? (crc >> 1) ^ 0xEDB88320UL : crc >> 1;
		}
		crc_table[i] = crc;
	}
}

void hash_f(unsigned char* buf, unsigned long len) {
	unsigned long crc = 0xFFFFFFFFUL;

	while (len--)
		crc = crc_table[(crc ^ *buf++) & 0xFF] ^ (crc >> 8);

	files_hash.push_back(crc ^ 0xFFFFFFFFUL);
}

void print_vector(vector<unsigned long>& vec) {

	for (int i = 0; i < vec.size(); i++)
		cout << vec[i] << endl;

}

void del_space(char path[], int i) {

	while (path[i] != '\0') {
		path[i] = path[i + 1];
		i++;
	}

}

void utf8_to_rus(char path[]) {

	int ansi_code;
	int i = 0;
	while (path[i] != '\0') {
		ansi_code = path[i];
		if (ansi_code < 0) {
			if (ansi_code < 0 && ansi_code == -47 && path[i + 1] % 2 == 0) {
				path[i] = char((path[i + 1] / 2) + abs(ansi_code) + (64 + path[i + 1] / 2) + 1);
				del_space(path, i + 1);
			}
			else if (ansi_code < 0 && ansi_code == -47 && path[i + 1] % 2 != 0) {
				path[i] = char((path[i + 1] / 2) + abs(ansi_code) + (64 + path[i + 1] / 2));
				del_space(path, i + 1);
			}
			else {
				path[i] = char(path[i + 1] + abs(ansi_code));
				del_space(path, i + 1);
			}
		}		
		i++;
	}
}

string char_to_string(char ch[]) {

	string s;
	std::stringstream ss;
	ss << ch;
	s = ss.str();

	return s;
}



int main() {

	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);

	setlocale(LC_ALL, "Russian");

	ifstream paths("Paths.txt");

	if (paths) {
		cout << "File \"Paths\" is open" << endl;
	}
	else
		cout << "Error" << endl;

	crc32_table_gen();	
	char path_ch[1024];
	int n = 4096 * 64;
	char* buf = new char[n];

	while (paths.getline(path_ch, 1024, '\n')){		
		utf8_to_rus(path_ch);
		ifstream file(path_ch, ios::binary);	

		if (file) {
			cout << "File is open" << endl;			

			file.read(buf, n);			

			hash_f((unsigned char*)buf, file.gcount());			
		}
		else
			cout << "Error" << endl;
		
		file.close();
	}
	cout << "Hashs: " << endl;
	print_vector(files_hash);

	delete[] buf;

	return 0;
}
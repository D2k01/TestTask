#include "File.h"

unsigned long crc_table[256];

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

File::File(char path_ch[]) {

	get_file_path(path_ch);
	get_file_hash();

};

File::File(string& str) {

	read_data(str);
	get_file_hash();
};

File::~File() {};

void File::get_file_path(char path_ch[]) {	
	
	utf8_to_rus(path_ch);
	path = char_to_string(path_ch);	

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

void del_space(char path[], int i) {

	while (path[i] != '\0') {
		path[i] = path[i + 1];
		i++;
	}

}

void File::write_data(ofstream& file) {	

	file << path << "|" << hash << "|" << "\n";
	
}

void File::read_data(string& str) {	
	
	int ptr;
	
	ptr = str.find('|');
	path = str.substr(0, ptr);
	str.erase(0, ptr + 1);
	ptr = str.find('|');
	hash = atoll(str.substr(0, ptr).c_str());
	
}

int File::get_file_size(ifstream& file) {

	file.seekg(0, ios::end);

	return file.tellg();

	file.clear();
	file.seekg(0);
}

void File::get_file_hash() {

	ifstream file(path, ios::binary);

	int n = get_file_size(file);

	if (file) {

		char* buf = new char[n];

		file.read(buf, n);

		hash_f((unsigned char*)buf, n);
	}
	else
		cerr << "File didn't open" << endl;

	file.close();

}

void File::hash_f(unsigned char* buf, unsigned long len) {
	unsigned long crc = 0xFFFFFFFFUL;
	unsigned long tmp = 0;

	while (len--)
		crc = crc_table[(crc ^ *buf++) & 0xFF] ^ (crc >> 8);

	tmp = crc ^ 0xFFFFFFFFUL;

	if (hash) {
		
		hash == tmp ? change = false : change = true;
			
	}
	else {
		change = false;
	}
	
	hash = tmp;
	
}

void print(vector<File*>& files) {

	int ptr_begin;
	int ptr_end;

	for (File* f : files) {

		ptr_begin = f->path.find("\\");
		ptr_end = f->path.rfind("\\");

		cout << f->path.substr(0, ptr_begin + 1) << "....." << f->path.substr(ptr_end, f->path.size())
			<< " | " << f->hash << " | ";

		if (f->change)
			cout << "Was changed" << endl;
		else
			cout << "Wasn't changed" << endl;

	}
	
}

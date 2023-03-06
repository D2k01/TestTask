#include "File.h"

File::File(char path_ch[]) {

	CTX = new TGOSTHashContext;	
	GOSTHashInit(CTX, 512);
	set_file_path(path_ch);	
	get_file_hash();

};

File::File(ifstream& data_r) {

	CTX = new TGOSTHashContext;
	GOSTHashInit(CTX, 512);
	read_data(data_r);
	get_file_hash();
};

File::~File() {

	 delete[] path;

};

void File::set_file_path(char path_ch[]) {		
	
	//path = char_to_string(path_ch);
	path = new char[128];
	utf8_to_rus(path_ch);
	strcpy(path, path_ch);

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

string char_to_string(char* ch) {

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

void File::write_data(ofstream& data_w) {		

	int i;

	lOp = char_to_string(path).size();

	data_w.write((char*)&lOp, sizeof(int));
	data_w.write((char*)&CTX->hash_size, sizeof(int));

	for (i = 0; i < lOp; i++) // Ввести константу для длины path или записывать на основе максимально длины
		data_w.write(&path[i], 1);

	if (CTX->hash_size == 256) {
		for (i = 32; i < 64; i++) {

			data_w.write((char*)&CTX->hash[i], 1);

		}
	}
	else {
		for (i = 0; i < 64; i++) {

			data_w.write((char*)&CTX->hash[i], 1);

		}
	}		
}

void File::read_data(ifstream& data_r) {	
	
	int i;

	data_r.read((char*)&lOp, sizeof(int));
	data_r.read((char*)&CTX->hash_size, sizeof(int));

	path = new char[lOp];	

	path[lOp] = '\0';

	for (i = 0; i < lOp; i++) // Ввести константу для длины path или записывать на основе максимально длины
		data_r.read((char*)&path[i], 1);	

	if (CTX->hash_size == 256) {
		for (i = 32; i < 64; i++) {

			data_r.read((char*)&hash_buf[i], 1);

		}
	}
	else {
		for (i = 0; i < 64; i++) {

			data_r.read((char*)&hash_buf[i], 1);

		}
	}

}

int File::get_file_size(ifstream& file) {	

	return file.tellg();
	
}

void File::get_file_hash() {

	ifstream file(path, ios::binary|ios::ate);	

	int n = file.tellg();	

	file.seekg(0, ios::beg);	

	uint8_t* buf = new uint8_t[2*n];

	buf[n] = '\0';

	if (file) {
		
		file.read((char*)buf, n);
		
		GOSTHashUpdate(CTX, buf, n);

		if(buf != NULL)
			delete[] buf;

		GOSTHashFinal(CTX);

		file.close();

		if (hash_buf != NULL)
			was_changed();		

	}
	else
		cerr << "File didn't open" << endl;
}

void File::was_changed() {

	int i;
	change = false;

	if (CTX->hash_size == 256) {
		for (i = 32; i < 64; i++) {

			if (hash_buf[i] != CTX->hash[i]) {

				change = true;
				break;

			}

		}		
	}
	else {
		for (i = 0; i < 64; i++) {

			if (hash_buf[i] != CTX->hash[i]) {

				change = true;
				break;

			}

		}
	}
}

/*
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
	
}*/

void print(vector<File*>& files) {

	string path_str;
	int ptr_begin;
	int ptr_end;
	int i;

	

	for (File* f : files) {
		
		path_str = char_to_string(f->path);

		ptr_begin = path_str.find("\\");
		ptr_end = path_str.rfind("\\");

		cout << path_str.substr(0, ptr_begin + 1) << "....." << path_str.substr(ptr_end, path_str.size())
			<< " | " ;

		if (f->CTX->hash_size == 256) {
			for (i = 32; i < 64; i++)				
				printf("%x", f->CTX->hash[i]);
		}
		else
			for (i = 0; i < 64; i++)				
				printf("%x", f->CTX->hash[i]);
				

		cout << "|";

		if (f->change)
			cout << "Was changed" << endl;
		else
			cout << "Wasn't changed" << endl;

	}
	
}

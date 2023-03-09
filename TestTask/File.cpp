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

// Конструктор для новых файлов из Paths.txt
File::File(char path_ch[]) {

	file_init();
	set_file_path(path_ch);	
	get_file_hash();

};

// Конструктор для проверки целостности 
File::File(ifstream& data_r) {

	file_init();
	read_data(data_r);
	get_file_hash();

};

File::~File() {

	 delete[] path;

};

void File::file_init() {

	//path = NULL;

	lOp = -1;

	change = Change::New;

	alg = Algoritm::crc32;

	hash_file = new Hash;
	memset(hash_file, 0x00, sizeof(Hash));

	CTX = new TGOSTHashContext;
	GOSTHashInit(CTX, 512);

}

// Запись пути к файлу в переменную path
// Если в пути есть русские символы, то их кодировка меняется с utf-8 на ansi
void File::set_file_path(char path_ch[]) {			
	
	path = new char[1024];
	utf8_to_rus(path_ch);
	strcpy(path, path_ch);

}

// Запись информации в файл Data.bin
// Информация для каждого файла: размер пути; алгоритм хеширования; путь; хеш;
// Для алгоритма GOST добавляется размер хеш-суммы
void File::write_data(ofstream& data_w) {

	int i;

	lOp = char_to_string(path).size();

	data_w.write((char*)&lOp, sizeof(int));
	data_w.write((char*)&alg, sizeof(alg));

	switch (alg) {

	case Algoritm::crc32: {

		for (i = 0; i < lOp; i++)
			data_w.write(&path[i], 1);

		data_w.write((char*)&hash_file->hash_crc32, sizeof(hash_file->hash_crc32));

		cout << "Inf wrote" << endl;

		break;
	}
	case Algoritm::GOST: {

		data_w.write((char*)&CTX->hash_size, sizeof(int));

		for (i = 0; i < lOp; i++)
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

		cout << "Inf wrote" << endl;

		break;
	}
	default:
		cout << "Error writing hash" << endl;
		break;
	}

}

// Считывается информация о файле для проверки целостности
void File::read_data(ifstream& data_r) {

	int i;

	data_r.read((char*)&lOp, sizeof(int));
	data_r.read((char*)&alg, sizeof(alg));

	path = new char[lOp];
	path[lOp] = '\0';

	switch (alg) {

	case Algoritm::crc32: {

		for (i = 0; i < lOp; i++)
			data_r.read(&path[i], 1);

		data_r.read((char*)&hash_file->hash_crc32, sizeof(hash_file->hash_crc32));

		cout << "Inf read" << endl;

		break;
	}
	case Algoritm::GOST: {

		data_r.read((char*)&CTX->hash_size, sizeof(int));

		for (i = 0; i < lOp; i++)
			data_r.read(&path[i], 1);

		if (CTX->hash_size == 256) {
			for (i = 32; i < 64; i++) {

				data_r.read((char*)&hash_file->hash_buf[i], 1);

			}
		}
		else {
			for (i = 0; i < 64; i++) {

				data_r.read((char*)&hash_file->hash_buf[i], 1);

			}
		}
		cout << "Inf read" << endl;
		break;
	}
	default:
		cout << "Error reading hash" << endl;
		break;
	}
}

int File::get_file_size(ifstream& file) {

	return file.tellg();

}

// Получение хеш-суммы файла
// 1. В n записывается размер файла;
// 2. Файл считывается;
// 3. Если список файлов обновился, то длина путей новых файлов имеет значение -1 и предлагается выбрать алгоритм хеширования;
// 4. В зависимости от выбранного/ранее использованного алгоритма высчитывается хеш-сумма;
// 5. Проверяется изменялся ли файл
void File::get_file_hash() {

	ifstream file(path, ios::binary | ios::ate);

	int n = file.tellg();

	file.seekg(0, ios::beg);

	uint8_t* buf = new uint8_t[2 * n];

	buf[n] = '\0';

	if (file) {

		file.read((char*)buf, n);

		if (lOp == -1) {

			cout << "Choose hash algoritm:" << endl
				<< "0 - crc32" << endl
				<< "1 - GOST" << endl;

			cin >> alg;

		}

		switch (alg) {

		case Algoritm::crc32:
			cout << "Hash calculating..." << endl;
			hash_f(buf, n);
			cout << "Hash calculated" << endl;
			break;

		case Algoritm::GOST:
		{

			cout << "Hash calculating..." << endl;

			GOSTHashUpdate(CTX, buf, n);

			if (buf != NULL)
				delete[] buf;

			GOSTHashFinal(CTX);

			cout << "Hash calculated" << endl;

			file.close();

			if (lOp == -1)
				change = Change::New;
			else
				was_changed();

			break;
		}

		default:
			cout << "The algorithm is chosen incorrectly" << endl;
			break;
		}

	}
	else
		cerr << "File didn't open" << endl;
}

// Проверка целостности для хеш-суммы, рассчитанной по алгоритму GOST
void File::was_changed() {

	int i;
	change = Change::N;

	if (CTX->hash_size == 256) {
		for (i = 32; i < 64; i++) {

			if (hash_file->hash_buf[i] != CTX->hash[i]) {

				change = Change::Y;
				break;

			}

		}
	}
	else {
		for (i = 0; i < 64; i++) {

			if (hash_file->hash_buf[i] != CTX->hash[i]) {

				change = Change::Y;
				break;

			}

		}
	}
}


// Вычисление хеш-суммы и проверка целостности файла по алгоритму crc32
void File::hash_f(unsigned char* buf, unsigned long len) {
	unsigned long crc = 0xFFFFFFFFUL;
	unsigned long tmp = 0;

	while (len--)
		crc = crc_table[(crc ^ *buf++) & 0xFF] ^ (crc >> 8);

	tmp = crc ^ 0xFFFFFFFFUL;

	if (hash_file->hash_crc32 != 0x00) {

		hash_file->hash_crc32 == tmp ? change = Change::N : change = Change::Y;

	}
	else {
		change = Change::New;
	}

	hash_file->hash_crc32 = tmp;

}

// Изменение кодировки utf-8 на ansi для путей,  в которых есть русские символы
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

// Удаление лишних байтов после изменения кодировки с utf-8 на ansi
void del_space(char path[], int i) {

	while (path[i] != '\0') {
		path[i] = path[i + 1];
		i++;
	}

}

// Вывод информации о файле: путь | хеш | изменён/не изменён/новый файл
void print(vector<File*>& files) {

	string path_str;
	int ptr_begin;
	int ptr_end;
	int i;					

	for (int j = 0; j < files.size(); j++){
		
		path_str = char_to_string(files[j]->path);

		ptr_begin = path_str.find("\\");
		ptr_end = path_str.rfind("\\");

		cout << j + 1 << " | ";
		cout << path_str.substr(0, ptr_begin + 1) << "....." << path_str.substr(ptr_end, path_str.size())
			<< " | " ;

		switch (files[j]->alg) {
		case Algoritm::crc32:
			cout << files[j]->hash_file;
			break;
		case Algoritm::GOST:
			if (files[j]->CTX->hash_size == 256) {
				for (i = 32; i < 64; i++)
					printf("%x", files[j]->CTX->hash[i]);
			}
			else
				for (i = 0; i < 64; i++)
					printf("%x", files[j]->CTX->hash[i]);
			break;
		default:
			cout << "Error print hash" << endl;
			break;
		}				

		cout << " | ";

		if (files[j]->change == Change::New)
			cout << "New file" << endl;
		else if(files[j]->change == Change::Y)
			cout << "Was changed" << endl;
		else
			cout << "Wasn't changed" << endl;

		cout << endl;

	}
}

void all_inf(File* f) {	

	int i;

	system("cls");

	cout << "Path: " << f->path << endl;

	switch (f->alg) {
	case Algoritm::crc32:
		cout << "Algorithm: " << "CRC32" << endl;
		cout << "Hash: " << f->hash_file << endl;
		break;
	case Algoritm::GOST:
		cout << "Algorithm: " << "GOST 34.11-2012" << endl;
		cout << "Hash: ";
		if (f->CTX->hash_size == 256) {
			for (i = 32; i < 64; i++)
				printf("%x", f->CTX->hash[i]);
		}
		else
			for (i = 0; i < 64; i++)
				printf("%x", f->CTX->hash[i]);
		cout << endl;
		break;
	default:
		cout << "Error print file information" << endl;
		break;
	}

	if (f->change == Change::New)
		cout << "Change: New file" << endl;
	else if (f->change == Change::Y)
		cout << "Change: Was changed" << endl;
	else
		cout << "Change: Wasn't changed" << endl;

}

istream& operator>>(std::istream& is, Algoritm& a)
{
	int n;
	is >> n;
	if (n == 0) a = Algoritm::crc32;
	else if (n == 1) a = Algoritm::GOST;	
	return is;
}

istream& operator>>(std::istream& is, Point& p)
{
	char s;
	is >> s;
	if (s == '0') p = Point::Exit;
	else if (s == '1') p = Point::One;
	else if (s == '2') p = Point::All;
	else if (s == '>') p = Point::Next;
	else if (s == '<') p = Point::Back;
	return is;
}

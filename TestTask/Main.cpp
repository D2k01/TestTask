#include "File.h"

using namespace std;

vector<File*> files;

int main() {

	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);

	setlocale(LC_ALL, "Russian");

	ifstream paths("Paths.txt");

	crc32_table_gen();
	char path_ch[512];

	while (paths.getline(path_ch, 512, '\n')) {

		files.push_back(new File(path_ch));	

	}	

	ofstream data("Data.txt");

	for (File* f : files) {

		cout << f->path << endl << f->hash << endl;
		f->write_data(data);

		if (f->change)
			cout << "Was changed" << endl;
		else
			cout << "Wasn't changed" << endl;

		cout << endl;

	}

	return 0;
}
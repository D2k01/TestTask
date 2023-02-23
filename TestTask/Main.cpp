#include "File.h"

using namespace std;

vector<File*> files;

int main() {

	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);

	setlocale(LC_ALL, "Russian");

	crc32_table_gen();

	ifstream data_r("Data.txt");

	string buf;

	if (data_r) {
		while (getline(data_r, buf)) {

			files.push_back(new File(buf));

		}
	}

	data_r.close();

	ifstream paths("Paths.txt");

	
	char path_ch[512];

	while (paths.getline(path_ch, 512, '\n')) {

		files.push_back(new File(path_ch));	

	}	

	paths.close();

	ofstream data_w("Data.txt");

	for (File* f : files) {		
		f->write_data(data_w);
	}

	data_w.close();

	ofstream paths_clear("Paths.txt");
	paths_clear.close();

	print(files);

	return 0;
}
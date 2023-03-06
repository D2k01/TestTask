#include "File.h"


using namespace std;

vector<File*> files;

int main() {

	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);

	setlocale(LC_ALL, "Russian");	

	ifstream data_r("Data.txt", ios::binary|ios::ate);	

	int end = data_r.tellg();

	data_r.seekg(0, ios::beg);

	if (data_r) {
		
		while(data_r.tellg() != end)
			files.push_back(new File(data_r));

	}
	else
		cout << "File \"Data.txt\" wasn't opened" << endl;

	data_r.close();
	
	

	ifstream paths("Paths.txt");

	
	char* path_ch = new char[128];	

	while (paths.getline(path_ch, 128, '\n')) {
		
		files.push_back(new File(path_ch));

	}	

	paths.close();	

	ofstream data_w("Data.txt", ios::binary);

	for (File* f : files) {		
		f->write_data(data_w);
	}

	data_w.close();

	ofstream paths_clear("Paths.txt");
	paths_clear.close();

	print(files);

	return 0;
}
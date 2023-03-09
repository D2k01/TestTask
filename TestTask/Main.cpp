#include "File.h"

using namespace std;



void menu(int n) {

	switch (n) {
	case 0:
		cout << "1 - Output information about one file" << endl			
			<< "0 - Exit" << endl;
		break;
	case 1:
		cout << "2 - Output information about all files" << endl
			<< "> - Next file" << endl
			<< "< - Previous file" << endl
			<< "0 - Exit" << endl;
		break;
	default:
		cout << "Error point of menu" << endl;
		break;
	}
	

}


int main() {

	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);

	setlocale(LC_ALL, "Russian");

	crc32_table_gen();
	vector<File*> files;

	//Считывание информации о файлах из Data.bin
	ifstream data_r("Data.bin", ios::binary|ios::ate);	

	int end = data_r.tellg();

	data_r.seekg(0, ios::beg);

	if (data_r) {
		
		while (data_r.tellg() != end) {
			files.push_back(new File(data_r));
			system("cls");
		}

	}
	else
		cout << "File \"Data.txt\" wasn't opened" << endl;

	system("cls");
	data_r.close();
	
	
	// Считывание путей из Paths.txt
	ifstream paths("Paths.txt");

	
	char* path_ch = new char[1024];	

	while (paths.getline(path_ch, 1024, '\n')) {
		
		files.push_back(new File(path_ch));
		system("cls");

	}	

	paths.close();	

	// Запись информации о файлах в Data.bin
	ofstream data_w("Data.bin", ios::binary);

	for (File* f : files) {		
		f->write_data(data_w);
	}

	data_w.close();

	// Очитска файла  Paths.txt
	ofstream paths_clear("Paths.txt");
	paths_clear.close();

	//Вывод информации о файлах	

	Point n;	
	int k = 0;	

	print(files);

	menu(0);
	cin >> n;

	while (n != Point::Exit) {
		
		switch (n) {		
		
		case Point::All:
			system("cls");			
			print(files);
			menu(0);
			cin >> n;
			break;
		case Point::One: {
			system("cls");
			print(files);
			do{
				cout << "Enter number of file: " << endl;
				cin >> k;
			} while (k < 0 || k > files.size());
			all_inf(files[k - 1]);
			menu(1);
			cin >> n;
			break;
		}
		case Point::Next:
			system("cls");
			if (k < files.size())
				all_inf(files[++k - 1]);
			else {
				all_inf(files[k - 1]);
				cout << endl << "Last file" << endl;
			}
			menu(1);
			cin >> n;
			break;
		case Point::Back:
			system("cls");
			if (k != 0)
				all_inf(files[--k - 1]);
			else {
				all_inf(files[k - 1]);
				cout << endl << "First file" << endl;
			}
			menu(1);
			cin >> n;
			break;
		default:
			cout << "Error point" << endl;
			n = Point::All;
			break;
		}

	}

	return 0;
}


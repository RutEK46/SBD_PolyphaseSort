#include "RecordContainer.h"
#include <conio.h>
#include <time.h>

std::string menuString = "\
0. Generate dataset to file\n\
1. Sort file\n\
2. Create own dataset\n\
3. Print file\n\
4. Experiment\n";

char set[] = { '0', '1', '2', '3', '4' };

bool isInSet(char c, char *set, int size)
{
	for (int i = 0; i < size; i++)
	{
		if (c == set[i])
			return true;
	}

	return false;
}


void experiment(std::string fileName, std::string fileNameOut)
{
	FILE *out;
	if (fopen_s(&out, fileNameOut.c_str(), "w+b") != 0)
	{
		std::string message = "Cannot open file " + fileNameOut + ".";
		throw std::exception(message.c_str());
	}

	fprintf_s(out, "Ilosc recordow\tIlosc odczytow\tIlosc zapisow\tIlosc faz");

	for (int i = 1; i <= 10; i++)
	{
		printf_s("\r%.2f%%", (i - 1) / 1.0 / 10 * 100);
		RecordContainer container(fileName);
		container.generateRandom(64 << i);
		int phase = container.sort(true);
		fprintf_s(out, "\r\n%d\t%d\t%d\t%d", 64 << i, TapeFile::getReadCounter(), TapeFile::getWriteCounter(), phase);
	}
	printf_s("\r100.00%%\n");
	fclose(out);
}


int main()
{
	srand(time(NULL));

	char c;
		
	std::cout << menuString;

	while(!isInSet(c = _getch(), set, sizeof(set) / sizeof(set[0]))) { }

	try
	{
		std::string fileName;
		switch (c)
		{
		case '0': {
			std::cout << "File name: ";
			std::cin >> fileName;
			RecordContainer container(fileName);
			int records;
			std::cout << "Records number: ";
			std::cin >> records;
			container.generateRandom(records);
			std::cout << "File generated." << std::endl;
			break;
		}
		case '1': {
			std::cout << "File name: ";
			std::cin >> fileName;
			RecordContainer container(fileName);
			container.sort();
			container.print();
			std::cout << "Done!" << std::endl;
			break;
		}
		case '2': {
			std::cout << "File name: ";
			std::cin >> fileName;
			RecordContainer container(fileName);
			container.write();
			break;
		}
		case '3': {
			std::cout << "File name: ";
			std::cin >> fileName;
			RecordContainer container(fileName);
			container.print();
			break;
		}
		case '4':
			experiment("tmp", "result.csv");
			break;
		}
	}
	catch (std::exception &ex)
	{
		std::cout << ex.what() << std::endl;
	}

	_getch();
	return 0;
}
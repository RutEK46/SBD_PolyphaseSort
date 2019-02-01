#pragma once
#include <queue>
struct Record
{
	static const int VALUE_COUNT = 15;
	unsigned long long flags;
	double values[VALUE_COUNT];

	void print();
	void write(int mValues);
	void generateRadomValues();
};


std::queue<double> recordToQueue(const Record &record);
Record queueToRecord(std::queue<double> &queue);
bool isInteger(double number);
int compare(double n1, double n2);
#include "Record.h"
#include <iostream>
#include <ctime>

void Record::print()
{
	int unused = 8 * sizeof(flags) - Record::VALUE_COUNT;
	unsigned long long tmpFlags = flags << unused;
	tmpFlags >>= unused;

	for (int i = 0; tmpFlags != 0; tmpFlags >>= 1, i++)
	{
		if (tmpFlags % 2 == 0) continue;

		std::cout << values[i] << std::endl;
	}
}


void Record::write(int mValues)
{
	if (mValues > VALUE_COUNT)
		mValues = VALUE_COUNT;

	flags = 0;
	for (int i = 0; i < mValues; i++)
	{
		std::cout << "Value " << i << ": ";
		std::cin >> values[i];
		flags <<= 1;
		flags |= 1;
	}
}


void Record::generateRadomValues()
{
	for (int i = 0; i < VALUE_COUNT; i++)
	{
		values[i] = (rand() % 100000) / 2.0;
	}
	flags = 0xFFFFFFFF;
}


std::queue<double> recordToQueue(const Record &record)
{
	std::queue<double> queue;
	int unused = 8 * sizeof(record.flags) - Record::VALUE_COUNT;
	unsigned long long flags = record.flags << unused;
	flags >>= unused;

	for (int i = 0; flags != 0; flags >>= 1, i++)
	{
		if (flags % 2 == 0) continue;

		queue.push(record.values[i]);
	}

	return queue;
}


Record queueToRecord(std::queue<double> &queue)
{
	Record record = { 0,{ 0 } };
	for (int i = 0; i < Record::VALUE_COUNT && !queue.empty(); i++)
	{
		record.values[i] = queue.front();
		record.flags = (record.flags << 1) | 1;
		queue.pop();
	}
	return record;
}


bool isInteger(double number)
{
	return number == double(long long(number)) || number >= 1ll << 52 || number <= ~(1ll << 52);
}


int compare(double n1, double n2)
{
	if (isInteger(n1) && !isInteger(n2))
		return 1;

	if (!isInteger(n1) && isInteger(n2))
		return -1;

	if (n1 < n2)
		return -1;

	return n1 > n2;
}
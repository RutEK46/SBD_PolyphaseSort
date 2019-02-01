#pragma once
#include "SeriesStream.h"
#include "Record.h"

class RecordContainer
{
public:
	RecordContainer(std::string fileName)
		: m_fileName(fileName),
		m_isEndOfStreamFunc([](Record record)
	{ 
		return record.flags == 0; 
	})
	{ }

	void print();
	void write();
	void generateRandom(int recordCount);
	int sort(bool forceNoPrint = false);
	

private:
	typedef SeriesStream<double, Record, 128> SeriesStream;
	std::string m_fileName;
	std::function<bool(Record)> m_isEndOfStreamFunc;

	static void _count()
	{
		TapeFile::count();
	}

	static void _stopCounting()
	{
		TapeFile::stopCounting();
	}

	static void _resetCounter()
	{
		TapeFile::resetCounter();
	}

	static int _getReadCounter()
	{
		return TapeFile::getReadCounter();
	}

	static int _getWriteCounter()
	{
		return TapeFile::getWriteCounter();
	}
};
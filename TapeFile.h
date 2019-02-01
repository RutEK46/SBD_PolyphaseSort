#pragma once
#include <iostream>
#include <string>
#include <fstream>

class TapeFile
{
public:
	TapeFile(const std::string fileName, const std::string fileMode)
		: m_fileName(fileName), m_savedPosition(0)
	{
		errno_t err;
		err = fopen_s(&m_file, fileName.c_str(), fileMode.c_str());

		if (err != 0)
		{
			std::string message = "Cannot open file " + fileName + " with file mode \"" + fileMode + "\".";
			throw std::exception(message.c_str());
		}
	}

	~TapeFile()
	{
		fclose(m_file);
	}

	std::string getFileName() const
	{
		return m_fileName;
	}

	void TapeFile::truncate()
	{
		fclose(m_file);
		errno_t err = fopen_s(&m_file, m_fileName.c_str(), "w+b");
		if (err != 0)
		{
			std::string message = "Cannot truncate file " + m_fileName + ".";
			throw std::exception(message.c_str());
		}
		m_savedPosition = 0;
	}

	void TapeFile::rewind()
	{
		::rewind(m_file);
	}

	void savePosition(int currentSize)
	{
		m_savedPosition = _ftelli64(m_file) - currentSize;
	}

	void loadPosition()
	{
		_fseeki64(m_file, m_savedPosition, SEEK_SET);
	}
	
	size_t read(void *buffer, size_t bufferSize, size_t elementSize, size_t elementCount)
	{
		size_t size = fread_s(buffer, bufferSize, elementSize, elementCount, m_file);
		if (size != 0 && m_isCounting)
			m_readCounter++;
		return size;
	}

	size_t write(const void *buffer, size_t elementSize, size_t elementCount)
	{
		if (m_isCounting)
			m_writeCounter++;
		return fwrite(buffer, elementSize, elementCount, m_file);
	}

	static void resetCounter()
	{
		m_readCounter = m_writeCounter = 0;
	}

	static void count()
	{
		m_isCounting = true;
	}

	static void stopCounting()
	{
		m_isCounting = false;
	}

	static int getReadCounter()
	{
		return m_readCounter;
	}

	static int getWriteCounter()
	{
		return m_writeCounter;
	}

private:
	std::fstream m_fstream;
	FILE *m_file;
	std::string m_fileName;
	long long m_savedPosition;
	static int m_readCounter;
	static int m_writeCounter;
	static bool m_isCounting;

	TapeFile(const TapeFile &copy);
};
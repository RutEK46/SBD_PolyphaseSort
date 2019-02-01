#pragma once
#include "TapeFile.h"

template<const int BUFFER_BYTES>
class TapeBuffer
{
public:
	TapeBuffer(const std::string fileName, const std::string fileMode)
		: m_tapeFile(fileName, fileMode), m_iterator(0), m_currentSize(0)
	{ }

	~TapeBuffer()
	{
		if (isSetOnlyForWriting() && m_iterator != 0)
			_saveBuffer();
	}

	std::string getFileName() const
	{
		return m_tapeFile.getFileName();
	}

	bool isSetOnlyForReading() const
	{
		return m_currentSize >= 0;
	}

	bool isSetOnlyForWriting() const
	{
		return m_currentSize == -1;
	}

	void setOnlyForReading()
	{
		if (isSetOnlyForWriting() && m_iterator != 0)
			_saveBuffer();

		m_tapeFile.rewind();
		_loadBuffer();
	}

	void setOnlyForWriting()
	{
		m_tapeFile.truncate();
		m_currentSize = -1;
		m_iterator = 0;
		memset(m_buffer, 0, BUFFER_BYTES);
	}

	void savePosition()
	{
		m_tapeFile.savePosition(m_currentSize);
		m_savedIterator = m_iterator;
		m_savedCurrentSize = m_currentSize;
	}

	void loadPosition()
	{
		m_tapeFile.loadPosition();
		_loadBuffer();
		m_iterator = m_savedIterator;
		m_currentSize = m_savedCurrentSize;
	}

	int read(void *dst, int size)
	{
		if (m_currentSize == -1)
		{
			std::string message = "Reading from tape buffer named " + getFileName() + " setted up only for writing.";
			throw std::exception(message.c_str());
		}

		int read = 0;
		for (;;)
		{
			if (m_iterator + size - read <= m_currentSize)
			{
				memcpy((char*)dst + read, m_buffer + m_iterator, size - read);
				m_iterator += size - read;
				return 0;
			}
			else if (m_iterator != m_currentSize)
			{
				int size1 = m_currentSize - m_iterator;
				memcpy((char*)dst + read, m_buffer + m_iterator, size1);
				read += size1;
			}

			_loadBuffer();
			if (m_currentSize == 0)
				return 1;
		}
	}

	void write(const void *src, int size)
	{
		if (m_currentSize >= 0)
		{
			std::string message = "Writing to tape buffer named " + getFileName() + "setted up only for reading.";
			throw std::exception(message.c_str());
		}

		int saved = 0;
		for (;;)
		{
			if (m_iterator + size - saved <= BUFFER_BYTES)
			{
				memcpy(m_buffer + m_iterator, (const char*)src + saved, size - saved);
				m_iterator += size - saved;
				return;
			}
			else if (m_iterator != BUFFER_BYTES)
			{
				int size1 = BUFFER_BYTES - m_iterator;
				memcpy(m_buffer + m_iterator, (const char*)src + saved, size1);
				saved += size1;
			}

			_saveBuffer();
		}
	}

private:
	TapeFile m_tapeFile;
	char m_buffer[BUFFER_BYTES];
	int m_iterator;
	int m_savedIterator;
	int m_currentSize;
	int m_savedCurrentSize;

	TapeBuffer(const TapeBuffer &tapeBuffer);

	void _loadBuffer()
	{
		m_currentSize = m_tapeFile.read(m_buffer, BUFFER_BYTES, 1, BUFFER_BYTES);
		m_iterator = 0;
	}

	void _saveBuffer()
	{
		m_tapeFile.write(m_buffer, 1, BUFFER_BYTES);
		m_iterator = 0;
		memset(m_buffer, 0, BUFFER_BYTES);
	}
};
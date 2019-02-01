#pragma once
#include <functional>
#include "TapeBuffer.h"

template<class TRecord, const int BUFFER_BYTES>
class RecordStream
{
public:
	RecordStream(std::string fileName, std::string fileMode,
		std::function<bool(TRecord&)> isEndOfStreamFunc)
		: m_tapeBuffer(fileName, fileMode),
		m_isEndOfStreamFunc(isEndOfStreamFunc),
		m_isModeSetted(false), m_EOS(true)
	{ }

	bool EOS() const
	{
		return m_EOS;
	}

	std::string getFileName() const
	{
		return m_tapeBuffer.getFileName();
	}

	bool isModeSetOnlyForReading() const
	{
		return m_isModeSetted && m_tapeBuffer.isSetOnlyForReading();
	}

	bool isModeSetOnlyForWriting() const
	{
		return m_isModeSetted && m_tapeBuffer.isSetOnlyForWriting();
	}
	
	void setModeOnlyForReading()
	{
		m_tapeBuffer.setOnlyForReading();
		m_isModeSetted = true;
		_loadRecord();
	}

	void setModeOnlyForWriting()
	{
		m_tapeBuffer.setOnlyForWriting();
		m_isModeSetted = true;
	}

	void savePosition()
	{
		m_tapeBuffer.savePosition();
		m_savedRecord = m_record;
		m_savedEOS = m_EOS;
	}

	void loadPosition()
	{
		m_tapeBuffer.loadPosition();
		m_record = m_savedRecord;
		m_EOS = m_savedEOS;
	}

	TRecord getRecord()
	{
		if (!m_isModeSetted)
		{
			std::string message = "Reading from record stream named " + getFileName() + " with unsetted mode.";
			throw std::exception(message.c_str());
		}
			
		TRecord r = m_record;
		_loadRecord();
		return r;
	}

	void setRecord(const TRecord &record)
	{
		if (!m_isModeSetted)
		{
			std::string message = "Writing to record stream named " + getFileName() + " with unsetted mode.";
			throw std::exception(message.c_str());
		}

		m_tapeBuffer.write(&record, sizeof(TRecord));
	}

private:
	TapeBuffer<BUFFER_BYTES> m_tapeBuffer;
	std::function<bool(TRecord&)> m_isEndOfStreamFunc;
	TRecord m_record;
	TRecord m_savedRecord;
	bool m_isModeSetted;
	bool m_EOS;
	bool m_savedEOS;

	RecordStream(const RecordStream &copy);

	void _loadRecord()
	{
		int code = m_tapeBuffer.read(&m_record, sizeof(TRecord));
		m_EOS = m_isEndOfStreamFunc(m_record) || code == 1;
	}
};
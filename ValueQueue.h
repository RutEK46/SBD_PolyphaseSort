#pragma once
#include <queue>
#include "RecordStream.h"


template<class TValue, class TRecord, const int BUFFER_BYTES>
class ValueQueue
{
public:
	ValueQueue(std::string fileName, std::string fileMode, int maxQueueSize,
		std::function<bool(TRecord&)> isEndOfStreamFunc,
		std::function<std::queue<TValue>(TRecord&)> recordToQueueFunc,
		std::function<TRecord(std::queue<TValue>&)> queueToRecordFunc)
		: m_recordStream(fileName, fileMode, isEndOfStreamFunc),
		m_maxQueueSize(maxQueueSize),
		m_recordToQueueFunc(recordToQueueFunc),
		m_queueToRecordFunc(queueToRecordFunc)
	{ }

	~ValueQueue()
	{
		if (m_recordStream.isModeSetOnlyForWriting() && !m_queue.empty())
		{
			TRecord record = m_queueToRecordFunc(m_queue);
			m_recordStream.setRecord(record);
		}
	}

	std::string getFileName() const
	{
		return m_recordStream.getFileName();
	}

	bool isModeSetOnlyForReading() const
	{
		return m_recordStream.isModeSetOnlyForReading();
	}

	bool isModeSetOnlyForWriting() const
	{
		return m_recordStream.isModeSetOnlyForWriting();
	}

	bool EOS() const
	{
		return m_queue.empty() && m_recordStream.EOS();
	}

	void setModeOnlyForReading()
	{
		if (m_recordStream.isModeSetOnlyForWriting() && !m_queue.empty())
		{
			TRecord record = m_queueToRecordFunc(m_queue);
			m_recordStream.setRecord(record);
		}

		m_queue = std::queue<TValue>();
		m_recordStream.setModeOnlyForReading();
	}

	void setModeOnlyForWriting()
	{
		m_queue = std::queue<TValue>();
		m_recordStream.setModeOnlyForWriting();
	}

	void savePosition()
	{
		m_recordStream.savePosition();
		m_savedQueue = m_queue;
	}

	void loadPosition()
	{
		m_recordStream.loadPosition();
		m_queue = m_savedQueue;
	}

	void popValue()
	{
		m_queue.pop();
	}

	TValue frontValue()
	{
		if (m_queue.empty())
		{
			TRecord record = m_recordStream.getRecord();
			m_queue = m_recordToQueueFunc(record);
		}

		return m_queue.front();
	}

	void pushValue(TValue value)
	{
		if (m_queue.size() == m_maxQueueSize)
		{
			TRecord record = m_queueToRecordFunc(m_queue);
			m_recordStream.setRecord(record);
			m_queue = std::queue<TValue>();
		}

		m_queue.push(value);
	}

private:
	RecordStream<TRecord, BUFFER_BYTES> m_recordStream;
	std::queue<TValue> m_queue;
	std::queue<TValue> m_savedQueue;
	std::function<std::queue<TValue>(TRecord&)> m_recordToQueueFunc;
	std::function<TRecord(std::queue<TValue>&)> m_queueToRecordFunc;
	int m_maxQueueSize;

	ValueQueue(const ValueQueue &copy);
};
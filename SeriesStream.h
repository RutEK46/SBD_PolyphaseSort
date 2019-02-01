#pragma once
#include "ValueQueue.h"

template<class TValue, class TRecord, const int BUFFER_BYTES>
class SeriesStream
{
public:
	SeriesStream(std::string fileName, std::string fileMode, int maxQueueSize,
		std::function<bool(TRecord&)> isEndOfStreamFunc,
		std::function<std::queue<TValue>(TRecord&)> recordToQueueFunc,
		std::function<TRecord(std::queue<TValue>&)> queueToRecordFunc)
		: m_valueQueue(fileName, fileMode, maxQueueSize, isEndOfStreamFunc, recordToQueueFunc, queueToRecordFunc)
	{ }

	std::string getFileName() const
	{
		return m_valueQueue.getFileName();
	}

	bool isModeSetOnlyForReading() const
	{
		return m_valueQueue.isModeSetOnlyForReading();
	}

	bool isModeSetOnlyForWriting() const
	{
		return m_valueQueue.isModeSetOnlyForWriting();
	}

	bool EOS() const
	{
		return m_valueQueue.EOS();
	}

	void setModeOnlyForReading()
	{
		m_valueQueue.setModeOnlyForReading();
	}

	void setModeOnlyForWriting()
	{
		m_seriesCount = 0;
		m_valueQueue.setModeOnlyForWriting();
	}

	int seriesCount() const
	{
		return m_seriesCount;
	}

	TValue getFirstValue()
	{
		return m_valueQueue.frontValue();
	}

	TValue getLastValue()
	{
		return m_lastValue;
	}

	void print(std::function<int(TValue, TValue)> compareFunc)
	{
		if (EOS())
		{
			std::cout << "<empty>" << std::endl << std::endl;
			return;
		}

		TValue value = m_valueQueue.frontValue();
		int series = 1;
		do
		{
			if (compareFunc(value, m_valueQueue.frontValue()) == 1)
			{
				std::cout << "| ";
				series++;
			}

			std::cout << m_valueQueue.frontValue() << ' ';
			value = m_valueQueue.frontValue();
			m_valueQueue.popValue();
		} while (!EOS());
		std::cout << "(" << series << " series)" << std::endl << std::endl;
	}

	void savePosition()
	{
		m_valueQueue.savePosition();
		m_savedLastValue = m_lastValue;
	}

	void loadPosition()
	{
		m_valueQueue.loadPosition();
		m_lastValue = m_savedLastValue;
	}

	static void move(SeriesStream &left, SeriesStream &rigth, std::function<int(TValue, TValue)> compareFunc)
	{
		if (rigth.EOS()) return;
		if (left.m_seriesCount != 0 && compareFunc(left.m_lastValue, rigth._frontValue()) != 1)
			left.m_seriesCount--;

		TValue value;
		do
		{
			value = rigth._frontValue();
			rigth._popValue();
			left._pushValue(value);
		} while (!rigth.EOS() && compareFunc(value, rigth._frontValue()) != 1);

		left.m_lastValue = value;
		left.m_seriesCount++;
	}

	static void merge(SeriesStream &left, SeriesStream& first, SeriesStream& second,
		std::function<int(TValue, TValue)> compareFunc)
	{
		if (first.EOS() && second.EOS()) return;
		if (first.EOS()) return move(left, second, compareFunc);
		if (second.EOS()) return move(left, first, compareFunc);

		TValue v1 = first._frontValue(), v2 = second._frontValue();

		if (compareFunc(v1, v2) != 1)
		{
			if (left.m_seriesCount != 0 && compareFunc(left.m_lastValue, v1) != 1)
				left.m_seriesCount--;
		}
		else
		{
			if (left.m_seriesCount != 0 && compareFunc(left.m_lastValue, v2) != 1)
				left.m_seriesCount--;
		}

		for (;;)
		{
			if (compareFunc(v1, v2) != 1)
			{
				left._pushValue(v1);
				first._popValue();
				if (first.EOS() || compareFunc(v1, first._frontValue()) == 1)
					return move(left, second, compareFunc);
				v1 = first._frontValue();
			}
			else
			{
				left._pushValue(v2);
				second._popValue();
				if (second.EOS() || compareFunc(v2, second._frontValue()) == 1)
					return move(left, first, compareFunc);
				v2 = second._frontValue();
			}
		}
	}

private:
	ValueQueue<TValue, TRecord, BUFFER_BYTES> m_valueQueue;
	TValue m_lastValue;
	TValue m_savedLastValue;
	int m_seriesCount;

	TValue _frontValue()
	{
		return m_valueQueue.frontValue();
	}

	void _popValue()
	{
		m_valueQueue.popValue();
	}

	void _pushValue(TValue value)
	{
		m_valueQueue.pushValue(value);
	}
};
#include "RecordContainer.h"
#include "Record.h"
#include <conio.h>


void RecordContainer::print()
{
	RecordStream<Record, 128> recordStream(m_fileName, "r+b", m_isEndOfStreamFunc);
	recordStream.setModeOnlyForReading();
	for (int i = 0; !recordStream.EOS(); i++)
	{
		std::cout << "RECORD " << i << ":" << std::endl;
		Record record = recordStream.getRecord();
		record.print();
		_getch();
	}
}


void RecordContainer::write()
{
	RecordStream<Record, 128> recordStream(m_fileName, "w+b", m_isEndOfStreamFunc);
	recordStream.setModeOnlyForWriting();
	
	int n, m;

	std::cout << "How many record do you want to create: ";
	std::cin >> n;
	for (int i = 0; i < n; i++)
	{
		std::cout << "How many values do you want to write to record " << i << ": ";
		std::cin >> m;
		Record record;
		record.write(m);
		recordStream.setRecord(record);
	}
}


void RecordContainer::generateRandom(int recordCount)
{
	RecordStream<Record, 128> recordStream(m_fileName, "w+b", m_isEndOfStreamFunc);
	recordStream.setModeOnlyForWriting();
	for (int i = 0; i < recordCount; i++)
	{
		Record record;
		record.generateRadomValues();
		recordStream.setRecord(record);
	}
}


int RecordContainer::sort(bool forceNoPrint)
{
	int phaseCount = 0;
	SeriesStream seriesStream[3] = {
		{ "Tape1", "w+b", Record::VALUE_COUNT, m_isEndOfStreamFunc, &recordToQueue, &queueToRecord },
		{ "Tape2", "w+b", Record::VALUE_COUNT, m_isEndOfStreamFunc, &recordToQueue, &queueToRecord },
		{ m_fileName, "r+b", Record::VALUE_COUNT, m_isEndOfStreamFunc, &recordToQueue, &queueToRecord }
	};
	char c;
	if (forceNoPrint)
		c = 'n';
	else
	{
		std::cout << "Sorting!!! Do you want to print the tapes in every phase? (y/n)";
		c = 0;
		while (c != 'n' && c != 'y')
		{
			c = _getch();
		}
	}

	std::cout << std::endl;
	if (c == 'y')
	{
		for (int i = 0; i < 3; i++)
		{
			std::cout << "Tape " << i+1 << ": ";
			seriesStream[i].setModeOnlyForReading();
			seriesStream[i].print(&compare);
		}
	}

	_resetCounter();
	_count();
	seriesStream[2].setModeOnlyForReading();

	if (seriesStream[2].EOS()) goto END;

	seriesStream[0].setModeOnlyForWriting();
	seriesStream[1].setModeOnlyForWriting();

	int fibb0 = 1, fibb1 = 1;
	int emptySeries = 0;

	SeriesStream::move(seriesStream[0], seriesStream[2], &compare);

	if (seriesStream[2].EOS()) goto END;

	SeriesStream::move(seriesStream[1], seriesStream[2], &compare);

	while (!seriesStream[2].EOS())
	{
		fibb0 += fibb1;
		while (!seriesStream[2].EOS() && seriesStream[0].seriesCount() != fibb0)
		{
			SeriesStream::move(seriesStream[0], seriesStream[2], compare);
		}

		if (seriesStream[2].EOS())
		{
			emptySeries = fibb0 - seriesStream[0].seriesCount();
			break;
		}

		fibb1 += fibb0;
		while (!seriesStream[2].EOS() && seriesStream[1].seriesCount() != fibb1)
		{
			SeriesStream::move(seriesStream[1], seriesStream[2], compare);
		}
		

		emptySeries = fibb1 - seriesStream[1].seriesCount();
	}


	phaseCount = 1;
	SeriesStream *pSeries[3] = { seriesStream, seriesStream + 1, seriesStream + 2 };
	if (fibb0 < fibb1)
		std::swap(pSeries[0], pSeries[1]);

	if (c == 'y')
	{
		_stopCounting();
		std::cout << std::endl << "Phase 1!" << std::endl;
		for (int i = 0; i < 2; i++)
		{
			std::cout << "Tape " << i + 1 << ": ";
			seriesStream[i].setModeOnlyForReading();
			seriesStream[i].print(&compare);
		}
		std::cout << "Tape 3: <empty>" << std::endl << std::endl;
		_count();
	}

	pSeries[0]->setModeOnlyForReading();
	pSeries[1]->setModeOnlyForReading();
	pSeries[2]->setModeOnlyForWriting();

	for (int i = 0; i < emptySeries; i++)
	{
		SeriesStream::move(*pSeries[2], *pSeries[1], &compare);
	}

	for (phaseCount = 2;!pSeries[0]->EOS() && !pSeries[1]->EOS(); phaseCount++)
	{
		while (!pSeries[1]->EOS())
		{
			SeriesStream::merge(*pSeries[2], *pSeries[0], *pSeries[1], &compare);
		}
		
		if (c == 'y')
		{
			_stopCounting();
			std::cout << std::endl << "Phase " << phaseCount << "!" << std::endl;
			for (int i = 0; i < 3; i++)
			{
				std::cout << "Tape " << i + 1 << ": ";
				if (seriesStream[i].isModeSetOnlyForReading() && seriesStream[i].EOS())
					std::cout << "<empty>" << std::endl;
				else if (seriesStream[i].isModeSetOnlyForReading())
				{
					seriesStream[i].savePosition();
					seriesStream[i].print(&compare);
					seriesStream[i].loadPosition();
				}
				else
				{
					seriesStream[i].setModeOnlyForReading();
					seriesStream[i].print(&compare);
				}
			}
			_count();
		}

		pSeries[1]->setModeOnlyForWriting();
		pSeries[2]->setModeOnlyForReading();
		std::swap(pSeries[1], pSeries[2]);
		std::swap(pSeries[0], pSeries[1]);
	}
	phaseCount--;

	if (&seriesStream[2] != pSeries[0])
	{
		seriesStream[2].setModeOnlyForWriting();
		pSeries[0]->setModeOnlyForReading();
		while (!pSeries[0]->EOS())
		{
			SeriesStream::move(seriesStream[2], *pSeries[0], compare);
		}
	}

END:
	if (!forceNoPrint)
	{
		std::cout << "Readings from disc: " << _getReadCounter() << std::endl;
		std::cout << "Writings to disc: " << _getWriteCounter() << std::endl;
		std::cout << "Phases: " << phaseCount << std::endl;
		_getch();
	}
	return phaseCount;
}
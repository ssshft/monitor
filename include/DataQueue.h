#pragma once

//#include "concurrentqueue.h"
#include "concurrentqueue/blockingconcurrentqueue.h"
#include "Utility.h"


class DataQueue {
public:
	static DataQueue& GetInstance();
	void InsertMdData(SubData& data);
	void PopMdData(SubData& data);
	void InsertTradeData(SubData& data);
	void PopTradeData(SubData& data);
	~DataQueue();

private:
	DataQueue();
	DataQueue(const DataQueue& dataQueue);
	const DataQueue& operator=(const DataQueue& dataQueue);

	//moodycamel::ConcurrentQueue<BaseData*> q;
	moodycamel::BlockingConcurrentQueue<SubData> qMd;
	moodycamel::BlockingConcurrentQueue<SubData> qTrade;
};

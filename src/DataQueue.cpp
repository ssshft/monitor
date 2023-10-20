#include "DataQueue.h"

DataQueue::DataQueue() {
}

DataQueue::~DataQueue() {
}

DataQueue& DataQueue::GetInstance() {
	static DataQueue dataQueue;
	return dataQueue;
}

void DataQueue::InsertMdData(SubData& data) {
    qMd.enqueue(data);
}

void DataQueue::PopMdData(SubData& data) {
	//q.try_dequeue(data);
	qMd.wait_dequeue(data);
}

void DataQueue::InsertTradeData(SubData& data) {
    qTrade.enqueue(data);
}

void DataQueue::PopTradeData(SubData& data) {
    qTrade.wait_dequeue(data);
}

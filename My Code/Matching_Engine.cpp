//
// Created by samue on 27/08/2026.
//

/*
 * TO-DO: make matching engine pluggable for itch local data and polymarket apis
 */

#include "Matching_Engine.h"
#include <map>
#include <boost/type.hpp>
#include <boost/asio/registered_buffer.hpp>
#include <boost/core/data.hpp>

#include "Fetch_Orders.h"
#include "simdjson.h"
using namespace simdjson;


//actual code
void TimePriorityEngine::add_order(bool is_buy, Price price, Order order) {
 auto& book = is_buy ? bids_ : asks_;
 book[price].push_back(std::move(order));
}

void TimePriorityEngine::remove_order(bool is_buy, Price price, Order order) {
 auto& book = is_buy ? bids_ : asks_;
 book[price].erase(std::move(order));
}

void TimePriorityEngine::match_order() {
 auto& bid_list = bids_.begin()->second;
 auto& ask_list = asks_.begin()->second;
 Order& bid = bid_list.front();
 Order& ask = ask_list.front();

 std::uint64_t difference = std::min(bid.quantity, ask.quantity);
 bid.quantity -= difference;
 ask.quantity -= difference; //min finds the smallest value between the two values and then i minus that from both

 if (bid.quantity <= 0) {
  bid_list.pop_front();
 }
 if (ask.quantity <= 0) {
  ask_list.pop_front();
 }
 if (bid_list.empty()) {
  bids_.erase(bids_.begin());
 }
 if (ask_list.empty()) {
  asks_.erase(asks_.begin());
 }
}


void TimePriorityEngine::cancel_order(bool is_buy, Price price, Order order) {
 auto& book = is_buy ? bids_ : asks_;
 book[price].erase(std::move(order));
}

void apply(MarketEvent const& e) {
 //TO-DO: set up parser
}

//lock-free buffer
//This is to keep the higher-latency operations like moving the data off the hotpath so one can ingest data quickly.
//The data ingestion occurs on it's own separate thread with an accumulator
void flush_to_kdb(std::vector<Trade> const& batch);

TimePriorityEngine::TimePriorityEngine() {
 writer_thread_ = std::thread(&TimePriorityEngine::writer_loop, this);
}

TimePriorityEngine::~TimePriorityEngine() {
 running_ = false;
 writer_thread_.join();

}
void TimePriorityEngine::record_trade(Trade trade) {
 if (!trade_queue_.push(trade)) {
  //this is for when the queue is full
 }
}

void TimePriorityEngine::writer_loop() {
 std::vector<Trade> batch; //accumulator is effectively the actual buffer
 while (running_) { //loop to keep it alive
  Trade trade; //the trade we just ingested from the hotpath
  while (trade_queue_.pop(trade)) { //loop that drains everything in the queue
   batch.push_back(trade); //accumulate trades in the accumulator
   if (batch.size() >= 1000) { //safety flush in case the cache fills
    flush_to_kdb(batch);
    batch.clear();
   }
  }
  if (!batch.empty()) { //actual flush at the end of the loop
   flush_to_kdb(batch);
   batch.clear();
  }
  //pacing sleep
  //std::this_thread::sleep_for(std::chrono::milliseconds(5));
 }
 //final drain after while loop finishes
 Trade trade;
 while (trade_queue_.pop(trade)) {
  batch.push_back(std::move(trade));
 } if (!batch.empty()) {
  flush_to_kdb(batch);
 }
}
//
// Created by samue on 28/08/2026.
//
/*
 * TO-DO: take output from main.cpp and resolve markets from the websocket
 * build the storage.q to take from the summarised order since itll have the same shape (surely?)
 */

#ifndef RESOLUTIONLOGIC_H
#define RESOLUTIONLOGIC_H

#include <string>
#include <list>
#include <cstdint>
#include <map>
#include <boost/lockfree/spsc_queue.hpp>
#include <thread>
#include <atomic>

using Price = int64_t;

struct Order {
 std::string id;
 std::uint64_t quantity;
};

struct Trade {
 std::string bid_id;
 std::string ask_id;
 Price price;
 std::uint64_t quantity;
};

class TimePriorityEngine {
public:
 TimePriorityEngine();
 ~TimePriorityEngine();
 void record_trade(Trade trade);
 void on_message(std::string const& raw); //called by websocket lambda
 void add_order(bool is_buy, Price price, Order order);
 void remove_order(bool is_buy, Price price, Order order);
 void match_order();
 void cancel_order(bool is_buy, Price price, Order order);

private:
 boost::lockfree::spsc_queue<Trade, boost::lockfree::capacity<1024>> trade_queue_;
 std::thread writer_thread_;
 std::atomic<bool> running_{true};
 void writer_loop();
 std::map<Price, std::list<Order>, std::greater<Price>> bids_; //ordered maps that sort in seperate order (lowest sell, highest bid)
 std::map<Price, std::list<Order>> asks_;
};
#endif // RESOLUTIONLOGIC_H

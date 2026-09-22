// adapters/itch_adapter.cpp
//
// Translation layer over itchcpp (https://github.com/bbalouki/itchcpp, MIT,
// Copyright (c) bbalouki). All binary decoding is itchcpp's work
// These are for adapting the struct into the data shape you want

#include "itch_adapter.h"

#include "itch/messages.hpp"
#include "itch/parser.hpp"

#include <chrono>
#include <type_traits>
#include <variant>

namespace {

std::uint64_t now_ns() {
    return static_cast<std::uint64_t>(
        std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count());
}

}  // namespace

struct ItchBinaryAdapter::Impl {
    itch::Parser parser;
};

ItchBinaryAdapter::ItchBinaryAdapter() : impl_(std::make_unique<Impl>()) {}
ItchBinaryAdapter::~ItchBinaryAdapter() = default;
ItchBinaryAdapter::ItchBinaryAdapter(ItchBinaryAdapter&&) noexcept = default;
ItchBinaryAdapter& ItchBinaryAdapter::operator=(ItchBinaryAdapter&&) noexcept = default;

void ItchBinaryAdapter::parse(std::string_view raw, std::vector<MarketEvent>& out) {
    out.clear();
    if (raw.empty()) return;

    const std::uint64_t recv = now_ns();

    impl_->parser.parse(raw.data(), raw.size(), [&](itch::Message const& msg) {
        MarketEvent e{};
        e.recv_ns   = recv;
        bool is_book_event = true;

        std::visit([&](auto const& m) {
            using T = std::decay_t<decltype(m)>;

            // NOTE: ITCH timestamps are nanoseconds since midnight (US/Eastern),
            // NOT since the unix epoch. Rebase against the session date if you
            // need absolute time -- the session date is not in the message.
            if constexpr (std::is_same_v<T, itch::AddOrderMessage>) {
                e.type     = EventType::Add;
                e.ts_ns    = m.timestamp;
                e.book_id  = m.stock_locate;
                e.order_id = m.order_reference_number;
                e.side     = (m.buy_sell_indicator == 'B') ? Side::Buy : Side::Sell;
                e.price    = static_cast<Price>(m.price);   // already 1/10000 USD, integer
                e.quantity = m.shares;

            } else if constexpr (std::is_same_v<T, itch::OrderExecutedMessage>) {
                e.type     = EventType::Execute;
                e.ts_ns    = m.timestamp;
                e.book_id  = m.stock_locate;
                e.order_id = m.order_reference_number;
                e.quantity = m.executed_shares;

            } else if constexpr (std::is_same_v<T, itch::OrderCancelMessage>) {
                e.type     = EventType::Cancel;              // partial: reduces size
                e.ts_ns    = m.timestamp;
                e.book_id  = m.stock_locate;
                e.order_id = m.order_reference_number;
                e.quantity = m.cancelled_shares;

            } else if constexpr (std::is_same_v<T, itch::OrderDeleteMessage>) {
                e.type     = EventType::Delete;              // full removal
                e.ts_ns    = m.timestamp;
                e.book_id  = m.stock_locate;
                e.order_id = m.order_reference_number;

            } else if constexpr (std::is_same_v<T, itch::OrderReplaceMessage>) {
                e.type         = EventType::Replace;
                e.ts_ns        = m.timestamp;
                e.book_id      = m.stock_locate;
                e.order_id     = m.original_order_reference_number;
                e.new_order_id = m.new_order_reference_number;
                e.price        = static_cast<Price>(m.price);
                e.quantity     = m.shares;
                // Side is not repeated on Replace -- engine carries it over
                // from the original order.

            } else {
                // TODO: AddOrderMPIDAttributionMessage, OrderExecutedWithPrice,
                // NonCrossTrade, CrossTrade, BrokenTrade. Field names not yet
                // verified against itchcpp's headers -- add once confirmed.
                is_book_event = false;
            }
        }, msg);

        if (is_book_event) out.push_back(e);
    });
}

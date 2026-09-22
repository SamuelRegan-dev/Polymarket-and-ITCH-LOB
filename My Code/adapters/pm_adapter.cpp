// adapters/pm_adapter.cpp
//
// !! SCHEMA IS FILLER -- see header. Everything marked FILLER below is a guess
// that must be replaced once real Polymarket frames have been captured.

#include "pm_adapter.h"
#include "../simdjson.h"

#include <chrono>
#include <string>
#include <unordered_map>

namespace {

// FILLER: Polymarket quotes probabilities in [0,1]. Six decimal places chosen
// arbitrarily -- confirm the venue's real tick size before trusting fills.
constexpr Price kTickScale = 1000000;

std::uint64_t now_ns() {
    return static_cast<std::uint64_t>(
        std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count());
}

// "0.6123" -> 612300 at kTickScale, without ever touching a float.
Price to_ticks(std::string_view s) {
    Price whole = 0, frac = 0, scale = 1;
    std::size_t i = 0;
    bool neg = (i < s.size() && s[i] == '-');
    if (neg) ++i;

    for (; i < s.size() && s[i] != '.'; ++i) {
        if (s[i] < '0' || s[i] > '9') return 0;
        whole = whole * 10 + (s[i] - '0');
    }
    if (i < s.size() && s[i] == '.') {
        for (++i; i < s.size() && scale < kTickScale; ++i) {
            if (s[i] < '0' || s[i] > '9') break;
            frac = frac * 10 + (s[i] - '0');
            scale *= 10;
        }
    }
    Price ticks = whole * kTickScale + frac * (kTickScale / scale);
    return neg ? -ticks : ticks;
}

}  // namespace

struct PmJsonAdapter::Impl {
    simdjson::ondemand::parser parser;
    std::string padded;

    // Polymarket asset ids are ~78-character decimal strings. Interning them to
    // a uint32 keeps them off the hot path and out of MarketEvent.
    std::unordered_map<std::string, std::uint32_t> book_ids;
    std::uint32_t next_book_id = 1;

    std::uint32_t intern(std::string_view asset_id) {
        // C++20 would allow a heterogeneous lookup and skip this allocation.
        std::string key(asset_id);
        auto it = book_ids.find(key);
        if (it != book_ids.end()) return it->second;
        auto id = next_book_id++;
        book_ids.emplace(std::move(key), id);
        return id;
    }
};

PmJsonAdapter::PmJsonAdapter() : impl_(std::make_unique<Impl>()) {}
PmJsonAdapter::~PmJsonAdapter() = default;
PmJsonAdapter::PmJsonAdapter(PmJsonAdapter&&) noexcept = default;
PmJsonAdapter& PmJsonAdapter::operator=(PmJsonAdapter&&) noexcept = default;

void PmJsonAdapter::parse(std::string_view raw, std::vector<MarketEvent>& out) {
    out.clear();
    if (raw.empty()) return;

    // simdjson requires SIMDJSON_PADDING slack past the end of the document.
    impl_->padded.assign(raw.begin(), raw.end());
    impl_->padded.reserve(impl_->padded.size() + simdjson::SIMDJSON_PADDING);

    const std::uint64_t recv = now_ns();

    auto doc = impl_->parser.iterate(impl_->padded.data(),
                                     impl_->padded.size(),
                                     impl_->padded.capacity());

    // FILLER: assumes the frame is a top-level array of message objects.
    for (auto raw_item : doc.get_array()) {
        auto item = raw_item.value();

        std::string_view event_type;
        if (item["event_type"].get_string().get(event_type)) continue;

        MarketEvent e{};
        e.recv_ns = recv;

        std::string_view asset_id;
        if (!item["asset_id"].get_string().get(asset_id)) {
            e.book_id = impl_->intern(asset_id);
        }

        // FILLER: Polymarket sends timestamps as a millisecond string.
        std::string_view ts;
        if (!item["timestamp"].get_string().get(ts)) {
            e.ts_ns = static_cast<std::uint64_t>(std::stoull(std::string(ts))) * 1000000ull;
        }

        // FILLER: real event-type strings on the market channel are believed to
        // be "book", "price_change" and "last_trade_price". Confirm, then map
        // each onto the right EventType.
        if (event_type == "price_change") {
            e.type = EventType::Add;

            std::string_view side;
            if (!item["side"].get_string().get(side)) {
                e.side = (side == "BUY" || side == "buy") ? Side::Buy : Side::Sell;
            }

            std::string_view price;
            if (!item["price"].get_string().get(price)) {
                e.price = to_ticks(price);
            }

            std::string_view size;
            if (!item["size"].get_string().get(size)) {
                e.quantity = static_cast<Qty>(to_ticks(size));
            }

            out.push_back(e);
        }

        // TODO: "book" (full snapshot -> Clear + a run of Adds) and
        // "last_trade_price" once the real payloads are known.
    }
}

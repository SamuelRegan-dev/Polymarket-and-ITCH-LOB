// event.h

//This is where the shape of the data is held for itch and polymarket
//more accurately is decides what one market event looks like
#pragma once
#include <cstdint>

// ---------------------------------------------------------------------------
// The canonical event. PROVISIONAL -- placeholder shapes, to be finalised.
//
// Every adapter converts its venue's wire format into this struct, and the
// engine only ever sees this struct. That is what keeps the engine free of
// venue knowledge: adding a venue costs one adapter, not an edit here.
//
// CONTRACT NOTE: Cancel / Delete / Execute events carry only `order_id`.
// Neither ITCH nor Polymarket repeats price or side on those messages, so
// the engine resolves them from its own order index. Do not expect
// `price` / `side` to be populated except on Add and Replace.
// ---------------------------------------------------------------------------

using Price   = std::int64_t;   // integer ticks. never a float.
using Qty     = std::uint64_t;
using OrderId = std::uint64_t;

enum class Side      : std::uint8_t { Buy, Sell };
enum class EventType : std::uint8_t { Add, Cancel, Delete, Execute, Replace };

struct MarketEvent {
    std::uint64_t ts_ns;        // venue timestamp (see adapter for epoch basis)
    std::uint64_t recv_ns;      // local receive time, ns since unix epoch
    std::uint32_t book_id;      // interned instrument id
    OrderId       order_id;
    OrderId       new_order_id; // Replace only
    EventType     type;
    Side          side;         // Add / Replace only
    Price         price;        // Add / Replace only
    Qty           quantity;
};

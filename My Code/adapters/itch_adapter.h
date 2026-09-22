// adapters/itch_adapter.h
#pragma once

#include "../event.h"
#include <memory>
#include <string_view>
#include <vector>

// ---------------------------------------------------------------------------
// ItchBinaryAdapter -- NASDAQ TotalView-ITCH 5.0 -> MarketEvent
//
// All credit to the binary decoding goes solely to itchcpp:
//
//     itchcpp -- https://github.com/bbalouki/itchcpp
//     Copyright (c) bbalouki. MIT License.
//
// This class owns no parsing logic whatsoever. Its entire job is to translate
// itchcpp's message variant into our MarketEvent, so that nothing downstream
// depends on itchcpp's types. If itchcpp is ever swapped out, this file is
// the only one that changes. See THIRD_PARTY.md in the repo root.
// ---------------------------------------------------------------------------
class ItchBinaryAdapter {
public:
    ItchBinaryAdapter();
    ~ItchBinaryAdapter();
    ItchBinaryAdapter(ItchBinaryAdapter&&) noexcept;
    ItchBinaryAdapter& operator=(ItchBinaryAdapter&&) noexcept;

    // Clears `out`, then appends one MarketEvent per order-book message found
    // in `raw`. Administrative messages (stock directory, halts, NOII, ...)
    // are dropped.
    //
    // Please note that there is no incremental parser, so`raw` must contain only whole messages, therefore
    // a partial message at the tail isn't buffered into the next call.
    // Whoever feeds this must respect ITCH message boundaries.

    void parse(std::string_view raw, std::vector<MarketEvent>& out);

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

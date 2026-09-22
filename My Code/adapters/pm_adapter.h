// adapters/pm_adapter.h
#pragma once

#include "../event.h"
#include <memory>
#include <string_view>
#include <vector>

// ---------------------------------------------------------------------------
// PmJsonAdapter -- Polymarket CLOB market channel (JSON) -> MarketEvent
//
// No third-party Polymarket parser exists that is worth depending on as far as I'm aware.
//
// !! THE SCHEMA BELOW IS FILLER !!
// Field names, event-type strings and the tick scale are placeholders. Replace
// them once real frames have been captured off the live socket; Polymarket's
// docs do not fully specify the market channel payloads.
// ---------------------------------------------------------------------------
class PmJsonAdapter {
public:
    PmJsonAdapter();
    ~PmJsonAdapter();
    PmJsonAdapter(PmJsonAdapter&&) noexcept;
    PmJsonAdapter& operator=(PmJsonAdapter&&) noexcept;

    // Clears `out`, then appends one MarketEvent per book message in `raw`.
    // `raw` is expected to be one complete websocket text frame.
    void parse(std::string_view raw, std::vector<MarketEvent>& out);

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

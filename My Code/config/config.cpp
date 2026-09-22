// config.cpp
#include "config.h"
#include "args.h"

bool build_config(Args const& a, RuntimeConfig& rt, std::string& err) {
    if (a.venue == "pm") {
        rt.host              = "ws-subscriptions-clob.polymarket.com";
        rt.port              = "443";
        rt.target            = "/ws/market";
        rt.subscribe_message = R"({"assets_ids": [")" + a.asset + R"("], "type": "market"})";
        rt.kdb_host          = "localhost";
        rt.kdb_port          = 5001;
        rt.table_prefix      = "pm";
    } else if (a.venue == "itch") {
        rt.kdb_host          = "localhost";
        rt.kdb_port          = 5002;
        rt.table_prefix      = "itch";
    } else {
        err = "unknown venue: " + a.venue;
        return false;
    }

    rt.path = a.path;

    // per-mode requirements
    if (a.mode == "live" && a.venue == "pm" && a.asset.empty()) {
        err = "live pm requires --asset=<asset_id>";
        return false;
    }
    if (a.mode == "replay" && a.path.empty()) {
        err = "replay requires --path=<file>";
        return false;
    }
    return true;
}
// main.cpp

// run live pm: ./engine live pm --asset=11298791229263523343365534742786171091149101723089757434771061509810634434984
// run local itch: ./engine replay itch --path=data/itch_20260918.bin

#include "run_feed.h"
#include "feeds.h"
#include "args.h"
#include "config.h"
#include "env.h"
#include <iostream>

struct Route {
    char const* mode;
    char const* venue;
    void (*run)(RuntimeConfig const&);
};

static constexpr Route kRoutes[] = {
    { "live",   "pm",   &run_feed<PmLive>     },
    { "replay", "pm",   &run_feed<PmReplay>   },
    { "replay", "itch", &run_feed<ItchReplay> },
};

struct Args;
bool build_config(Args const& a, RuntimeConfig& rt, std::string& err);

int main(int argc, char** argv) {
    auto env = load_env(".env");
    //std::string ORDER_KEY = env.at("ORDER_KEY");
    //std::string LIVE_WEBSOCKET = env.at("LIVE_WEBSOCKET");
    //auto config = polymarket_market_feed("11298791229263523343365534742786171091149101723089757434771061509810634434984");

    Args args;
    std::string err;
    if (!parse_args(argc, argv, args, err)) { std::cerr << err << "\n"; return 1; }

    RuntimeConfig rt;
    if (!build_config(args, rt, err))       { std::cerr << err << "\n"; return 1; }

    for (auto const& r : kRoutes) {
        if (args.mode == r.mode && args.venue == r.venue) {
            r.run(rt);
            return 0;
        }
    }

    std::cerr << "unsupported combination: " << args.mode << " " << args.venue << "\nsupported:\n";
    for (auto const& r : kRoutes) std::cerr << "  " << r.mode << " " << r.venue << "\n";
    return 1;
}

/*
 * in the hypothetical where i could poll a bunch and edit them in and out live on the basis of say "market resolved nothing
 * left to poll", how would that work?
 * say i call with an array of 100 markets at once that are between 12-16 hours from resolution. then say i poll and another market comes in,
 * and i want to add a new one, it would need a new live request. i could cut the old connection every thirty minutes to refresh it,
 * but you're risking doing so during the tiny window where the resolution edge exists.
 * how many of these can i call at once? i could just call an entirely new batch every hour.
 * even better, given i know the exact hourly window of my edge, i can call only within that timeframe plus 30 minutes in both directions
 * and when that window ends the next 500 markets will be soon to enter it.
 * This strategy relies on capturing EVERY current market that is soon to enter the window. if im polling this group for 1 hour,
 * that means no new market can be entering the edge window until that hour ends. is that logical?
 * lets assume i have a window, and there is a market one microsecond outside it. by the time the batch window ends, how can i guarantee
 * that i capture that market?
 * pretend its 12-13 hours to close. last market is in 13 hours. an hour later the edge has been traded, grab a new market.
 *
 * you will need async for all of these markets in this batch. this could be insanely computationally expensive
 * every new batch there is an edge case where certain markets are in the window. given i know the time to resolution i can just take it
 * using the same algorithm though, so its fine.
 *
 * now for identifying markets: not all markets have a resolution.
 * In the hypothetical where all markets follow this trend, resolution or not, I will find the approximate shape first. i can calculate
 * that using a crawler that scans current market price and make an integral. if it matches the shape close to this window with an
 * edge, at it to the queue to be batched.
 *
 * I CAN AUTO SUBSCRIBE/UNSUBSCRIBE BASED ON THE WINDOW
 *
 * I will continue this when the full matching engine is built and ready. this may not even be orderbook territory.
 */

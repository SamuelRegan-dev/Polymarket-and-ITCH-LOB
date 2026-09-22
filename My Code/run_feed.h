// run_feed.h
#pragma once

#include "config/config.h"
#include "config/event.h"
#include <string_view>
#include <vector>

// The pipeline for this intended configuration. It just looks a lot cleaner than having it in main.
//
// Cfg names four types and holds nothing else:
//   Cfg::Transport   where bytes come from    (WebSocketTransport / FileTransport)
//   Cfg::Adapter     bytes -> MarketEvent     (PmJsonAdapter / ItchBinaryAdapter)
//   Cfg::Engine      MarketEvent -> Trade     (TimePriorityEngine<Sink>)
//   Cfg::Sink        Trade -> out of process  (KdbSink)
//
// Nothing below names a venue or a mode. Adding either leaves this file alone.
template <typename Cfg>
void run_feed(RuntimeConfig const& rt) {
    typename Cfg::Sink      sink{rt};
    typename Cfg::Engine    engine{sink};
    typename Cfg::Adapter   adapter;
    typename Cfg::Transport transport;

    std::vector<MarketEvent> events;
    events.reserve(1024);

    transport.run(rt, [&](std::string_view bytes) {
        adapter.parse(bytes, events);
        for (MarketEvent const& e : events) {
            engine.apply(e);
        }
    });
}

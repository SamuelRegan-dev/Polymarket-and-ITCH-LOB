// feeds.h

//this holds the preconfigured settings for the (to-be) four possible models
//I would prefer if this were implicit, as every new one is a new struct but who knows maybe its fine. i wont need other venues for a while
#pragma once

#include "transport/ws_transport.h"
#include "transport/file_transport.h"
#include "adapters/pm_adapter.h"
#include "adapters/itch_adapter.h"
#include "sinks/kdb_sink.h"
#include "Matching_Engine.h"

struct PmLive {
    using Transport = WebSocketTransport;
    using Adapter   = PmJsonAdapter;
    using Sink      = KdbSink;
    using Engine    = TimePriorityEngine<KdbSink>;
};

struct PmReplay {
    using Transport = FileTransport;
    using Adapter   = PmJsonAdapter;
    using Sink      = KdbSink;
    using Engine    = TimePriorityEngine<KdbSink>;
};

struct ItchReplay {
    using Transport = FileTransport;
    using Adapter   = ItchBinaryAdapter;
    using Sink      = KdbSink;
    using Engine    = TimePriorityEngine<KdbSink>;
};
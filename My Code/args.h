// Holds and parses argument struct! like that wasn't obvious from the name

#pragma once
#include <string>

struct Args {
    std::string mode;    // live | replay
    std::string venue;   // pm | itch
    std::string asset;   // --asset=...  (pm live)
    std::string path;    // --path=...   (replay)
};

bool parse_args(int argc, char** argv, Args& out, std::string& err);
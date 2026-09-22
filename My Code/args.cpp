/*
* args.cpp exists so you can call the specific format you want to process (itch, polymarket / live, local)
 * */
#include "args.h"
#include <cstring>

static bool take_flag(char const* arg, char const* name, std::string& out) {
    auto n = std::strlen(name);
    if (std::strncmp(arg, name, n) != 0) return false;
    out = arg + n;
    return true;
}

bool parse_args(int argc, char** argv, Args& out, std::string& err) {
    int positional = 0;
    for (int i = 1; i < argc; ++i) {
        char const* a = argv[i];
        if (a[0] == '-') {
            if (take_flag(a, "--asset=", out.asset)) continue;
            if (take_flag(a, "--path=",  out.path))  continue;
            err = std::string("unknown flag: ") + a;
            return false;
        }
        if (positional == 0)      out.mode  = a;
        else if (positional == 1) out.venue = a;
        else { err = std::string("unexpected argument: ") + a; return false; }
        ++positional;
    }
    if (positional < 2) { err = "usage: engine <live|replay> <pm|itch> [--asset=ID] [--path=FILE]"; return false; }
    return true;
}
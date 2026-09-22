/* config is the config for the runtime settings, such as:
*  - host
*  - port
*  - file path
*  - etc.
*/


#pragma once
#include <string>

struct RuntimeConfig {
    // live transport
    std::string host;
    std::string port;
    std::string target;
    std::string subscribe_message;

    // replay transport
    std::string path;

    // sink
    std::string kdb_host;
    int         kdb_port = 0;
    std::string table_prefix;
};

struct Args;
bool build_config(Args const& a, RuntimeConfig& rt, std::string& err);

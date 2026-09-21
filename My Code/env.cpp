//
// Created by samue on 04/09/2026.
//

// includes files I/O string handling

#include "env.h"
#include <fstream>
#include <cstdlib>

std::unordered_map<std::string, std::string> load_env(const std::string& path) {
    std::unordered_map<std::string,std::string> values;

    std::ifstream file(path);
    if (!file.is_open()) {
        return values;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        if (line.empty() || line[0] == '#') {
            continue;
        }

        size_t eq = line.find('=');
        if (eq == std::string::npos) {
            continue;
        }

        std::string key = line.substr(0, eq);
        std::string value = line.substr(eq + 1);

        values[key] = value;
        setenv(key.c_str(), value.c_str(), 1);
    }

    return values;
}
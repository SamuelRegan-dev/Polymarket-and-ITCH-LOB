#pragma once
#include <string>
#include <unordered_map>

std::unordered_map<std::string, std::string> load_env(const std::string& path = ".env");
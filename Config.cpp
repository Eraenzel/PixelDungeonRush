#include "Config.hpp"
#include <fstream>
#include <iostream>
#include <sstream>

Config& Config::instance() {
    static Config inst;
    return inst;
}

bool Config::loadFromFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open config file: " << path << "\n";
        std::cerr << "Using default values. Please ensure 'assets/config.json' exists.\n";
        return false;
    }

    try {
        file >> config;
        loaded = true;
        std::cout << "Config loaded successfully from: " << path << "\n";
        return true;
    }
    catch (const json::exception& e) {
        std::cerr << "Error: Failed to parse config JSON: " << e.what() << "\n";
        return false;
    }
}

json* Config::navigate(const std::string& key) {
    if (!loaded) return nullptr;

    // Split key by dots (e.g., "gameplay.combat.attackRadius")
    std::istringstream iss(key);
    std::string part;
    json* current = &config;

    while (std::getline(iss, part, '.')) {
        if (current->is_object() && current->contains(part)) {
            current = &(*current)[part];
        }
        else {
            return nullptr; // Key not found
        }
    }

    return current;
}

const json* Config::navigate(const std::string& key) const {
    if (!loaded) return nullptr;

    std::istringstream iss(key);
    std::string part;
    const json* current = &config;

    while (std::getline(iss, part, '.')) {
        if (current->is_object() && current->contains(part)) {
            current = &(*current)[part];
        }
        else {
            return nullptr;
        }
    }

    return current;
}

float Config::getFloat(const std::string& key, float defaultValue) {
    auto ptr = navigate(key);
    if (!ptr || !ptr->is_number()) {
        return defaultValue;
    }
    return ptr->get<float>();
}

int Config::getInt(const std::string& key, int defaultValue) {
    auto ptr = navigate(key);
    if (!ptr || !ptr->is_number()) {
        return defaultValue;
    }
    return ptr->get<int>();
}

bool Config::getBool(const std::string& key, bool defaultValue) {
    auto ptr = navigate(key);
    if (!ptr || !ptr->is_boolean()) {
        return defaultValue;
    }
    return ptr->get<bool>();
}

std::string Config::getString(const std::string& key, const std::string& defaultValue) {
    auto ptr = navigate(key);
    if (!ptr || !ptr->is_string()) {
        return defaultValue;
    }
    return ptr->get<std::string>();
}

std::optional<json> Config::getArray(const std::string& key) {
    auto ptr = navigate(key);
    if (!ptr || !ptr->is_array()) {
        return std::nullopt;
    }
    return *ptr;
}

std::optional<json> Config::getObject(const std::string& key) {
    auto ptr = navigate(key);
    if (!ptr || !ptr->is_object()) {
        return std::nullopt;
    }
    return *ptr;
}

bool Config::hasKey(const std::string& key) {
    return navigate(key) != nullptr;
}

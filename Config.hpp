#pragma once
#include <string>
#include <nlohmann/json.hpp>
#include <optional>

using json = nlohmann::json;

/// Singleton configuration manager for loading and accessing game settings
/// Usage: Config::instance().getFloat("gameplay.combat.attackRadius")
class Config {
public:
    static Config& instance();

    /// Load configuration from JSON file
    bool loadFromFile(const std::string& path);

    /// Get float value with default fallback
    float getFloat(const std::string& key, float defaultValue = 0.0f);

    /// Get int value with default fallback
    int getInt(const std::string& key, int defaultValue = 0);

    /// Get bool value with default fallback
    bool getBool(const std::string& key, bool defaultValue = false);

    /// Get string value with default fallback
    std::string getString(const std::string& key, const std::string& defaultValue = "");

    /// Get JSON array for iteration (e.g., "gameplay.loot.drops")
    std::optional<json> getArray(const std::string& key);

    /// Get entire JSON object for a section
    std::optional<json> getObject(const std::string& key);

    /// Check if key exists
    bool hasKey(const std::string& key);

private:
    Config() = default;
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;

    json config;
    bool loaded = false;

    // Helper to navigate nested keys (e.g., "gameplay.combat.attackRadius")
    json* navigate(const std::string& key);
    const json* navigate(const std::string& key) const;
};

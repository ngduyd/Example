#include "ConfigManager.h"

ConfigManager::ConfigManager(const char* ns) : nvsNamespace(ns) {
    resetToDefault();
}

uint32_t ConfigManager::calcChecksum(const ConfigData &cfg) const {
    const uint8_t *ptr = reinterpret_cast<const uint8_t*>(&cfg);
    uint32_t sum = 0;
    for (size_t i = 0; i < sizeof(ConfigData) - sizeof(uint32_t); i++)
        sum += ptr[i];
    return sum;
}

bool ConfigManager::load() {
    prefs.begin(nvsNamespace, true); // read-only
    if (!prefs.isKey("cfg")) {
        prefs.end();
        Serial.println("[NVS] No config found");
        return false;
    }

    prefs.getBytes("cfg", &data, sizeof(ConfigData));
    prefs.end();

    if (!isValid()) {
        Serial.println("[NVS] Invalid checksum -> reset to default");
        resetToDefault();
        return false;
    }

    Serial.println("[NVS] Config loaded successfully");
    return true;
}

bool ConfigManager::save() {
    data.checksum = calcChecksum(data);

    prefs.begin(nvsNamespace, false);
    prefs.putBytes("cfg", &data, sizeof(ConfigData));
    prefs.end();

    Serial.println("[NVS] Config saved successfully");
    return true;
}

void ConfigManager::clear() {
    prefs.begin(nvsNamespace, false);
    prefs.clear();
    prefs.end();
    Serial.println("[NVS] Config cleared");
}

void ConfigManager::resetToDefault() {
    memset(&data, 0, sizeof(data));
    strcpy(data.ssid, "ABC");
    strcpy(data.pass, "12345678");
    strcpy(data.mqttHost, "192.168.1.100");
    strcpy(data.status, "online");
    data.mqttPort = 1883;
    data.checksum = calcChecksum(data);
}

bool ConfigManager::isValid() const {
    return data.checksum == calcChecksum(data);
}

#ifndef APSettingsConfig_h
#define APSettingsConfig_h

#include <HttpEndpoint.h>
#include <FSPersistence.h>

#include <DNSServer.h>
#include <IPAddress.h>

#define MANAGE_NETWORK_DELAY 10000

#define AP_MODE_ALWAYS 0
#define AP_MODE_DISCONNECTED 1
#define AP_MODE_NEVER 2

#define DNS_PORT 53

#ifndef FACTORY_AP_SSID
#define FACTORY_AP_SSID "ESP8266-React"
#endif

#ifndef FACTORY_AP_PASSWORD
#define FACTORY_AP_PASSWORD "esp-react"
#endif

#ifndef FACTORY_AP_PROVISION_MODE
#define FACTORY_AP_PROVISION_MODE AP_MODE_DISCONNECTED
#endif

#define AP_SETTINGS_FILE "/config/apSettings.json"
#define AP_SETTINGS_SERVICE_PATH "/rest/apSettings"

enum APNetworkStatus {
  ACTIVE = 0,
  INACTIVE,
  LINGERING
};

class APSettings {
 public:
  uint8_t provisionMode;
  String ssid;
  String password;

  static void read(APSettings& settings, JsonObject& root) {
    root["provision_mode"] = settings.provisionMode;
    root["ssid"] = settings.ssid;
    root["password"] = settings.password;
  }

  static StateUpdateResult update(JsonObject& root, APSettings& settings) {
    APSettings newSettings = {};
    newSettings.provisionMode = root["provision_mode"] | FACTORY_AP_PROVISION_MODE;
    switch (settings.provisionMode) {
      case AP_MODE_ALWAYS:
      case AP_MODE_DISCONNECTED:
      case AP_MODE_NEVER:
        break;
      default:
        newSettings.provisionMode = AP_MODE_ALWAYS;
    }
    newSettings.ssid = root["ssid"] | FACTORY_AP_SSID;
    newSettings.password = root["password"] | FACTORY_AP_PASSWORD;
    if (newSettings.provisionMode == settings.provisionMode && newSettings.ssid.equals(settings.ssid) &&
        newSettings.password.equals(settings.password)) {
      return StateUpdateResult::UNCHANGED;
    }
    settings = newSettings;
    return StateUpdateResult::CHANGED;
  }
};

class APSettingsService : public StatefulService<APSettings> {
 public:
  APSettingsService(AsyncWebServer* server, FS* fs, SecurityManager* securityManager);

  void begin();
  void loop();
  APNetworkStatus getAPNetworkStatus();

 private:
  HttpEndpoint<APSettings> _httpEndpoint;
  FSPersistence<APSettings> _fsPersistence;

  // for the captive portal
  DNSServer* _dnsServer;

  // for the mangement delay loop
  volatile unsigned long _lastManaged;
  volatile boolean _reconfigureAp;

  void reconfigureAP();
  void manageAP();
  void startAP();
  void stopAP();
  void handleDNS();
};

#endif  // end APSettingsConfig_h
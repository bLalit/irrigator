#include "UnoR4WiFi.hpp"

#include <WiFiS3.h>

namespace UnoR4 {
namespace Wifi {

namespace {

bool credentialsAreValid(const NetworkCredentials& credentials) {
  if (credentials.ssid == nullptr || credentials.ssid[0] == '\0') {
    return false;
  }

  if (credentials.password == nullptr) {
    return true;
  }

  return credentials.password[0] == '\0' ||
         (strlen(credentials.password) >= 8 && strlen(credentials.password) <= 63);
}

ConnectionResult connectionResultFromStatus(uint8_t status) {
  switch (status) {
    case WL_CONNECTED:
      return ConnectionResult::Connected;
    case WL_CONNECT_FAILED:
      return ConnectionResult::AuthenticationFailed;
    case WL_NO_SSID_AVAIL:
      return ConnectionResult::NetworkNotFound;
    case WL_CONNECTION_LOST:
      return ConnectionResult::ConnectionLost;
    case WL_NO_SHIELD:
      return ConnectionResult::NoModule;
    case WL_DISCONNECTED:
    default:
      return ConnectionResult::ConnectionLost;
  }
}

}  // namespace

Module::Module(const NetworkCredentials& credentials,
               uint32_t connectionTimeoutMilliseconds,
               uint32_t retryIntervalMilliseconds)
    : credentials_(credentials),
      connectionTimeoutMilliseconds_(connectionTimeoutMilliseconds),
      retryIntervalMilliseconds_(retryIntervalMilliseconds),
      lastReconnectAttemptMilliseconds_(0UL),
      lastConnectionCheckMilliseconds_(0UL),
      mode_(NetworkMode::Off) {
}

bool Module::initialize() {
  return WiFi.status() != WL_NO_SHIELD &&
         WiFi.firmwareVersion() != nullptr;
}

ConnectionResult Module::connect() {
  if (!credentialsAreValid(credentials_)) {
    return ConnectionResult::InvalidConfiguration;
  }

  if (!initialize()) {
    return ConnectionResult::NoModule;
  }

  if (credentials_.password == nullptr || credentials_.password[0] == '\0') {
    WiFi.begin(credentials_.ssid);
  } else {
    WiFi.begin(credentials_.ssid, credentials_.password);
  }

  mode_ = NetworkMode::Station;
  return ConnectionResult::Started;
}

ConnectionResult Module::waitUntilConnected() {
  const uint32_t startedAt = millis();

  while (WiFi.status() != WL_CONNECTED) {
    const uint32_t elapsed = millis() - startedAt;
    if (elapsed >= connectionTimeoutMilliseconds_) {
      return ConnectionResult::TimedOut;
    }

    const uint8_t status = WiFi.status();
    if (status == WL_CONNECT_FAILED ||
        status == WL_NO_SSID_AVAIL ||
        status == WL_NO_SHIELD) {
      return connectionResultFromStatus(status);
    }

    delay(100);
  }

  return ConnectionResult::Connected;
}

ConnectionResult Module::connectAndPrintStatus() {
  Serial.println("Uno R4 WiFi starting");

  if (!initialize()) {
    Serial.println("Wi-Fi module or firmware unavailable");
    return ConnectionResult::NoModule;
  }

  Serial.print("Connecting to ");
  Serial.println(credentials_.ssid);

  const ConnectionResult connectionStarted = connect();
  if (connectionStarted != ConnectionResult::Started) {
    Serial.print("Wi-Fi configuration error: ");
    Serial.println(static_cast<int>(connectionStarted));
    return connectionStarted;
  }

  const ConnectionResult connectionResult = waitUntilConnected();
  if (connectionResult == ConnectionResult::Connected) {
    const NetworkDetails networkDetails = details();
    Serial.println("Wi-Fi connected");
    Serial.print("IP address: ");
    Serial.println(networkDetails.localAddress);
    Serial.print("Signal strength: ");
    Serial.print(networkDetails.signalStrengthDbm);
    Serial.println(" dBm");
  } else {
    Serial.print("Wi-Fi connection failed: ");
    Serial.println(static_cast<int>(connectionResult));
  }

  return connectionResult;
}

void Module::maintainConnection() {
  const uint32_t now = millis();

  if (now - lastConnectionCheckMilliseconds() >= 500UL) {
    lastConnectionCheckMilliseconds_ = now;
    if (mode_ == NetworkMode::Station && !isConnected()) {
      if (now - lastReconnectAttemptMilliseconds_ >= retryIntervalMilliseconds_) {
        lastReconnectAttemptMilliseconds_ = now;
        connect();
      }
    }
  }
}

void Module::disconnect() {
  WiFi.disconnect();
  mode_ = NetworkMode::Off;
}

bool Module::isConnected() const {
  return WiFi.status() == WL_CONNECTED;
}

bool Module::isFirmwareAvailable() const {
  return WiFi.firmwareVersion() != nullptr;
}

uint8_t Module::statusCode() const {
  return WiFi.status();
}

const char* Module::statusDescription() const {
  switch (WiFi.status()) {
    case WL_IDLE_STATUS:
      return "Idle";
    case WL_NO_SSID_AVAIL:
      return "Network not found";
    case WL_SCAN_COMPLETED:
      return "Scan completed";
    case WL_CONNECTED:
      return "Connected";
    case WL_CONNECT_FAILED:
      return "Authentication failed";
    case WL_CONNECTION_LOST:
      return "Connection lost";
    case WL_DISCONNECTED:
      return "Disconnected";
    case WL_AP_LISTENING:
      return "Access point listening";
    case WL_AP_CONNECTED:
      return "Client connected";
    case WL_AP_FAILED:
      return "Access point failed";
    case WL_NO_SHIELD:
    default:
      return "Wi-Fi module not available";
  }
}

NetworkMode Module::mode() const {
  return mode_;
}

String Module::firmwareVersion() const {
  const char* version = WiFi.firmwareVersion();
  return version == nullptr ? String() : String(version);
}

NetworkDetails Module::details() const {
  NetworkDetails details{};
  WiFi.macAddress(details.macAddress);
  details.ssid = isConnected() ? WiFi.SSID() : String();
  details.localAddress = WiFi.localIP();
  details.gatewayAddress = WiFi.gatewayIP();
  details.subnetMask = WiFi.subnetMask();
  details.primaryDnsAddress = WiFi.dnsIP();
  details.signalStrengthDbm = isConnected() ? WiFi.RSSI() : 0;
  return details;
}

AccessPoint Module::scannedNetwork(uint8_t index) const {
  AccessPoint network{};
  const char* ssid = WiFi.SSID(index);
  network.ssid = ssid == nullptr ? String() : String(ssid);
  network.signalStrengthDbm = WiFi.RSSI(index);
  network.channel = WiFi.channel(index);
  network.encryptionType = WiFi.encryptionType(index);
  return network;
}

int8_t Module::scanForNetworks(bool showHiddenNetworks) {
  (void)showHiddenNetworks;
  return WiFi.scanNetworks();
}

void Module::delayWithConnectionMaintenance(
    uint32_t durationMilliseconds) {
  const uint32_t endTime = millis() + durationMilliseconds;
  while (static_cast<int32_t>(millis() - endTime) < 0) {
    maintainConnection();
    delay(50);
  }
}

bool Module::startAccessPoint(const char* ssid,
                              const char* password,
                              uint8_t channel) {
  if (ssid == nullptr || ssid[0] == '\0' || channel == 0 || channel > 14) {
    return false;
  }

  uint8_t result;
  if (password == nullptr || password[0] == '\0') {
    result = WiFi.beginAP(ssid, channel);
  } else {
    if (strlen(password) < 8 || strlen(password) > 63) {
      return false;
    }
    result = WiFi.beginAP(ssid, password, channel);
  }

  mode_ = NetworkMode::AccessPoint;
  return result == WL_AP_LISTENING;
}

void Module::stopAccessPoint() {
  WiFi.disconnect();
  mode_ = NetworkMode::Off;
}

bool Module::configureStaticAddress(const IPAddress& localAddress,
                                    const IPAddress& gatewayAddress,
                                    const IPAddress& subnetMask,
                                    const IPAddress& primaryDnsAddress) {
  WiFi.config(localAddress, primaryDnsAddress, gatewayAddress, subnetMask);
  return true;
}

uint32_t Module::lastConnectionCheckMilliseconds() const {
  return lastConnectionCheckMilliseconds_;
}

}  // namespace Wifi
}  // namespace UnoR4

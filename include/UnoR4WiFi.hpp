#pragma once

#include <Arduino.h>

#include <stdint.h>

namespace UnoR4 {
namespace Wifi {

enum class ConnectionResult : uint8_t {
  Started,
  Connected,
  NoModule,
  NetworkNotFound,
  AuthenticationFailed,
  ConnectionLost,
  TimedOut,
  InvalidConfiguration,
};

enum class NetworkMode : uint8_t {
  Station,
  AccessPoint,
  Off,
};

struct NetworkCredentials {
  const char* ssid;
  const char* password;
};

struct NetworkDetails {
  String ssid;
  IPAddress localAddress;
  IPAddress gatewayAddress;
  IPAddress subnetMask;
  IPAddress primaryDnsAddress;
  int32_t signalStrengthDbm;
  uint8_t macAddress[6];
};

struct AccessPoint {
  String ssid;
  int32_t signalStrengthDbm;
  uint8_t channel;
  uint8_t encryptionType;
};

class Module {
 public:
  Module(const NetworkCredentials& credentials,
         uint32_t connectionTimeoutMilliseconds = 15000UL,
         uint32_t retryIntervalMilliseconds = 10000UL);

  bool initialize();
  ConnectionResult connect();
  ConnectionResult waitUntilConnected();
  ConnectionResult connectAndPrintStatus();
  void maintainConnection();
  void disconnect();

  bool isConnected() const;
  bool isFirmwareAvailable() const;
  uint8_t statusCode() const;
  const char* statusDescription() const;
  NetworkMode mode() const;

  String firmwareVersion() const;
  NetworkDetails details() const;
  AccessPoint scannedNetwork(uint8_t index) const;
  int8_t scanForNetworks(bool showHiddenNetworks = false);
  void delayWithConnectionMaintenance(uint32_t durationMilliseconds);

  bool startAccessPoint(const char* ssid,
                        const char* password = nullptr,
                        uint8_t channel = 1);
  void stopAccessPoint();

  bool configureStaticAddress(const IPAddress& localAddress,
                              const IPAddress& gatewayAddress,
                              const IPAddress& subnetMask,
                              const IPAddress& primaryDnsAddress);

 private:
  uint32_t lastConnectionCheckMilliseconds() const;

  NetworkCredentials credentials_;
  uint32_t connectionTimeoutMilliseconds_;
  uint32_t retryIntervalMilliseconds_;
  uint32_t lastReconnectAttemptMilliseconds_;
  uint32_t lastConnectionCheckMilliseconds_;
  NetworkMode mode_;
};

}  // namespace Wifi
}  // namespace UnoR4

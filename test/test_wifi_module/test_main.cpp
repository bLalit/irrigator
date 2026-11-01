#include <Arduino.h>
#include <unity.h>

#include "UnoR4WiFi.hpp"
#include "../../src/UnoR4WiFi.cpp"

namespace {

using UnoR4::Wifi::NetworkCredentials;
using UnoR4::Wifi::Module;
using UnoR4::Wifi::ConnectionResult;
using UnoR4::Wifi::NetworkMode;

void test_empty_ssid_is_rejected() {
  NetworkCredentials credentials{"", "password"};
  Module wifi(credentials);

  TEST_ASSERT_EQUAL(static_cast<int>(ConnectionResult::InvalidConfiguration),
                    static_cast<int>(wifi.connect()));
}

void test_short_password_is_rejected() {
  NetworkCredentials credentials{"TestNetwork", "short"};
  Module wifi(credentials);

  TEST_ASSERT_EQUAL(static_cast<int>(ConnectionResult::InvalidConfiguration),
                    static_cast<int>(wifi.connect()));
}

void test_password_longer_than_63_characters_is_rejected() {
  char password[65];
  memset(password, 'x', sizeof(password) - 1);
  password[sizeof(password) - 1] = '\0';

  NetworkCredentials credentials{"TestNetwork", password};
  Module wifi(credentials);

  TEST_ASSERT_EQUAL(static_cast<int>(ConnectionResult::InvalidConfiguration),
                    static_cast<int>(wifi.connect()));
}

void test_open_network_credentials_are_accepted() {
  NetworkCredentials credentials{"TestNetwork", ""};
  Module wifi(credentials);

  TEST_ASSERT_EQUAL(static_cast<int>(ConnectionResult::Started),
                    static_cast<int>(wifi.connect()));
}

void test_null_password_is_accepted() {
  NetworkCredentials credentials{"TestNetwork", nullptr};
  Module wifi(credentials);

  TEST_ASSERT_EQUAL(static_cast<int>(ConnectionResult::Started),
                    static_cast<int>(wifi.connect()));
}

void test_disconnected_wifi_has_off_mode() {
  NetworkCredentials credentials{"", ""};
  Module wifi(credentials);
  wifi.disconnect();

  TEST_ASSERT_EQUAL(static_cast<int>(NetworkMode::Off),
                    static_cast<int>(wifi.mode()));
}

void test_invalid_access_point_configuration_is_rejected() {
  NetworkCredentials credentials{"", ""};
  Module wifi(credentials);

  TEST_ASSERT_FALSE(wifi.startAccessPoint("", nullptr, 1));
  TEST_ASSERT_FALSE(wifi.startAccessPoint("TestAP", nullptr, 0));
  TEST_ASSERT_FALSE(wifi.startAccessPoint("TestAP", "short", 1));
  TEST_ASSERT_FALSE(wifi.startAccessPoint("TestAP", nullptr, 15));
}

void test_network_details_are_safe_when_disconnected() {
  NetworkCredentials credentials{"", ""};
  Module wifi(credentials);
  wifi.disconnect();

  const auto details = wifi.details();

  TEST_ASSERT_EQUAL_STRING("", details.ssid.c_str());
  TEST_ASSERT_EQUAL(0, details.signalStrengthDbm);
  TEST_ASSERT_EQUAL(6, sizeof(details.macAddress));
}

void test_firmware_information_is_available() {
  NetworkCredentials credentials{"", ""};
  Module wifi(credentials);

  TEST_ASSERT_TRUE(wifi.initialize());
  TEST_ASSERT_TRUE(wifi.isFirmwareAvailable());
  TEST_ASSERT_FALSE(wifi.firmwareVersion().isEmpty());
}

}  // namespace

void setup() {
  delay(2000);
  UNITY_BEGIN();
  RUN_TEST(test_empty_ssid_is_rejected);
  RUN_TEST(test_short_password_is_rejected);
  RUN_TEST(test_password_longer_than_63_characters_is_rejected);
  RUN_TEST(test_open_network_credentials_are_accepted);
  RUN_TEST(test_null_password_is_accepted);
  RUN_TEST(test_disconnected_wifi_has_off_mode);
  RUN_TEST(test_invalid_access_point_configuration_is_rejected);
  RUN_TEST(test_network_details_are_safe_when_disconnected);
  RUN_TEST(test_firmware_information_is_available);
  UNITY_END();
}

void loop() {
}

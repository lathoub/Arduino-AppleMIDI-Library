#include <ETH.h>
#include <SPI.h>
#include <ESPmDNS.h>

#ifndef ETH_PHY_CS
#define ETH_PHY_TYPE ETH_PHY_W5500
#define ETH_PHY_ADDR 1
#define ETH_PHY_CS SS
#define ETH_PHY_IRQ 4
#define ETH_PHY_RST 15
#endif

const char* mdnsName = "myesp32";

bool isETHconnected = false;

std::function<void(bool)> cbfunc;

void onEvent(arduino_event_id_t event, arduino_event_info_t info) {
  switch (event) {
    case ARDUINO_EVENT_ETH_START:
      AM_DBG("ETH Started");
      // set eth hostname here
      ETH.setHostname("esp32-eth0");
      break;
    case ARDUINO_EVENT_ETH_CONNECTED:
      AM_DBG("ETH Connecting");
      break;
    case ARDUINO_EVENT_ETH_GOT_IP:
      AM_DBG("ETH Connected");
      Serial.println(ETH);

      MDNS.begin(mdnsName);
      MDNS.addService("_apple-midi", "_udp", 5004);
      AM_DBG("MDNS started");

      isETHconnected = true;
      AM_DBG("isETHconnected", isETHconnected);

      cbfunc(true);
      break;
    case ARDUINO_EVENT_ETH_LOST_IP:
      AM_DBG("ETH Lost IP");
      isETHconnected = false;
      break;
    case ARDUINO_EVENT_ETH_DISCONNECTED:
      AM_DBG("ETH Disconnected");
      isETHconnected = false;
      break;
    case ARDUINO_EVENT_ETH_STOP:
      AM_DBG("ETH Stopped");
      isETHconnected = false;
      break;
    default:
      break;
  }
}

bool ETH_startup(std::function<void(bool)> callback) {
  cbfunc = callback;
  Network.onEvent(onEvent);

  SPI.begin();
  if (!ETH.begin(ETH_PHY_TYPE, ETH_PHY_ADDR, ETH_PHY_CS, ETH_PHY_IRQ, ETH_PHY_RST, SPI))
    return false;

  return true;
}

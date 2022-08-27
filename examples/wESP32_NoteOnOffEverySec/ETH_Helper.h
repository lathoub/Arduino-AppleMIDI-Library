#include <ETH.h>
#include <ESPmDNS.h>

static bool eth_connected = false;

void WiFiEvent(WiFiEvent_t event)
{
  switch (event) {
    case SYSTEM_EVENT_ETH_START:
      AM_DBG(F("ETH Started"));
      //set eth hostname here
      ETH.setHostname("esp32-ethernet");
      break;
    case SYSTEM_EVENT_ETH_CONNECTED:
      AM_DBG(F("ETH Connected"));
      break;
    case SYSTEM_EVENT_ETH_GOT_IP:
      AM_DBG(F("ETH MAC:"), ETH.macAddress(), F("IPv4:"), ETH.localIP(), ETH.linkSpeed(), F("Mbps"));
      if (ETH.fullDuplex())
              AM_DBG(F("FULL_DUPLEX"));
      eth_connected = true;
      break;
    case SYSTEM_EVENT_ETH_DISCONNECTED:
      AM_DBG(F("ETH Disconnected"));
      eth_connected = false;
      break;
    case SYSTEM_EVENT_ETH_STOP:
      AM_DBG(F("ETH Stopped"));
      eth_connected = false;
      break;
    default:
      break;
  }
}

bool ETH_startup()
{
  WiFi.onEvent(WiFiEvent);
  ETH.begin();

  AM_DBG(F("Getting IP address..."));

  while (!eth_connected)
    delay(100);

  return true;
}

#include <ETH.h>
#include <ESPmDNS.h>

static bool eth_connected = false;

void WiFiEvent(WiFiEvent_t event)
{
  switch (event) {
    case SYSTEM_EVENT_ETH_START:
      DBG("ETH Started");
      //set eth hostname here
      ETH.setHostname("esp32-ethernet");
      break;
    case SYSTEM_EVENT_ETH_CONNECTED:
      DBG("ETH Connected");
      break;
    case SYSTEM_EVENT_ETH_GOT_IP:
      DBG("ETH MAC:", ETH.macAddress(), "IPv4:", ETH.localIP(), ETH.linkSpeed(), "Mbps");
      if (ETH.fullDuplex())
              DBG("FULL_DUPLEX");
      eth_connected = true;
      break;
    case SYSTEM_EVENT_ETH_DISCONNECTED:
      DBG("ETH Disconnected");
      eth_connected = false;
      break;
    case SYSTEM_EVENT_ETH_STOP:
      DBG("ETH Stopped");
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

  DBG(F("Getting IP address..."));

  while (!eth_connected)
    delay(100);

  return true;
}

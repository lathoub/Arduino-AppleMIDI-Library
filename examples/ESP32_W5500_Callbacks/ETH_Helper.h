#include <Ethernet.h>
#include <EthernetBonjour.h> // https://github.com/TrippyLighting/EthernetBonjour
#include <string>

// to get the Mac address
#include "esp_mac.h"

#define RESET_PIN  26
#define CS_PIN     5

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};

const char* mdnsAppleMIDI = "._apple-midi";

/*
   Wiz W5500 reset function.  Change this for the specific reset
   sequence required for your particular board or module.
*/
void hardreset() {
  pinMode(RESET_PIN, OUTPUT);
  digitalWrite(RESET_PIN, HIGH);
  delay(150);

  digitalWrite(RESET_PIN, LOW);
  delay(500);
  digitalWrite(RESET_PIN, HIGH);
  delay(150);
}

bool ETH_startup()
{
  Ethernet.init(CS_PIN);
  AM_DBG("Resetting Wiz Ethernet Board...  ");
  hardreset();

  // Borrow MAC from the ESP32 wifi 
  esp_read_mac(mac, ESP_MAC_WIFI_STA);

  /*
      Network configuration - all except the MAC are optional.

      IMPORTANT NOTE - The mass-produced W5500 boards do -not-
                       have a built-in MAC address (depite
                       comments to the contrary elsewhere). You
                       -must- supply a MAC address here.
  */
  AM_DBG("Starting Ethernet connection...");

  Ethernet.begin(mac);
  AM_DBG("Ethernet IP is:", Ethernet.localIP());

  /*
     Sanity checks for W5500 and cable connection.
  */
  AM_DBG("Checking connection.");
  bool rdy_flag = false;
  for (uint8_t i = 0; i <= 20; i++) {
    if ((Ethernet.linkStatus() == Unknown)) {
      Serial.print(".");
      rdy_flag = false;
      delay(80);
    } else {
      rdy_flag = true;
      break;
    }
  }
  if (rdy_flag == false) {
    AM_DBG("\n\r\tHardware fault, or cable problem... cannot continue.");
    while (true) {
      delay(10);          // Halt.
    }
  } else {
    AM_DBG("OK");
  }

  return true;
}

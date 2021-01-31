#include <WiFi.h>
#include <Ethernet3.h>
//#include "AsyncUDP_Facade.h"

#define RESET_PIN  26
#define CS_PIN     5

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};

/*
   Wiz W5500 reset function.  Change this for the specific reset
   sequence required for your particular board or module.
*/
void WizReset() {
  Serial.print("Resetting Wiz W5500 Ethernet Board...  ");
  pinMode(RESET_PIN, OUTPUT);
  digitalWrite(RESET_PIN, HIGH);
  delay(250);
  digitalWrite(RESET_PIN, LOW);
  delay(50);
  digitalWrite(RESET_PIN, HIGH);
  delay(350);
  Serial.println("Done.");
}

/*
   Print the result of the ethernet connection
   status enum as a string.
   Ethernet.h currently contains these values:-

    enum EthernetLinkStatus {
       Unknown,
       LinkON,
       LinkOFF
    };
*/
void prt_ethval(uint8_t refval) {
  switch (refval) {
    case 0:
      Serial.println("Uknown status.");
      break;
    case 1:
      Serial.println("Link flagged as UP.");
      break;
    case 2:
      Serial.println("Link flagged as DOWN. Check cable connection.");
      break;
    default:
      Serial.println("UNKNOWN - Update espnow_gw.ino to match Ethernet.h");
  }
}

bool ETH_startup()
{
  // Use Ethernet.init(pin) to configure the CS pin.
  Ethernet.setRstPin(RESET_PIN);
  Ethernet.setCsPin(CS_PIN);
  Ethernet.init(4); // maxSockNum = 4 Socket 0...3 -> RX/TX Buffer 4k
  Serial.println("Resetting Wiz W5500 Ethernet Board...  ");
  Ethernet.hardreset();
  //  WizReset();

  Serial.println(WiFi.macAddress());
  esp_read_mac(mac, ESP_MAC_WIFI_STA);

  /*
      Network configuration - all except the MAC are optional.

      IMPORTANT NOTE - The mass-produced W5500 boards do -not-
                       have a built-in MAC address (depite
                       comments to the contrary elsewhere). You
                       -must- supply a MAC address here.
  */
  Serial.println("Starting ETHERNET connection...");
  Ethernet.begin(mac);

  Serial.print("Ethernet IP is: ");
  Serial.println(Ethernet.localIP());

  /*
     Sanity checks for W5500 and cable connection.
  */
  Serial.println("Checking connection.");
  bool rdy_flag = false;
  for (uint8_t i = 0; i <= 20; i++) {
    if ((Ethernet.link() == 0)) {
      Serial.print(".");
      rdy_flag = false;
      delay(80);
    } else {
      rdy_flag = true;
      break;
    }
  }
  if (rdy_flag == false) {
    Serial.println("\n\r\tHardware fault, or cable problem... cannot continue.");
    Serial.print("   Cable Status: ");
    prt_ethval(Ethernet.link());
    while (true) {
      delay(10);          // Halt.
    }
  } else {
    Serial.println("OK");
  }
  
  return true;
}

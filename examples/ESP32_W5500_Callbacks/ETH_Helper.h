#include <WiFi.h>
#ifdef ETHERNET3
#include <Ethernet3.h>
#else
#define MAX_SOCK_NUM 4
#define ETHERNET_LARGE_BUFFERS
#include <Ethernet.h>
#endif

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
void hardreset() {
  pinMode(RESET_PIN, OUTPUT);
  digitalWrite(RESET_PIN, HIGH);
  delay(150);

  digitalWrite(RESET_PIN, LOW);
  delay(1);
  digitalWrite(RESET_PIN, HIGH);
  delay(150);
}

bool ETH_startup()
{
#ifdef ETHERNET3
  Ethernet.setRstPin(RESET_PIN);
  Ethernet.setCsPin(CS_PIN);
  Ethernet.init(4); // maxSockNum = 4 Socket 0...3 -> RX/TX Buffer 4k
  Serial.println("Resetting Wiz W5500 Ethernet Board...  ");
  Ethernet.hardreset();
#else
  Ethernet.init(CS_PIN);
  Serial.println("Resetting Wiz Ethernet Board...  ");
  hardreset();
#endif

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
#ifdef ETHERNET3
    if ((Ethernet.link() == 0)) {
#else
    if ((Ethernet.linkStatus() == Unknown)) {
#endif  
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
    while (true) {
      delay(10);          // Halt.
    }
  } else {
    Serial.println("OK");
  }

  return true;
}

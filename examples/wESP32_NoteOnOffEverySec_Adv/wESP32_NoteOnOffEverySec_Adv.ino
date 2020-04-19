#include "ETH_Helper.h"

#include "WebServerTask.h"
#include "AppleMIDITask.h"

TaskHandle_t TaskWebServer;
TaskHandle_t TaskAppleMIDI;

void setup()
{
  Serial.begin(115200);
  while (!Serial);
  Serial.println("Booting");

  ETH_startup();

  MDNS.begin("ESP32");

  //create a task that will be executed in the Task2code() function, with priority 1 and executed on core 1
  xTaskCreatePinnedToCore(
    TaskWebServercode,  // Task function.
    "WebServer",        // name of task.
    10000,              // Stack size of task
    NULL,               // parameter of the task
    tskIDLE_PRIORITY,   // priority of the task
    &TaskWebServer,     // Task handle to keep track of created task
    0);                 // pin task to core 0

  //create a task that will be executed in the Task1code() function, with priority 1 and executed on core 0
  xTaskCreatePinnedToCore(
    TaskAppleMIDIcode,  // Task function.
    "AppleMIDI",        // name of task.
    10000,              // Stack size of task
    NULL,               // parameter of the task
    tskIDLE_PRIORITY + 1, // priority of the task
    &TaskAppleMIDI,     // Task handle to keep track of created task
    1);                 // pin task to core 1
}

void loop()
{
}

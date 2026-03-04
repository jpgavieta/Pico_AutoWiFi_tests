// date created: 02/24/2026
// last updated:
// refs:
  // - https://pcbsync.com/raspberry-pi-pico-with-arduino-ide/ (arduino setup)
  // - https://github.com/Dayleyfocus/pico-wifi-manager (intro tutorial)
  // - https://github.com/tuyennv1602/esp-wifi-manager/blob/main/wifi_manager.cpp (main inspo)
  // - https://github.com/Dayleyfocus/pico-wifi-manager/blob/main/code.py (maybe later)


// ---------- WiFi stuff ----------
#include <WiFi.h> // pico wifi library

  // network name & password
const char* ssid = "xx"; 
const char* password = "xx!"; 
    // why? const makes variables "read-only", assigning a new value causea compile error 

unsigned long strtRconnectTime = 0; // rconnect time tracker

enum WifiState {
  WIFI_NOCONNECT,   // Not connected
  WIFI_SEARCH,     // Attempting connection
  WIFI_CONNECT,      // Successfully connected
  WIFI_PAUSED         // Short break
};

WifiState wifiState = WIFI_NOCONNECT; // initial state
  // state new var (wifiState) to new type (WifiState)

// ---------- LED stuff ----------
void led_on() { digitalWrite(LED_BUILTIN, HIGH); }
void led_off() { digitalWrite(LED_BUILTIN, LOW); }

const long blinkInterval = 150; // blink every 0.150s (*in milliseconds) 
void led_blink() {
    if (millis() % blinkInterval < (blinkInterval / 2)) {
      led_on(); // led on in 1st half of the interval
    } else {
      led_off(); // led off in 2nd half of the interval
    }
}

// ----------   ----------  -----------  ----------
void setup() {

  Serial.begin(115200);
    while (!Serial) { ; } // wait for USB connect before running program
    // why? since pico's USB serial port is virtual (not dedicated UART-to-USB chip)

  pinMode(LED_BUILTIN, OUTPUT); // initialize digital pin LED_BUILTIN as an output
  led_on();
  Serial.print("I'm waking up!");
  Serial.println(ssid);

// initial connection
WiFi.begin(ssid, password);
wifiState = WIFI_SEARCH;

}

// ----------   ----------  -----------  ----------
void loop() {
  unsigned long currentTime = millis();
  static bool wasConnected = false; // tracks connection state
  static unsigned long lastAttempt = 0; // for 5s retry interval

  if (wifiState == WIFI_SEARCH) led_blink();

  switch (wifiState) {

    // ---------- Not connected ----------
    case WIFI_NOCONNECT:
      if(WiFi.status() != WL_CONNECTED) { //if its not connected
        strtRconnectTime = millis();
         wifiState = WIFI_SEARCH; // immediately go to search mode
      }
      break;

    // ---------- Searching mode ----------
    case WIFI_SEARCH:
      if(WiFi.status() == WL_CONNECTED) { //if it is connected
        if (!wasConnected) {
          Serial.println("Connected! Pico is happy (^_^ )");
          Serial.print("IP: ");
          Serial.println(WiFi.localIP());
          wasConnected = true;
        }
        led_on();
        wifiState = WIFI_CONNECT; // update to connect mode
      } 
      else if (currentTime - strtRconnectTime < 300000) { // if its not, starts 5min search
        if (currentTime - lastAttempt > 5000 ) { // every 5s (no spamming)
          lastAttempt = currentTime; // updates first
          Serial.println("Searching for connection...");
          if (WiFi.status() != WL_CONNECTED && WiFi.status() != WL_IDLE_STATUS) {
            // WiFi.disconnect(); // clean slate
            // WiFi.begin(ssid, password); // try again
          }
        } 
      }
      else { // after 5min over
        Serial.println("I'm tired. Give me a minute please.");
        led_off(); 
        wifiState = WIFI_PAUSED;
        strtRconnectTime = millis(); // restart for pause timer 
      }
      break;

      // note: for some fucking reason the light doesn't blink if it runs the wifi credentials in search mode. so here's what i did:
        // 1. first connection, WiFi.begin() runs the network credentials in setup() no need to run it again in loop()
        // 2. second connection, if the first connection succeeds but later fails, the network credentials will rerun upon:
                // - immediately after its disconnected in Connected mode
                // - immediately after its break in Pause mode 
  
    // ---------- Paused mode ----------
    case WIFI_PAUSED: // timer already restarted at end of search mode
      led_off();
      if (currentTime - strtRconnectTime >= 60000) { // waits 1min
        WiFi.begin(ssid, password); // one fresh attempt after the break
        wifiState = WIFI_SEARCH; 
        strtRconnectTime = millis(); // restarts rconnect timer for next 5min search
      }
      break;
      
    // ---------- Connected mode ----------
    case WIFI_CONNECT: // checks connection
      if(WiFi.status() == WL_CONNECTED) { // if it is connected
        if (!wasConnected) { // first loop after rconnect
          Serial.println("Connected! Pico is happy (^_^ )");
          Serial.print("IP: ");  
          Serial.println(WiFi.localIP()); 
          wasConnected = true;    
        }
        led_on();
      } 
      else { // if it looses connection
        if (wasConnected) {
          Serial.println("Lost connection. Pico is sad again (v_v )");
          wasConnected = false;
              wasConnected = false;
              WiFi.begin(ssid, password); // called immediately on disconnect
        }
        wifiState = WIFI_SEARCH;
        strtRconnectTime = millis(); // starts rconnect timer
      }
      break;
  }
}

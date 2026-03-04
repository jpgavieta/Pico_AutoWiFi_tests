// date created: 02-17-2026
// last updated: 02-24-2026
// refs:
  // - https://pcbsync.com/raspberry-pi-pico-with-arduino-ide/
  // - https://github.com/Dayleyfocus/pico-wifi-manager

/////////// global variables+functions+libraries
#include <WiFi.h> // WiFi library

// WiFi stuff
const char* ssid = "xx"; // hotspot name
const char* password = "xx!"; // hotspot password
    // why? const makes variables "read-only", attempts to assign a new value will cause compilation error 

bool wasConnect = false; //  boolean variable for tracking connection status
unsigned long lastRconnectAtmpt = 0; // size variable for non-neg #s, only 0 and pos integers (unlike standard longs)
unsigned long strtRconnectTime = 0;

// led stuff
void led_on() { digitalWrite(LED_BUILTIN, LOW); }
void led_off() { digitalWrite(LED_BUILTIN, HIGH); }
const long blinkInterval = 150; // interval at which to blink (ms *milliseconds; 0.150s)

void led_blink() {
  // if (!connect && strtRconnectTime > 0) {  // can only run after running strtRconnectTime
    if (millis() % blinkInterval < (blinkInterval / 2)) {
      led_on(); // led on in 1st half of the interval
    } else {
      led_off(); // led off in 2nd half of the interval
    }
  // }
}
// explanation:
          // millis() --> returns the number of ms since the program started running (constantly increases as it runs)
          // % blinkInterval --> calculates the remainder of dividing by blinkInterval
          // < (blinkInterval / 2) --> remainder has to be smaller than 75; first half of interval! = on
          // } else { --> if remainder isn't smaller (is larger) than 75; second half of interval = off
          // rapid blinking every 0.075s, not every 0.150s

//void led_fastblink() { led_on(); delay(150); led_off(); delay(150); }
//void led_slowblink() { led_on(); delay(500); led_off(); delay(500); }
  // DONT USE delay() to control the LED! why?
    // - its a blocking function that halts program execution for a specified period
    // - when delay runs (when led blinks/off), the pico can't perform any other operations, including network tasks
    // - so when the device is stuck in disconnected, it's blocked in delay; it can't check for updates in the network to reconnect when blinking
    // - delay would result in longer reconnection waiting

void setup() {
    // put your setup code here, to run once:
  Serial.begin(115200);
  while (!Serial) { ; } // wait for USB connect before running program
  // why? since pico's USB serial port is virtual (not dedicated UART-to-USB chip)

  pinMode(LED_BUILTIN, OUTPUT); // initialize digital pin LED_BUILTIN as an output
  Serial.print("I'm waking up!");
  Serial.println(ssid);

/////////// initial connection
  WiFi.begin(ssid, password);

  unsigned long strtTime = millis(); // track start time
  unsigned long atmptDur = 30000; // attempt for first 30s
  
  while (millis() - strtTime < atmptDur && WiFi.status() != WL_CONNECTED) {
      Serial.print("I want to feel connected.");
      led_blink();
  }  // <-- FIXED: properly closed while loop

  if (WiFi.status() == WL_CONNECTED) {
      Serial.println("Connected! Pico is happy."); // prints only once (to print repeatedly "\nConnected.")
      Serial.print("IP Address: ");
      Serial.println(WiFi.localIP());
      led_on(); 
  } else {
      Serial.println("Failed. Pico is sad.");
      led_off(); 
      Serial.flush(); // waits until this message is completely sent before continuing
  }
}
  
void loop() {

/////////// check connection status
  bool connect = (WiFi.status() == WL_CONNECTED);
  if (connect != wasConnect) { // checking if not connected // ! --> unary operator to invert the truth of boolean expression (makes it false) 
      Serial.println(connect ? "Connected :)" : "Disconnected :()"); // ? --> ternary operator to express conditional true/false statements
      connect ? led_on() : led_off();
      wasConnect = connect; // updates status
      if (connect) { // notifying if its connected
          Serial.print("IP: ");
          Serial.println(WiFi.localIP());
          strtRconnectTime = 0;
      } else {
          strtRconnectTime = millis(); // if unconnected, starts reconnect timer
      }
  }

/////////// automatic reconnection logic
  if (!connect) {
      if (millis() - strtRconnectTime < 300000) { // attempts for 5min
        led_blink();
        Serial.println("Sorry. I'm trying my best...");
        WiFi.begin(ssid, password);
     } else if (millis() - strtRconnectTime < 360000) { // adds 1min break (+60000)
        if (millis() - strtRconnectTime == 300000) { // prints before break
            led_off();
            Serial.println("I'm tired. Give me a minute please");
        }
     } else { // after 1min pause, reset reconnect timer
        strtRconnectTime = millis();
        }
  }  
}

// reflection: its not using machine states, its using boolean logic this is hell for cocurring tasks

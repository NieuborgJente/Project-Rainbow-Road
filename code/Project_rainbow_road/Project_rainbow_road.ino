#include <Bluepad32.h>

// define GPIO-pins for the transistors
const int pinForward  = 22; // Controlled via RT
const int pinBackward = 23; // Controlled via LT
const int pinLeft     = 21; // Controlled via Joystick to left
const int pinRight    = 19; // Controlled via Joystick to right

ControllerPtr myControllers[BP32_MAX_GAMEPADS];

// variables to toggle
bool lastForward = false;
bool lastBackward = false;
bool lastLeft = false;
bool lastRight = false;

void onConnectedController(ControllerPtr ctl) {
  bool foundEmptySlot = false;
  for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
    if (myControllers[i] == nullptr) {
      Serial.printf("VERBONDEN: Controller index=%d\n", i);
      myControllers[i] = ctl;
      foundEmptySlot = true;
      break;
    }
  }
}

void onDisconnectedController(ControllerPtr ctl) {
  for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
    if (myControllers[i] == ctl) {
      Serial.printf("LOSGEKOPPELD: Controller index=%d\n", i);
      myControllers[i] = nullptr;
      break;
    }
  }
}

void setup() {
  // When using the serial monitor, don't forget to set the baud rate to 115200.
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n--- ESP32 starting (RT/LT Gas + Joystick Sturen) ---");
  
  // Initialize GPIO pins
  pinMode(pinForward, OUTPUT);
  pinMode(pinBackward, OUTPUT);
  pinMode(pinLeft, OUTPUT);
  pinMode(pinRight, OUTPUT);

  digitalWrite(pinForward, LOW);
  digitalWrite(pinBackward, LOW);
  digitalWrite(pinLeft, LOW);
  digitalWrite(pinRight, LOW);

  BP32.setup(&onConnectedController, &onDisconnectedController);
  BP32.forgetBluetoothKeys();
  
  Serial.println("Ready! Use RT to go forward, LT to go in reverse, and the joystick to steer.");
}

void loop() {
  bool dataUpdated = BP32.update();
  if (dataUpdated) {
    processControllers();
  }
  delay(20);
}

void processControllers() {
  for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
    ControllerPtr ctl = myControllers[i];
    if (ctl && ctl->isConnected() && ctl->isGamepad()) {
      
      // Triggers uitlezen
      int triggerRight = ctl->throttle(); // RT (gas / vooruit)
      int triggerLeft  = ctl->brake();     // LT (rem / achteruit)
      
      // Joystick X-as uitlezen
      int joystickX = ctl->axisX(); // Loopt van -511 tot 511

      // Deadzones values
      int triggerDeadzone = 30; 
      int joystickDeadzone = 150; 

      bool currentForward  = (triggerRight > triggerDeadzone);
      bool currentBackward = (triggerLeft > triggerDeadzone);
      bool currentLeft     = (joystickX < -joystickDeadzone);
      bool currentRight    = (joystickX > joystickDeadzone);

      // Forward (RT)
      if (currentForward != lastForward) {
        digitalWrite(pinForward, currentForward ? HIGH : LOW);
        if (currentForward) Serial.println("Vooruit (RT ingedrukt): AAN");
        else Serial.println("Vooruit (RT losgelaten): UIT");
        lastForward = currentForward;
      }

      // Backward (LT)
      if (currentBackward != lastBackward) {
        digitalWrite(pinBackward, currentBackward ? HIGH : LOW);
        if (currentBackward) Serial.println("Achteruit (LT ingedrukt): AAN");
        else Serial.println("Achteruit (LT losgelaten): UIT");
        lastBackward = currentBackward;
      }

      // Left (Joystick left)
      if (currentLeft != lastLeft) {
        digitalWrite(pinLeft, currentLeft ? HIGH : LOW);
        if (currentLeft) Serial.println("Links (Joystick): AAN");
        else Serial.println("Links (Joystick): UIT");
        lastLeft = currentLeft;
      }

      // Right (Joystick right)
      if (currentRight != lastRight) {
        digitalWrite(pinRight, currentRight ? HIGH : LOW);
        if (currentRight) Serial.println("Rechts (Joystick): AAN");
        else Serial.println("Rechts (Joystick): UIT");
        lastRight = currentRight;
      }
    }
  }
}
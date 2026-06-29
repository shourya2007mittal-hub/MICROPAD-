#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <RotaryEncoder.h>
#include <PluggableUSBHID.h>
#include <USBKeyboard.h>

// --- OLED Display Configuration ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// --- Pin Definitions from Schematic ---
// Matrix Rows & Columns
const int rowPins[3] = {D0, D1, D2}; // R1=D0, R2=D1, R3=D2
const int colPins[3] = {D3, D6, D8}; // C1=D3, C2=D6, C3=D8

// Rotary Encoder Pins
#define ENCODER_PIN_A D9   // A1 mapped to D9 (P1.14_MISO_D9)
#define ENCODER_PIN_B D10  // B1 mapped to D10 (P1.15_MOSI_D10)

// Debug Status LED
#define STATUS_LED D5      // LED mapped to D5 (P0.05_AIN3_A5_D5)

// --- Operating Modes ---
enum DeviceMode { OUTPUT_MODE, INPUT_MODE };
DeviceMode currentMode = OUTPUT_MODE;

// --- Layer Configuration ---
int currentLayer = 0;
const int TOTAL_LAYERS = 3;

// Keymap definitions for INPUT MODE (USB HID Keyboard layout)
const char keymaps[TOTAL_LAYERS][3][3] = {
  { // Layer 0: Media / Navigation
    {'1', '2', '3'},
    {'4', '5', '6'},
    {'7', '8', '9'}
  },
  { // Layer 1: Numpad / Custom
    {'a', 'b', 'c'},
    {'d', 'e', 'f'},
    {'g', 'h', 'i'}
  },
  { // Layer 2: Macro Shortcuts
    {'X', 'Y', 'Z'},
    {'+', '-', '*'},
    {'[', ']', '='}
  }
};

// Track key states to prevent duplicate presses
bool lastKeyState[3][3] = { {false, false, false}, {false, false, false}, {false, false, false} };

// Setup Rotary Encoder
RotaryEncoder encoder(ENCODER_PIN_A, ENCODER_PIN_B, RotaryEncoder::LatchMode::FOUR3);
USBKeyboard Keyboard; // Instantiated for HID functionality

void updateDisplay() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  // Display current active system mode
  display.setCursor(0, 0);
  display.print("MODE: ");
  if (currentMode == INPUT_MODE) {
    display.print("USB KEYBOARD (IN)");
  } else {
    display.print("PERIPHERAL (OUT)");
  }
  
  // Display active functional layer
  display.setCursor(0, 16);
  display.setTextSize(2);
  display.print("LAYER: ");
  display.print(currentLayer + 1);
  
  display.display();
}

void checkBootMode() {
  // Boot Code rule: Check if Top-Left Matrix button (R1, C1) is held at startup
  pinMode(rowPins[0], OUTPUT);
  digitalWrite(rowPins[0], LOW); // Set Row 1 active
  
  pinMode(colPins[0], INPUT_PULLUP); // Read Column 1
  delay(50); // Small debounce window
  
  if (digitalRead(colPins[0]) == LOW) {
    currentMode = INPUT_MODE; // Switched on and pressing a button -> Input Device
  } else {
    currentMode = OUTPUT_MODE; // Default -> Output Device
  }
  
  // Reset pins back to default states
  digitalWrite(rowPins[0], HIGH);
}

void setup() {
  // Initialize Status LED
  pinMode(STATUS_LED, OUTPUT);
  digitalWrite(STATUS_LED, HIGH); // Off (XIAO LEDs are usually active-low)

  // Initialize Matrix Pins
  for (int i = 0; i < 3; i++) {
    pinMode(rowPins[i], OUTPUT);
    digitalWrite(rowPins[i], HIGH);
    pinMode(colPins[i], INPUT_PULLUP);
  }

  // Evaluate startup boot conditions
  checkBootMode();

  // Initialize OLED Display via standard I2C (SDA=D4, SCL=D5 as per schematic context)
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    // Blink LED rapidly if display connection fails
    while(1) {
      digitalWrite(STATUS_LED, LOW); delay(100);
      digitalWrite(STATUS_LED, HIGH); delay(100);
    }
  }
  
  updateDisplay();
}

void loop() {
  // 1. Read Rotary Encoder to change Active Layers
  encoder.tick();
  int newPos = (int)encoder.getDirection();
  if (newPos != 0) {
    currentLayer += (int)newPos;
    if (currentLayer >= TOTAL_LAYERS) currentLayer = 0;
    if (currentLayer < 0) currentLayer = TOTAL_LAYERS - 1;
    updateDisplay();
  }

  // 2. Scan Key Matrix
  for (int r = 0; r < 3; r++) {
    digitalWrite(rowPins[r], LOW); // Activate active row
    
    for (int c = 0; c < 3; c++) {
      bool isPressed = (digitalRead(colPins[c]) == LOW);
      
      if (isPressed && !lastKeyState[r][c]) {
        // --- KEY DOWN EVENT ---
        if (currentMode == INPUT_MODE) {
          // Act as USB Input Device
          Keyboard.key_code(keymaps[currentLayer][r][c]);
        } else {
          // Act as Output Device (Example: Debug print over Serial, or MIDI commands)
          digitalWrite(STATUS_LED, LOW); // Pulse active LED indicator on active output
        }
        lastKeyState[r][c] = true;
        delay(10); // Debounce
      } 
      else if (!isPressed && lastKeyState[r][c]) {
        // --- KEY UP EVENT ---
        if (currentMode == OUTPUT_MODE) {
          digitalWrite(STATUS_LED, HIGH);
        }
        lastKeyState[r][c] = false;
        delay(10); // Debounce
      }
    }
    digitalWrite(rowPins[r], HIGH); // Deactivate row
  }
}

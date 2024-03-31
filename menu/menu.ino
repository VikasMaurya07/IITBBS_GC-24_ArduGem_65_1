#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <Wire.h>

// Define pins for the joystick
#define JOY_X 0
#define JOY_Y 1
#define LEFT_BUTTON 5
#define RIGHT_BUTTON 3
#define START_BUTTON 6
#define JOY_BTN 8
#define UP_BTN 2
#define DOWN_BTN 4

#define TFT_CS 10
#define TFT_RST 12  // Or set to -1 and connect to Arduino RESET pin
#define TFT_DC 9

#define BLACK 0x0000
#define RED 0x001F
#define BLUE 0xF800
#define GREEN 0x07E0
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define WHITE 0xFFFF

int inGame = 0;

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);


void menu() {
    tft.fillScreen(BLACK);
    tft.setTextSize(1); // Set text size to 2
    tft.setCursor(30, 54); // Set cursor position
    tft.setTextColor(WHITE);
    tft.println("PLAY: UP BTN");
    tft.setTextColor(WHITE);
    tft.setCursor(1, 74);
    tft.println("INSTRUCTIONS: DOWN BTN");
    
    while(true) {
    if (digitalRead(UP_BTN) == LOW) {
        tft.fillScreen(BLACK);
        tft.setCursor(10, 50);
        tft.setTextColor(WHITE);
        tft.println("PLAY pressed!");
        delay(1000); // Display message for 1 second
        tft.fillScreen(BLACK); // Clear the screen
        return;
    }

    if (digitalRead(DOWN_BTN) == LOW) {
        tft.fillScreen(BLACK);
        tft.setCursor(10, 50);
        tft.setTextColor(WHITE);
        tft.println("INSTRUCTIONS pressed!");
        delay(1000); // Display message for 1 second
        tft.fillScreen(BLACK); // Clear the screen
        instruct();
        return;
    }}
}


void setup() {
   pinMode(LEFT_BUTTON, INPUT_PULLUP);
   pinMode(RIGHT_BUTTON, INPUT_PULLUP);
   pinMode(START_BUTTON, INPUT_PULLUP);
    pinMode(JOY_BTN, INPUT_PULLUP);
  pinMode(UP_BTN, INPUT_PULLUP);
    tft.initR(INITR_BLACKTAB); // Initialize screen
    tft.fillScreen(BLACK);
    tft.setRotation(3);
    tft.setCursor(44, 34);
    tft.setTextSize(1);
    tft.setTextColor(WHITE);
    tft.println("Welcome");
    tft.setCursor(60, 50);
    tft.setTextColor(WHITE);
    tft.println("to");
    tft.setTextSize(2);
    tft.setCursor(10, 70);
    tft.println("KAMI-KRAZE");
    Serial.begin(9600);
    delay(1000);
}

void loop() {
  menu();

  delay(1000); // Adjust delay as needed for desired speed
}

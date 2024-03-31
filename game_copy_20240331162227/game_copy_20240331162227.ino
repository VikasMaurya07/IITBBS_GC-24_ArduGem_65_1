#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <Wire.h>

#define TFT_CS     10
#define TFT_RST    12 // Or set to -1 and connect to Arduino RESET pin
#define TFT_DC     9

#define TFT_WIDTH  128
#define TFT_HEIGHT 128

#define SHIP_WIDTH  5
#define SHIP_HEIGHT 5

#define BLACK   0x0000
#define RED     0x001F
#define BLUE    0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0  
#define WHITE   0xFFFF

#define JOY_X 0 // Analog pin connected to joystick X-axis
#define JOY_Y 1 // Analog pin connected to joystick Y-axis

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
int shipX = 0;     // Initial position of the ship
int shipY = 0;     // Initial position of the ship in Y-axis
int directionX = 10; // Initial direction in the X-axis
int directionY = 9; // Initial direction in the Y-axis
int ratepX = 0;
int ratenX = -1;
int ratepY = 0;
int ratenY = -1; 
int xpos = 44;
int ypos = 5;
int direc = 10;
int ownposx = 44;
bool inGame = false; // Flag to indicate if the game is active

int up_button = 2;
int down_button = 4;
int left_button = 5;
int right_button = 3;
int start_button = 6;
int select_button = 7;
int joystick_button = 8;
int joystick_axis_x = A0;
int joystick_axis_y = A1;

int buttons[] = {up_button, down_button, left_button, right_button, start_button, select_button, joystick_button};
bool upButtonPressed = false;
bool downButtonPressed = false;

void moveShipLinearly() {
  // Calculate new position for the ship based on the direction
  int newShipX = shipX + directionX;
  int newShipY = shipY + directionY;
  if (newShipX < 0 || newShipX + SHIP_WIDTH > TFT_WIDTH) {
    // Change the direction in the X-axis
    
    if(newShipX + SHIP_WIDTH > TFT_WIDTH){
      directionX = random(-10, -3);
    }
    else {
      directionX = random(4, 10);
    }
    // Recalculate new position based on the new direction
    newShipX = shipX + directionX;
  }
    Serial.print(newShipY);
  Serial.print("..................");
  Serial.print(ownposx);
  Serial.print("..................");
   Serial.print(newShipX);
  Serial.print("\n");
  // Check if the ship hits the top or bottom
  if (newShipY < 10 && newShipX < xpos+20 &&  newShipX > xpos-20) {
    // Change the direction in the Y-axis
    
      directionY = random(1, 10);
    }
  else if (newShipY < 10  && newShipX < xpos+30 &&  newShipX > xpos-30){

  }
  if( newShipY > 115 && newShipX < ownposx+30 &&  newShipX > ownposx-30){
    directionY = random(-10, -7);
  }
  
  else if(newShipY > 115 && (newShipX > ownposx+20 ||  newShipX < ownposx-20)) {
    
  }
    // Recalculate new position based on the new direction
    newShipY = shipY + directionY;
  

  // Clear previous position of the ship
  // tft.fillRect(shipX, shipY, SHIP_WIDTH, SHIP_HEIGHT, BLACK);
  // tft.fillCircle(shipX, shipX, 10, ILI9341_WHITE);

  // Draw ship at the new position
  // tft.drawBitmap(newShipX, newShipY, (const uint8_t*)ship, SHIP_WIDTH, SHIP_HEIGHT, WHITE);
  tft.fillCircle(shipX, shipY, 3, BLACK);
  tft.fillCircle(newShipX, newShipY, 3, BLUE);
  
  // Update current position of the ship
  shipX = newShipX;
  shipY = newShipY;
}

void comppaddle(){
  int newxpos = xpos + direc;
  // Serial.print(newxpos);
  // Serial.print("..................");
  // Serial.print("30");
  // Serial.print("\n");
  if(newxpos <0 || newxpos + 40 > TFT_WIDTH){
    direc = -direc;
  }
  tft.fillRect(xpos, 5, 40, 5, BLACK);
  tft.fillRect(newxpos, 5, 40, 5, RED);
  xpos=newxpos;
}

void moveownrect(){
  int joyX = analogRead(JOY_X)-332;
  int joyY = analogRead(JOY_Y);
  // Serial.print(joyX);
  // Serial.print("..................");
  // Serial.print(joyY);
  // Serial.print("\n");
  int newownposx = ownposx + joyX/25;
  if(newownposx<0){
    newownposx = 0;
  }
  if(newownposx + 40 > TFT_WIDTH){
    newownposx = 88;
  }
  tft.fillRect(ownposx, 120, 40, 5, BLACK);
  tft.fillRect(newownposx, 120, 40, 5, RED);
  ownposx=newownposx;
}
void title(bool option1) {
    tft.fillScreen(BLACK);
    tft.setTextSize(1); // Set text size to 2
    tft.setCursor(44, 54); // Set cursor position

    if (option1) {
        tft.setTextColor(RED);
        tft.println("PLAY");
        tft.setTextColor(WHITE);
        tft.setCursor(44, 74);
        tft.println("SCORE");

        // Check if select_button is pressed
        if (digitalRead(right_button) == 0) {
            tft.fillScreen(BLACK);
            Serial.print("hello");
            inGame = true; // Enter game mode
            game(); // Start the game
            return;
        }
    } else {
        tft.setTextColor(WHITE);
        tft.println("PLAY");
        tft.setTextColor(RED);
        tft.setCursor(44, 74);
        tft.println("SCORE");
    }
}

void game() {
    while (inGame) {
        moveShipLinearly();
        comppaddle();
        moveownrect();
        // No button press check here to keep the game running until inGame is false
    }
}

void setup() {
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
    for (int i = 0; i < 7; i++) {
        pinMode(buttons[i], INPUT);
        digitalWrite(buttons[i], HIGH);
    }

    Serial.begin(9600);
}

void loop() {
    // Check if up_button is pressed
    if (digitalRead(up_button) == 0) {
        if (!inGame) {
            title(true); // Display "PLAY" only if not in game
            while (digitalRead(up_button) == 0); // Wait until up_button is released
        }
    }

    // Check if down_button is pressed
    if (digitalRead(down_button) == 0) {
        if (!inGame) {
            title(false); // Display "SCORE" only if not in game
            while (digitalRead(down_button) == 0); // Wait until down_button is released
        }
    }
    delay(100); // Adjust delay as needed for desired speed
}

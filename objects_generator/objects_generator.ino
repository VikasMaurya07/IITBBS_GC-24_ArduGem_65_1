#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <SPI.h>

#define TFT_CS     10
#define TFT_RST    12 // Or set to -1 and connect to Arduino RESET pin
#define TFT_DC     9

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
// Base class for flying objects
class FlyingObject {
public:
  float x, y; // Position
  int id;   // Unique identifier
  int width, height; // Dimensions for erasing the object
  virtual void update() = 0; // Update position
  virtual void draw(Adafruit_ST7735 &tft) = 0; // Draw the object
  virtual void clear(Adafruit_ST7735 &tft) { // Clear the object's previous position
    tft.fillRect(x+1, y-6, width + 6, height + 6, ST7735_BLACK);
  }
};

// Derived class for birds
class Bird : public FlyingObject {
public:
  Bird(int startX, int startY, int newId) {
    x = startX;
    y = startY;
    id = newId;
    width = 12;
    height = 12;
  }
  void update() override {
    x -= 0.5; // Move left
  }
  void draw(Adafruit_ST7735 &tft) override {
    tft.fillTriangle(x, y, x+7, y+3, x+7, y-3, ST7735_WHITE);
  }
};

// Derived class for airplanes
class Airplane : public FlyingObject {
public:
  Airplane(int startX, int startY, int newId) {
    x = startX;
    y = startY;
    id = newId;
    width = 12;
    height = 12;
  }
  void update() override {
    x -= 0.5; // Move left faster than birds
  }
  void draw(Adafruit_ST7735 &tft) override {
    tft.fillRect(x, y, 10, 5, ST7735_RED);
  }
};

// Derived class for health objects
class Health : public FlyingObject {
public:
  Health(int startX, int startY, int newId) {
    x = startX;
    y = startY;
    id = newId;
    width = 12;
    height = 12;
  }
  void update() override {
    x -= 0.5; // Move left slowly
  }
  void draw(Adafruit_ST7735 &tft) override {
    tft.fillCircle(x, y, 5, ST7735_BLUE); // Assuming radius 5 for health objects
  }
};

// Function to create a random object
FlyingObject* createRandomObject(int layer, int id) {
  int startY = 30 + layer * 20; // Y position based on layer with a gap of 20 pixels
  int startX = tft.width(); // Start from the right edge
  int objectType = random(0, 20); // Randomly choose the type of object
  switch (objectType) {
    case 13:
    case 12: 
    return new Bird(startX, startY, id);
    case 1:
    case 19: 
    return new Airplane(startX, startY, id);
    case 7: return new Health(startX, startY, id);
  }
  return nullptr; // In case of an unexpected value
}


FlyingObject* objects[100]; // Array to hold pointers to objects
int nextId = 0; // ID for the next object

void setup() {
  tft.initR(INITR_BLACKTAB);   // Initialize a ST7735S chip, black tab
  tft.setRotation(3);          // Set rotation to landscape mode
  tft.fillScreen(ST7735_BLACK);   // Fill screen with black
  randomSeed(analogRead(0)); // Seed the random number generator
}

void loop() {
  // Randomly decide whether to create a new object
  if (random(0, 10) == 0) { // 10% chance to create a new object
    int layer = random(0, 3); // Choose a random layer
    objects[nextId % 100] = createRandomObject(layer, nextId);
    nextId++;
  }

  // Update and draw all objects
  for (int i = 0; i < 100; i++) {
    if (objects[i] != nullptr) {
      objects[i]->clear(tft); // Clear the object's previous position
      objects[i]->update();
      objects[i]->draw(tft);
      objects[i+1] = nullptr;
      // Check if the object is off the screen
      if (objects[i]->x < 10 - objects[i]->width) {
      tft.fillRect(objects[i]->x-2, objects[i]->y-6, objects[i]->width+7, objects[i]->height+7, ST7735_BLACK);
        delete objects[i]; // Delete the object
        objects[i] = nullptr; // Set the pointer to null
      }
    }
  }

  // Add a delay to control the speed of the game
  delay(0.05);
}

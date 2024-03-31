#include <Adafruit_GFX.h>  // Core graphics library // Hardware-specific library
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

#define BALL_RADIUS 2
#define BALL_SPEED 4

unsigned long currentTime = millis(); 

// Initialize the display
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

int score = 0;
int playerLife = 100;


float playerX = 64;
float playerY = 120;
float playerSize = 11.6;
int moveStep = 1;

class GreenBall {
public:
  float x, y;                    // Position
  float radius = 2;              // Radius of the ball
  float speed = 1.5;             // Speed of the ball
  float directionX, directionY;  // Direction of movement
  bool released;                 // Whether the ball has been released

  GreenBall(float startX, float startY, float r, float s)
    : x(startX), y(startY), radius(r), speed(s), released(false) {}

  void clear(Adafruit_ST7735 &tft) {
    if (released) {
      tft.fillCircle(x, y, radius, ST7735_BLACK);
      tft.fillCircle(playerX, playerY, playerSize, ST7735_BLUE);
    } else {
      tft.fillCircle(playerX, playerY, radius, ST7735_GREEN);
    }
  }

  // Function to draw the green ball
  void draw(Adafruit_ST7735 &tft) {
    tft.fillCircle(x, y, radius, ST7735_GREEN);
  }

  // Function to update the position of the green ball
  void update() {
    if (released) {
      x += directionX * speed;
      y += directionY * speed;
    }
  }

  // Function to release the green ball
  void release(float dirX, float dirY) {
    directionX = dirX;
    directionY = dirY;
    released = true;
  }

  // Function to check if the ball is out of bounds
  bool isOutOfBounds() {
    return (x < 5 || x > 125 || y < 5 || y > 125);
  }

  // Function to reset the green ball
  void reset(float startX, float startY) {
    x = startX;
    y = startY;
    released = false;
  }
};

class FlyingObject {
public:
  float x, y;  // Position
  int id;      // Unique identifier
  int width, height, type;
  // Dimensions for erasing the object
  virtual void update() = 0;                    // Update position
  virtual void draw(Adafruit_ST7735 &tft) = 0;  // Draw the object
  virtual void clear(Adafruit_ST7735 &tft) {    // Clear the object's previous position
    tft.fillRect(x - 7, y - 7, width + 7, height + 7, ST7735_BLACK);
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
    type = 0;
  }
  void update() override {
    x -= 0.8+ currentTime*0.05;  // Move left
  }
  void draw(Adafruit_ST7735 &tft) override {
    tft.fillTriangle(x, y, x + 7, y + 3, x + 7, y - 3, ST7735_WHITE);
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
    type = 1;
  }
  void update() override {
    x -= 0.8+ currentTime*0.05;  // Move left faster than birds
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
    type = 2;
  }
  void update() override {
    x -= 0.8+ currentTime*0.05;  // Move left slowly
  }
  void draw(Adafruit_ST7735 &tft) override {
    tft.fillCircle(x, y, 5, ST7735_BLUE);  // Assuming radius 5 for health objects
  }
};

// Function to create a random object
FlyingObject *createRandomObject(int layer, int id) {
  int startY = 30 + layer * 20;    // Y position based on layer with a gap of 20 pixels
  int startX = tft.width();        // Start from the right edge
  int objectType = random(0, 20);  // Randomly choose the type of object
  switch (objectType) {
    case 13:
    case 12:
      return new Bird(startX, startY, id);
    case 1:
    case 19:
    case 11:
      return new Airplane(startX, startY, id);
    case 3: return new Health(startX, startY, id);
  }
  return nullptr;  // In case of an unexpected value
}


FlyingObject *objects[215];  // Array to hold pointers to objects
int nextId = 0;

// Function to draw the ball
void drawBall(int x, int y) {
  tft.fillCircle(x, y, BALL_RADIUS, ST7735_GREEN);
}

const int triangleVertices[3][2] = {
  { -1, 0 },  // Bottom left
  { 1, 0 },   // Bottom right
  { 0, -6 }   // Top point
};

void fallingKame(int x, int y) {
  // Initial drawing of the falling object
  tft.fillRect(x, y, 10, 5, ST7735_RED);
  // Loop to animate the falling object
  while (y < 120) {  // Update the loop condition to avoid indefinite looping
    // Erase the previous position
    if (digitalRead(LEFT_BUTTON) == LOW && playerX > 8) {
      // Erase the previous player position
      tft.fillCircle(playerX, playerY, playerSize, ST7735_BLACK);
      // Update player position to the left
      playerX -= moveStep;
      // Draw the player at the new position
      tft.fillCircle(playerX, playerY, playerSize, ST7735_BLUE);
    }

    // Check if right button is pressed
    if (digitalRead(RIGHT_BUTTON) == LOW && playerX < 122) {
      // Erase the previous player position
      tft.fillCircle(playerX, playerY, playerSize, ST7735_BLACK);
      // Update player position to the right
      playerX += moveStep;
      // Draw the player at the new position
      tft.fillCircle(playerX, playerY, playerSize, ST7735_BLUE);
    }

    tft.fillRect(x, y, 10, 5, ST7735_BLACK);

    // Increment y position for falling effect
    y++;

    // Draw the object at the new position
    tft.fillRect(x, y, 10, 5, ST7735_RED);

    // Add a slight delay for animation smoothness
    delay(10);  // Adjust the delay value as needed
  }

  // Draw an explosion or any other effect at the final position
  tft.fillRoundRect(x - 2, y - 2, 15, 15, 15, WHITE);
  delay(60);
  tft.fillRoundRect(x - 2, y - 2, 15, 15, 40, BLACK);
  score += 50;
  playerLife = max(0, playerLife - (1 - abs(x - playerX) / 130) * 50);
}

// Variables to store the previous vertices of the ball pointer
int prevVertices[3][2];

void setup() {
  Serial.begin(9600);
  // Initialize the TFT screen
  tft.initR(INITR_BLACKTAB);
  tft.setRotation(3);
  // Draw the initial game screen
  // Set the joystick button as input
  pinMode(LEFT_BUTTON, INPUT_PULLUP);
  pinMode(RIGHT_BUTTON, INPUT_PULLUP);
  pinMode(START_BUTTON, INPUT_PULLUP);
  tft.fillScreen(ST7735_BLACK);  // Fill screen with black
  // Draw the player for the first time
  tft.fillCircle(playerX, playerY, playerSize, ST7735_BLUE);

  for (int i = 0; i < 3; i++) {
    prevVertices[i][0] = playerX + triangleVertices[i][0];
    prevVertices[i][1] = playerY + triangleVertices[i][1];
  }
  pinMode(JOY_BTN, INPUT_PULLUP);
  pinMode(UP_BTN, INPUT_PULLUP);
  randomSeed(analogRead(56));  // Seed the random number generator
}

int prevX = playerX;
int prevY = playerY;
float xDirection = 1;
float yDirection = 1;
float angle = atan2(yDirection, xDirection);
// Variables to store the filtered joystick values
float filteredXValue = 666;  // player value for X-axis
float filteredYValue = 666;  // player value for Y-axis
float filterStrength = 0.1;  // Adjust this value to control the filtering strength
GreenBall greenBall(playerX, playerY, BALL_RADIUS, BALL_SPEED);

void gamePlay() {
  while (playerLife > 0) {
    //TopScreen
    tft.fillRect(0, 0, tft.width(), 20, ST7735_BLACK);
    tft.fillRect(3, 4, 0.30*(playerLife), 10,ST7735_BLUE);
    tft.setCursor(37, 7);
    tft.print("KAMIKRAZE");
    tft.setTextColor(ST7735_GREEN);
    tft.setCursor(105, 7);
    tft.print(score);
    tft.setTextColor(ST7735_WHITE);

    int rawXValue = analogRead(JOY_X);
    int rawYValue = analogRead(JOY_Y);

    filteredXValue = (filterStrength * rawXValue) + ((1 - filterStrength) * filteredXValue);
    filteredYValue = (filterStrength * rawYValue) + ((1 - filterStrength) * filteredYValue);

    // Map the joystick values to a range from -1 to 1
    float xDirection = (filteredXValue - 0) / 666.0;
    float yDirection = (filteredYValue - 340) / 666.0;

    if (digitalRead(LEFT_BUTTON) == LOW && playerX > 8) {
      // Erase the previous player position
      tft.fillCircle(playerX, playerY, playerSize, ST7735_BLACK);
      // Update player position to the left
      playerX -= moveStep;
      // Draw the player at the new position
      tft.fillCircle(playerX, playerY, playerSize, ST7735_BLUE);
    }

    // Check if right button is pressed
    if (digitalRead(RIGHT_BUTTON) == LOW && playerX < 122) {
      // Erase the previous player position
      tft.fillCircle(playerX, playerY, playerSize, ST7735_BLACK);
      // Update player position to the right
      playerX += moveStep;
      // Draw the player at the new position
      tft.fillCircle(playerX, playerY, playerSize, ST7735_BLUE);
    }


    float angle = atan2(yDirection, xDirection);

    angle = constrain(angle, -PI / 2, PI / 2);

    // Rotate the triangle vertices based on the new angle
    int rotatedVertices[3][2];
    for (int i = 0; i < 3; i++) {
      rotatedVertices[i][0] = playerX + (cos(angle) * triangleVertices[i][0] - sin(angle) * triangleVertices[i][1]);
      rotatedVertices[i][1] = playerY + (sin(angle) * triangleVertices[i][0] + cos(angle) * triangleVertices[i][1]);
    }
    // Clear the triangle at its previous position by redrawing it in the background color
    tft.fillTriangle(
      prevVertices[0][0], prevVertices[0][1],
      prevVertices[1][0], prevVertices[1][1],
      prevVertices[2][0], prevVertices[2][1],
      ST7735_BLUE);

    // Draw the new rotated triangle
    tft.fillTriangle(
      rotatedVertices[0][0], rotatedVertices[0][1],
      rotatedVertices[1][0], rotatedVertices[1][1],
      rotatedVertices[2][0], rotatedVertices[2][1],
      ST7735_WHITE);



    // Update the previous vertices to the current ones for the next loop iteration
    for (int i = 0; i < 3; i++) {
      prevVertices[i][0] = rotatedVertices[i][0];
      prevVertices[i][1] = rotatedVertices[i][1];
    }
    greenBall.clear(tft);
    greenBall.update();
    if (digitalRead(JOY_BTN) == LOW || digitalRead(UP_BTN) == LOW) {
      if (!greenBall.released) {
        greenBall.x = playerX;
        greenBall.y = playerY;
        float angle = atan2(yDirection, xDirection);
        greenBall.release(cos(angle - PI / 2.0), sin(angle - PI / 2.0));
      }
    }
    if (greenBall.isOutOfBounds()) {
      greenBall.reset(130, 130);
      greenBall.released = 0;
      greenBall.clear(tft);
    }
    greenBall.draw(tft);

    // Randomly decide whether to create a new object
    if (random(0, 9) == 0) {     // 10% chance to create a new object
      int layer = random(0, 3);  // Choose a random layer
      objects[nextId++ % 215] = createRandomObject(layer, nextId);
    }

    // Update and draw all objects
    for (int i = 0; i < 215; i++) {
      if (objects[i] != nullptr) {
        objects[i]->clear(tft);  // Clear the object's previous position
        objects[i]->update();
        objects[i]->draw(tft);
        objects[i + 1] = nullptr;
        // Check if the object is off the screen
        if (objects[i]->x < 10 - objects[i]->width) {
          tft.fillRect(objects[i]->x - 2, objects[i]->y - 6, objects[i]->width + 7, objects[i]->height + 7, ST7735_BLACK);
          delete objects[i];     // Delete the object
          objects[i] = nullptr;  // Set the pointer to null
        }
      }
    }

    // Check for collisions between the green ball and flying objects
    for (int i = 0; i < 215; i++) {
      if (objects[i] != nullptr && greenBall.released) {
        // Calculate bounding box of the flying object
        int objectLeft = objects[i]->x - objects[i]->width / 2;
        int objectRight = objects[i]->x + objects[i]->width / 2;
        int objectTop = objects[i]->y - objects[i]->height / 2;
        int objectBottom = objects[i]->y + objects[i]->height / 2;

        // Calculate bounding box of the green ball
        int ballLeft = greenBall.x - greenBall.radius;
        int ballRight = greenBall.x + greenBall.radius;
        int ballTop = greenBall.y - greenBall.radius;
        int ballBottom = greenBall.y + greenBall.radius;

        // Check for intersection between bounding boxes
        if (ballRight >= objectLeft && ballLeft <= objectRight && ballBottom >= objectTop && ballTop <= objectBottom) {
          // Erase the object from the screen
          // Print the class of the flying object
          if (objects[i]->type == 1) {
            fallingKame(objects[i]->x, objects[i]->y);
          } else if (objects[i]->type == 0) {
            score += 15;

          } else if (objects[i]->type == 2) {
            score += 7;
            playerLife = min(100, playerLife + 20);
          }
          Serial.print("PL: ");
          Serial.println(playerLife);
          Serial.print("Score: ");
          Serial.println(score);

          objects[i]->clear(tft);

          // Delete the object
          delete objects[i];
          objects[i] = nullptr;

          // Reset the green ball
          greenBall.clear(tft);
          greenBall.reset(130, 130);
          greenBall.released = false;
        }
      }
    }
    unsigned long currentTime = millis();   // Current time since the Arduino board started running
    unsigned long decreaseInterval = 1000;  // Decrease player's life every 1 second (adjust as needed)
    static unsigned long previousDecreaseTime = 0;

    if (currentTime - previousDecreaseTime >= decreaseInterval) {
      // Decrease player's life by a certain amount every interval
      playerLife = max(0, playerLife - 3);  
      previousDecreaseTime = currentTime;
    }

    // Add a small delay to see the shapes on screen
    delay(2.5);
    Serial.print("PL22:");
    Serial.println(playerLife);
  }
}

  
void endGame() {
  tft.fillScreen(ST7735_BLACK);    // Clear the screen
  tft.setTextSize(2);              // Set text size
  tft.setTextColor(ST7735_WHITE);  // Set text color
  tft.setCursor(12, 40);           // Set cursor position
  tft.println("Game Over");        // Print "Game Over" message
  tft.setCursor(12, 70); // Set cursor position for the score
  tft.print("Score:"); // Print "Score: " prefix
  tft.println(score); // Print the actual score value
}

void loop() {
  gamePlay();
  endGame();
}



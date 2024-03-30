#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <SPI.h>

#define TFT_CS     10
#define TFT_RST    12 // Or set to -1 and connect to Arduino RESET pin
#define TFT_DC     9

#define TFT_WIDTH  128
#define TFT_HEIGHT 128


#define	BLACK   0x0000
#define	RED     0x001F
#define	BLUE    0xF800
#define	GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0  
#define WHITE   0xFFFF

#define JOY_X 0 // Analog pin connected to joystick X-axis
#define JOY_Y 1 // Analog pin connected to joystick Y-axis
#define JOY_BTN 8
#define UP_BTN 2

#define BALL_RADIUS 2
#define BALL_SPEED 1.5

class GreenBall {
public:
  float x, y; // Position
  float radius = 2; // Radius of the ball
  float speed = 1.5; // Speed of the ball
  float directionX, directionY; // Direction of movement
  bool released; // Whether the ball has been released

  GreenBall(float startX, float startY, float r, float s) : x(startX), y(startY), radius(r), speed(s), released(false) {}

    void clear(Adafruit_ST7735 &tft) {
    tft.fillCircle(x, y, radius, ST7735_BLACK);
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



// Initialize the display
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

// Triangle vertices relative to center
const int triangleVertices[3][2] = {
  {-1, 0}, // Bottom left
  {1, 0},  // Bottom right
  {0, -7}  // Top point
};

// Center of the screen
int centerX = 64;
int centerY = 80;
GreenBall greenBall(centerX, centerY, BALL_RADIUS, BALL_SPEED);

// Variables to store the previous vertices of the triangle
int prevVertices[3][2];

void setup() {
  tft.initR(INITR_BLACKTAB);   // Initialize a ST7735S chip, black tab
  tft.setRotation(3);          // Set rotation to landscape mode
  tft.fillScreen(ST7735_BLACK);   // Fill screen with black
    // Initialize the previous vertices to match the initial triangle vertices
  for (int i = 0; i < 3; i++) {
    prevVertices[i][0] = centerX + triangleVertices[i][0];
    prevVertices[i][1] = centerY + triangleVertices[i][1];
  }
 // Set the joystick button as an input
  pinMode(JOY_BTN, INPUT_PULLUP);
  pinMode(UP_BTN, INPUT_PULLUP);
}

int prevX = centerX;
int prevY = centerY;
float xDirection = -1;
float yDirection = -1;
float angle = atan2(yDirection, xDirection);
// Variables to store the filtered joystick values
float filteredXValue = 666; // Center value for X-axis
float filteredYValue = 666; // Center value for Y-axis
float filterStrength = 0.1; // Adjust this value to control the filtering strength


void loop() {
  // Read joystick values
  int rawXValue = analogRead(JOY_X);
  int rawYValue = analogRead(JOY_Y);

  filteredXValue = (filterStrength * rawXValue) + ((1 - filterStrength) * filteredXValue);
  filteredYValue = (filterStrength * rawYValue) + ((1 - filterStrength) * filteredYValue);

  // Map the joystick values to a range from -1 to 1
  float xDirection = (filteredXValue - 0) / 666.0;
  float yDirection = (filteredYValue - 333) / 666.0;

  float angle = atan2(yDirection, xDirection);

  angle = constrain(angle, -PI/2, PI/2);

  // Rotate the triangle vertices based on the new angle
  int rotatedVertices[3][2];
  for (int i = 0; i < 3; i++) {
    rotatedVertices[i][0] = centerX + (cos(angle) * triangleVertices[i][0] - sin(angle) * triangleVertices[i][1]);
    rotatedVertices[i][1] = centerY + (sin(angle) * triangleVertices[i][0] + cos(angle) * triangleVertices[i][1]);
  }
// Clear the triangle at its previous position by redrawing it in the background color
  tft.fillTriangle(
    prevVertices[0][0], prevVertices[0][1],
    prevVertices[1][0], prevVertices[1][1],
    prevVertices[2][0], prevVertices[2][1],
    ST7735_BLACK
  );

  // Draw the new rotated triangle
  tft.fillTriangle(
    rotatedVertices[0][0], rotatedVertices[0][1],
    rotatedVertices[1][0], rotatedVertices[1][1],
    rotatedVertices[2][0], rotatedVertices[2][1],
    ST7735_WHITE
  );

  // Update the previous vertices to the current ones for the next loop iteration
  for (int i = 0; i < 3; i++) {
    prevVertices[i][0] = rotatedVertices[i][0];
    prevVertices[i][1] = rotatedVertices[i][1];
  }
   greenBall.clear(tft);
 greenBall.update();
  if (digitalRead(JOY_BTN) == LOW || digitalRead(UP_BTN) == LOW) {
    if (!greenBall.released) {
      float angle = atan2(yDirection, xDirection);
      greenBall.release(cos(angle - PI / 2.0), sin(angle - PI / 2.0));
    }
  }
  if (greenBall.isOutOfBounds()) {
    greenBall.reset(centerX, centerY);
  }
  greenBall.draw(tft);
  // Add a small delay to control the speed of rotation
  delay(10);
}

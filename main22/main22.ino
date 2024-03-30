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

// Define the ball properties
#define BALL_RADIUS 2
#define BALL_SPEED 1.0


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
float ballX, ballY;
float ballXDirection, ballYDirection;

void drawBall(int x, int y) {
  tft.fillCircle(x, y, BALL_RADIUS, ST7735_GREEN);
}

bool ballReleased = false;

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
  drawBall(centerX,centerY);
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

if (digitalRead(JOY_BTN) == LOW||digitalRead(UP_BTN) == LOW) { // Assuming the button is active low
    // If the button is pressed and the ball hasn't been released yet
    if (!ballReleased) {
      // Calculate the ball's trajectory based on the pointer's angle
      ballXDirection = cos(angle-PI/2.0) * BALL_SPEED;
      ballYDirection = sin(angle-PI/2.0) * BALL_SPEED;

      // Release the ball
      ballReleased = true;
    }
  }

  // If the ball has been released, update its position
  if (ballReleased) {
    // Clear the previous ball position
    tft.fillCircle(ballX, ballY, BALL_RADIUS, ST7735_BLACK);
    // Update the ball's position
    ballX += ballXDirection;
    ballY += ballYDirection;

    // Draw the ball
    drawBall(ballX, ballY);

    // Check if the ball has reached the edge of the screen
    if (ballX < 5 || ballX > 125 || ballY < 5 || ballY > 125) {
      tft.fillCircle(ballX, ballY, BALL_RADIUS, ST7735_BLACK);
      // Reset the ball's position to the center of the pointer
      ballX = centerX;
      ballY = centerY;
      // Reset the ball's state
      ballReleased = false;
      drawBall(ballX, ballY);
    }
  }

  
  // Add a small delay to control the speed of rotation
  delay(10);
}

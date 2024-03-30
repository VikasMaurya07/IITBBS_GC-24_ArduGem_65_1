#include <Adafruit_GFX.h>    // Core graphics library // Hardware-specific library
#include <Adafruit_ST7735.h>
#include <Wire.h>

// Define pins for the joystick
#define JOY_X      0
#define JOY_Y      1
#define LEFT_BUTTON  5
#define RIGHT_BUTTON  3
#define START_BUTTON  6
#define JOY_BTN 8
#define UP_BTN 2

#define TFT_CS     10
#define TFT_RST    12 // Or set to -1 and connect to Arduino RESET pin
#define TFT_DC     9

#define	BLACK   0x0000
#define	RED     0x001F
#define	BLUE    0xF800
#define	GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0  
#define WHITE   0xFFFF

#define BALL_RADIUS 2
#define BALL_SPEED 5

// Initialize the display
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

float playerX = 64;
float playerY = 120;
float playerSize = 11.6;
int moveStep = 1; // Number of pixels the player moves with each button press
float ballX, ballY;
float ballXDirection, ballYDirection;

// Function to draw the ball
void drawBall(int x, int y) {
  tft.fillCircle(x, y, BALL_RADIUS, ST7735_GREEN);
}

const int triangleVertices[3][2] = {
  {-1, 0}, // Bottom left
  {1, 0},  // Bottom right
  {0, -6}  // Top point
};
// Game settings
int birdSpeed = 2;
int kamikazeSpeed = 3;
int playerLife = 100;
int score = 0;

// Function to draw a triangle (bird)
void drawBird(int x, int y) {
  tft.fillTriangle(x, y, x+7, y+5, x+7, y-5, ST7735_WHITE); // Coordinates for the triangle vertices
}

bool ballReleased = false;
// Function to draw a rectangle (kamikaze)
void drawKamikaze(int x, int y) {
  tft.fillRect(x, y, 10, 5, ST7735_RED); // x, y, width, height
}

// Variables to store the previous vertices of the ball pointer
int prevVertices[3][2];

void setup() {
  // Initialize the TFT screen
  tft.initR(INITR_BLACKTAB);
  tft.setRotation(3);  
  // Draw the initial game screen
  // Set the joystick button as input
  pinMode(LEFT_BUTTON, INPUT_PULLUP);
  pinMode(RIGHT_BUTTON, INPUT_PULLUP);
  pinMode(START_BUTTON, INPUT_PULLUP);
  tft.fillScreen(ST7735_BLACK);   // Fill screen with black
  // Draw the player for the first time
  tft.fillCircle(playerX, playerY, playerSize, ST7735_BLUE);

   for (int i = 0; i < 3; i++) {
    prevVertices[i][0] = playerX + triangleVertices[i][0];
    prevVertices[i][1] = playerY + triangleVertices[i][1];
  }
  pinMode(JOY_BTN, INPUT_PULLUP);
  pinMode(UP_BTN, INPUT_PULLUP);
  drawBall(playerX,playerY);
}
int prevX = playerX;
int prevY = playerY;
float xDirection = 1;
float yDirection = 1;
float angle = atan2(yDirection, xDirection);
// Variables to store the filtered joystick values
float filteredXValue = 666; // Center value for X-axis
float filteredYValue = 666; // Center value for Y-axis
float filterStrength = 0.1; // Adjust this value to control the filtering strength


void loop() {

  // Keep player within screen 
  int birdX = 20;
  int birdY = 30;
  int kamikazeX = 50;
  int kamikazeY = 60;

  int rawXValue = analogRead(JOY_X);
  int rawYValue = analogRead(JOY_Y);

  filteredXValue = (filterStrength * rawXValue) + ((1 - filterStrength) * filteredXValue);
  filteredYValue = (filterStrength * rawYValue) + ((1 - filterStrength) * filteredYValue);

  // Map the joystick values to a range from -1 to 1
  float xDirection = (filteredXValue - 0) / 666.0;
  float yDirection = (filteredYValue - 333) / 666.0;

  if (digitalRead(LEFT_BUTTON) == LOW && playerX>8) {
    // Erase the previous player position
    tft.fillCircle(playerX, playerY, playerSize, ST7735_BLACK);
    // Update player position to the left
    playerX -= moveStep;
    // Draw the player at the new position
    tft.fillCircle(playerX, playerY, playerSize, ST7735_BLUE);
  }

  // Check if right button is pressed
  if (digitalRead(RIGHT_BUTTON) == LOW && playerX<122) {
    // Erase the previous player position
    tft.fillCircle(playerX, playerY, playerSize, ST7735_BLACK);
    // Update player position to the right
    playerX += moveStep;
    // Draw the player at the new position
    tft.fillCircle(playerX, playerY, playerSize, ST7735_BLUE);
  }

  
  float angle = atan2(yDirection, xDirection);

  angle = constrain(angle, -PI/2, PI/2);

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
    ST7735_BLUE
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
      ballX = playerX;
      ballY= playerY;
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
    tft.fillCircle(playerX, playerY, playerSize, ST7735_BLUE);
    // Check if the ball has reached the edge of the screen
    if (ballX < 5 || ballX > 125 || ballY < 5 || ballY > 125) {
      tft.fillCircle(ballX, ballY, BALL_RADIUS, ST7735_BLACK);
      // Reset the ball's position to the center of the pointer
      ballX = playerX;
      ballY = playerY;
      drawBall(ballX, ballY);
      ballReleased = false;
    }
  }

  // Draw the shapes
  drawBird(birdX, birdY);       // Draw a triangle for the bird
  drawKamikaze(kamikazeX, kamikazeY); // Draw a rectangle for the kamikaze

  // Add a small delay to see the shapes on screen
  delay(10);

  // Update the positions or add game logic here
  // Read joystick inputs
  
  // Game logic for moving birds and kamikaze planes
  // Collision detection
  // Update player life and score
  // Redraw graphics
  
  // Check for game over condition
  if(playerLife <= 0) {
    // End game
    gameOver();
  }
}

// Function to generate a new bird
void generateBird() {
  // Generate bird at random position with birdSpeed
}

// Function to generate a new kamikaze plane
void generateKamikaze() {
  // Generate kamikaze plane at random position with kamikazeSpeed
}

// Function to handle game over
void gameOver() {
  // Display game over screen
  tft.fillScreen(BLACK);
  tft.setTextColor(WHITE);
  tft.setTextSize(2);
  tft.setCursor(50, 50);
  tft.print("Game Over!");
  
  // Optionally restart the game or return to a menu
}

// Additional functions for game logic

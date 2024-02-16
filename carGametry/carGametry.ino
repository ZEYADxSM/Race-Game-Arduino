#include <AberLED.h>
#include <EEPROM.h>

#define S_INVALID -1  // invalid state to which we initialise the state variable
#define S_START 0     // the starting state
#define S_PLAYING 1   // the playing state
#define S_LIFELOST 2  // a life has just been lost
#define S_PAUSE 3     // FIRE button was pressed
#define S_END 4       // the player has lost their last life

                                
///////////////////
// variables
///////////////////

#define EMPTY 0
#define REDBLOCK 1
#define X 0
#define Y 1

// Speed at wich the dot moves down, start at 300ms
unsigned long stateStartTime;
int state = S_INVALID;            //All above is setting up states and importing libraries
int pPos[8][8];                       //Player Position
unsigned long lastMoveTime = 0;     //Used for updateLCD()
int blocks[8][8];                   //The blocks array of the game
bool flash = false;                 //Used to flash a LED on start, pause and end states
unsigned char score;                //the current score
unsigned char highScore;            //the EEPROM score
int count = 0;                      //counts how many times updateLCD() is called in a single play state
float interval;                     //the speed that the game updates
int maxRan = 10;                     //the chances of a red dot spawning on a single LED
int dotPos[2] = {4, 0};              // used to initlise the obsticales
int dot [4][8];
unsigned long timePaused = 0;       //used for Pause state


// Array to store pattern data of every space on
// the screen during end and start states
int screenData[8][8];

// Array to store the locations of all red blocks
int redBlocks[8][8];


// always change state by calling this function, never
// set the state variable directly

void gotoState(int s) {
    Serial.print("Going to state "); // handy for debugging!
    Serial.println(s);
    state = s;
    stateStartTime=millis();
}

// get the time the system has been in the current state
unsigned long getStateTime(){
    return millis()-stateStartTime;
}


/**************************************************************************
 * 
 * The player model and its code
 * 
 **************************************************************************/


int playerX;      // player X position
int playerLives;  // number of lives remaining

// initialise the player model

void initPlayer(){
    playerX = 4;
    playerLives = 3;
}

void initPlayer1(){
  playerX = 4;
}

// removes a life from the player,
// returns true if the player is now dead

bool removePlayerLife(){
    playerLives--;
    return playerLives == 0;
}

// player movement routines, limiting the motion to the screen

void movePlayerLeft() {
    if(playerX>0)
        playerX--;
}

void movePlayerRight() {
    if(playerX<7){
        playerX++;
    }
}

// draw the player (must be between clear()/swap())

void renderPlayer() {
    AberLED.set(playerX,7,GREEN);
}

// render lives in the LiveLost state as 3 lights
void renderLives(){
    AberLED.set(2,4,GREEN);       // left dot always green
    if(playerLives>1)             // middle dot green if lives>1
        AberLED.set(3,4,GREEN);
    else
        AberLED.set(3,4,RED);
    if(playerLives>2)             // right dot green if lives>2
        AberLED.set(4,4,GREEN);
    else
        AberLED.set(4,4,RED);
}

/**************************************************************************
 * 
 * The track model and its code
 * 
 **************************************************************************/
 
  
void drawDot(){
  int x;
  int y;
  AberLED.set(dotPos[x], dotPos[y], YELLOW);
}

// Checks if the dot is in the same position as a red block and
// check if the dot has reached the bottom of the screen. Both these
// events will be treated as a collision.
bool checkForDotCollision(){

  if(dotPos[Y] > 7){
    return true;
  }
int x;
int y;
  if(REDBLOCK == redBlocks[dotPos[x]][dotPos[y]]){
    return true;
  }

  else{
    return false;
  }
}

bool hasPlayerBeenHit() {

// return whether the block at the player's
// position is not empty - i.e. not 0

    return blocks[playerX][7]!=0;
}  

////////////////////////////////////////////
//
// Red Blocks code
//
////////////////////////////////////////////


//// To be called at every state change
//void clearScreenData(){
//  // Clears whole screen of any red blocks
//  // or state screen patterns
//  for(int x = 0; x < 8; x++){
//    for(int y = 0; y < 8; y++){
//      redBlocks[x][y] = EMPTY;
//      screenData[x][y] = 0;
//    }
//  }
//}


// Checks the redBlocks array at specific co-ordinates
bool checkForRedBlock(int x, int y){

  if(REDBLOCK == redBlocks[x][y]){
    return true;
  }

  else{
    return false;
  }
}

void drawRedBlocks(){

  for(int x = 2; x < 7; x++){
    for(int y = 0; y < 8; y++){
      if(checkForRedBlock(x,y)){
        AberLED.set(x, y, YELLOW);
      }
    }
  }
}

// This will check if a red block exists in a specific location
// and will assign a location for the red block at the location of
// a collision
void updateBlocks(){

  if(checkForDotCollision()){

    redBlocks[dotPos[X]][dotPos[Y]-1] = playerX;
  }

  if ( hasPlayerBeenHit()){
    gotoState(S_LIFELOST);
  }
}


//// Random obsticals are in an 8x8 array.
//  
//int dotPos[2] = {4, 0};
//
//// Variable that holds the status of the game
//bool lostGame = false;
//
// Initialize dot by moving to top of screen and assigning a new (random) x position
void initDot(){
  dotPos[Y] = 0;
  dotPos[X] = random(6);
}


// scrolling timer
#define SCROLLINTERVAL 1000       // put this line with the rest of the wall model
unsigned long lastScrollTime=0;


void scrolling(){
    for(int y=6;y>=0;y--){ // loop 6,5,4,3,2,1,0
        for(int x=0;x<8;x++){
            blocks[x][y+1] = blocks[x][y];
        }
    }
}


void generation(){              //the track
  int x = 1;
  for (int y=0; y<8; y++){
    blocks [x][y] = 2;
  }
  x = 6;
  for (int y=0; y<8; y++){
    blocks [x][y] = 2;
  }
  x = 0;
  for (int y=0; y<8; y++){
    blocks [x][y] = 0;
  }
  x = 7;
  for (int y=0; y<8; y++){
    blocks [x][y] = 0;
  }
}
//
//
//
//
// return true if the player has been hit
//bool hasPlayerBeenHit() {
//
//// return whether the block at the player's
//// position is not empty - i.e. not 0
//
//    return blocks[playerX][7]!=0;
//}   
//
//
// draw the blocks
void renderBlocks() {

    // AberLED.clear() must have been called before

    for(int y=0;y<8;y++){ // for all locations
        for(int x=0;x<8;x++) {
            switch(blocks[x][y]) { // look at the block type here
                case 0:  // don't draw anything, the location is empty
                    break;
                case 1:  // case 1 
                  AberLED.set(x,y,YELLOW);
                  break;
                case 2:  // ...into case 2, so both will draw as red
                    AberLED.set(x,y,RED);
                    break;
                default:
                  break;
            }
        }
    }
    // AberLED.swap() must be called later
}

/**************************************************************************
 * 
 * The main loop code
 * 
 **************************************************************************/

/*
 * 
 * The setup function to initialise everything
 * 
 */

 void setup(){
    AberLED.begin();
    Serial.begin(9600);
    gotoState(S_START);  // start in the Start state
    initPlayer();
    clearScreenData();
}

// To be called at every state change
void clearScreenData(){
  // Clears whole screen of any red blocks
  // or state screen patterns
  for(int x = 0; x < 8; x++){
    for(int y = 0; y < 8; y++){
      redBlocks[x][y] = EMPTY;
      screenData[x][y] = 0;
    }
  }
}

 
String bitTranslation(unsigned char num){   //translates an unsigned char to bits
  int theNums[] = {128,64,32,16,8,4,2,1};
  String result;
  for(int i=0;i<8;i++){
    if(num>=theNums[i]){
      num = num - theNums[i];
      result = result + '1';
    }
    else{
      result = result + '0';
    }
  }
  return result;
}


/*
 * The three main loop functions : handleInput, updateModel and render. Each
 * has a state machine switch inside it, doing different things for each state.
 */

void handleInput(){
    switch(state){
    case S_START:
        // on FIRE, restart the game by reinitialising the model
        // and going into the playing state.
        if(AberLED.getButtonDown(FIRE)){
            initPlayer();
            gotoState(S_PLAYING);
            clearScreenData();
        }   
        break;
        
    case S_PLAYING:
        // handle move/fire buttons
        if(AberLED.getButtonDown(LEFT))
            movePlayerLeft();
        if(AberLED.getButtonDown(RIGHT))
            movePlayerRight();
        break;
        
    case S_LIFELOST:
        break;
    case S_END:
        // fire button returns to start state
        if(AberLED.getButtonDown(FIRE)){
            gotoState(S_START);
             clearScreenData();
        }
        break;
        case S_PAUSE:
      if(AberLED.getButtonDown(FIRE)){
        timePaused = timePaused + getStateTime();
        state = 1;
      }
    default:
        Serial.println("Bad state in handleInput");
        break;
    }
}

// if the player has been hit, deduct a life and go to either
// the End or LifeLost state.
void checkPlayerHit(){
    if(hasPlayerBeenHit()){
        if(removePlayerLife()) // returns true if out of lives
            gotoState(S_END);
        else
            gotoState(S_LIFELOST);
    }
}

void updateModel(){
  

  unsigned long elapsedTime = millis() - lastMoveTime;
  if(elapsedTime > interval){
    lastMoveTime = millis();
    switch(state){
    case S_START:   //sets all the variables ready for the playing state
    interval = 250;
        score = 0;
        highScore = 0;
        flash = !flash;
        maxRan = 10;
        count = 0;
        break;
    case S_PLAYING:
        generation();
        renderBlocks();
         delay(20);
        // scroll the blocks every SCROLLINTERVAL milliseconds
        if(millis() - lastScrollTime > SCROLLINTERVAL) {
            lastScrollTime = millis();
          scrolling();
            if(interval<1){
          if(EEPROM.read(0)<score) EEPROM.write(0,score);
          highScore = EEPROM.read(0);
          state = 2;
          break;
        }
        if(hasPlayerBeenHit()) interval = interval - 50;
        if(hasPlayerBeenHit()&&interval<250) interval = interval + 25;
        interval = interval - (interval/256);
        count++;
        if(count==20||count==40||count==60){
          if(maxRan>2&&count==60)maxRan--;
          if(count==60)count=0;
          score++;
        }
            
           
        }
        // check the player hasn't collided
        checkPlayerHit();
        break;
    case S_LIFELOST:
    
        // if we're in this state for 2 seconds,
        // go back to Playing, clearing the screen
        // of blocks and bullets first.
        if(getStateTime()>2000){
          initPlayer1();
          clearScreenData();
          gotoState(S_PLAYING);
        }
        flash = !flash;
        break;
    case S_END:
        break;
    case S_PAUSE:
        flash = !flash;
        break;
    default:
        Serial.println("Bad state in update!");
        break;
    }
  }
}

// Function to Display a smiley face with two horizontal lines
// on the start screen
void drawStartStatePattern(){

//   Setting draw locations for the smiley face
   screenData[2][2] = 2;
   screenData[5][2] = 2;
  screenData[1][5] = 2;
  screenData[6][5] = 2;

  for(int index = 2; index < 6; index++){
    screenData[index][6] = 2;
  }

  //setting draw locations for the border
  for(int index = 0; index < 8; index++){
    screenData[index][0] = 1;
    screenData[index][7] = 1;
  }

  // Drawing the pattern
  for(int x = 0; x < 8; x++){
    for(int y = 0; y < 8; y++){
      if(2 == screenData[x][y]){
        AberLED.set(x, y, GREEN);
      }
      if(1 == screenData[x][y]){
        AberLED.set(x, y, RED);
      }
    }
  }
}




// draw a box of a given colour
void renderBox(int colour) {
    for(int i=0;i<8;i++){
        AberLED.set(0,i,colour); // left edge
        AberLED.set(7,i,colour); // right edge
        AberLED.set(i,0,colour); // top edge
        AberLED.set(i,7,colour); // bottom edge
    }
}

 
void render(){  //renders the displays for each state with functions helping out
    
    switch(state){
    case S_START:
        // just draw a green box
        drawStartStatePattern();
        break;
    case S_PLAYING:
        // draw the game 
        drawDot();
        renderBlocks();
        renderPlayer();
        break;
    case S_LIFELOST:
        // draw a yellow box and remaining lives
        renderBox(YELLOW);
        renderLives();
        break;
    case S_PAUSE: //displays the dots and player
      renderBlocks();
      if(flash)AberLED.set(pPos[0],pPos[1],3);
      break;
    case S_END:
        // draw a red box
        renderBox(RED);
        String points;
      points = bitTranslation(score);
      for(int i=0;i<8;i++){
        if(points[i]=='1') AberLED.set(i,0,2);
      }
      AberLED.set(4,2,3);
      AberLED.set(3,3,3);
      AberLED.set(5,3,3);
      AberLED.set(4,4,3);
      if(flash) AberLED.set(2,4,1);
      String topPoints;
      topPoints = bitTranslation(highScore);
      if(score==highScore){
        if(!flash){
          for(int i;i<8;i++){
            if(topPoints[i]=='1') AberLED.set(i,7,2);
          }
        }
      }
      else{
        for(int i=0;i<8;i++){
          if(topPoints[i]=='1') AberLED.set(i,7,2);
        }
      }
      break;
    default:
        Serial.println("Bad state in render!");
        break;
    }
}


// and we don't change anything below this point

void loop(){
    handleInput();
    updateModel();
    AberLED.clear();
    render();
    AberLED.swap();
}

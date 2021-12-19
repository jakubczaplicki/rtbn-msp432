// Derived from TI clock.c
// World Shapers, hand-held video game
// TI-RTOS+MSP432+MKII
// 1) Install CCS7.1
// 2) Install TI-RTOS for MSP432
// 3) Import this project

/* XDC module Headers */
#include <xdc/std.h>
#include <xdc/runtime/System.h>
#include "../inc/bsp.h"

/* BIOS/TIRTOS module Headers */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Semaphore.h>

/* Example/Board Header files */
#include "Board.h"
#include "random.h"
#include "images.h"
#include "score.h"
// Foreground threads
// task0 GameThread: runs game, reads joystick, draws LCD
// task1 MoveEnemiesThread: AI for enemies
// task2 SoundThread: used to make buzzing sounds

// Periodic task
// SlowPeriodicTask: adds enemies, handles buttons

Clock_Struct clk0Struct, clk1Struct;
#define TASKSTACKSIZE   512
Task_Struct tsk0Struct;
UInt8 tsk0Stack[TASKSTACKSIZE];
Task_Handle task0;
Task_Struct tsk1Struct;
UInt8 tsk1Stack[TASKSTACKSIZE];
Task_Handle task1;
Task_Struct tsk2Struct;
UInt8 tsk2Stack[TASKSTACKSIZE];
Task_Handle task2;

// semaphores
int32_t IntermissionFlag=1;  // pause game during intermissions
Semaphore_Struct MutexStruct;
Semaphore_Handle MutexHandle;


#define FIX 64    // 1/64 pixels

/*  ****************************************
    *x=0,y=0                      x=127,y=0*
    *                                      *
    *                                      *
    *                                      *
    *                                      *
    *                                      *
    *                                      *
    *                                      *
    *                                      *
    *                                      *
    *                                      *
    *x=0,y=127                  x=127,y=127*
    ****************************************
*/
// *************************** Capture image dimensions out of BMP**********
struct sprite{
  const unsigned short *ImagePt[2]; // two images, animated
  uint32_t AnimationIndex;    // index through images, 0 or 1
  uint32_t AnimationCount;    // count of outputs since last image change
  uint32_t AnimationDuration; // number of outputs per image
  const unsigned short *DeathImagePt;
  short x,y;   // lower left coordinate
  short fx,fy; // lower left coordinate in 1/FIX pixels
  short w,h;   // size of image
  short vx,vy; // motion, change in 1/FIX pixels per video frame
  unsigned short life; // >0 is alive, 0 is dead
};
typedef struct sprite sprite_t;
#define NUMSPRITES 45
#define SHIP 0
#define ROCKETMIN 1
#define ROCKETMAX 15
#define ENEMYMIN 16
#define ENEMYMAX 30
#define EMISSILEMIN 31
#define EMISSILEMAX 44
#define MISSILE2SHIP 5   // hit distance
#define ROCKET2ENEMY 5   // hit distance
#define ENEMY2SHIP 5     // hit distance
#define MISSILE2ROCKET 2 // hit distance
#define CuteSpeed (FIX/16)
#define EnemySpeed (FIX/8)
#define RocketSpeed (FIX*2)
sprite_t Things[NUMSPRITES];
struct level{
  uint32_t KillCount;             // kills required to advance to next level
  uint32_t EnemyMissileThreshold; // enemies fire if random number 0 to 65535 less than this number
  uint32_t EnemyThreshold;        // spawn an enemy if random number 0 to 65535 less than this number
  int32_t  EnemyMissileSpeed;     // fixed point speed
  uint32_t EnemyLife;             // number of hits required to kill enemy
  uint16_t Landcolor;
  uint16_t MaxLandHeight;
  int32_t  City;
};
typedef struct level level_t;
#define NUMLEVELS 5  // number of levels
level_t Levels[NUMLEVELS] = {
  {3,    0, 2048,  FIX/4, 1, LCD_GREEN,      60,0}, // 0: rolling hills
  {5,    0, 2048,  FIX/4, 1, LCD_BLUE,        8,0}, // 1: over lake
  {5,  512, 4096,  FIX/2, 2, LCD_ORANGE,     40,0}, // 2: low lands
  {8, 1024, 4096,  FIX,   2, LCD_LIGHTGREEN, 80,0}, // 3: mountains
  {8, 2048, 4096,  FIX,  10, LCD_GREY,       30,1}  // 4: city
};
int CurrentLevel = 0;  // index of current level
uint32_t KillsThisLevel = 0;
short DesiredPlace;  // place to move ship to
uint32_t Score=0;
#define SCREENWIDTH 128
#define SCREENHEIGHT 128

//sound thread********************
#define BUZZ100MS 1400
uint32_t buzzerDuration=0;
// ------------Sound_Shoot------------
// Make a sound for shoot
// Input: none
// Output: none
void Sound_Shoot(void){
  buzzerDuration=4*BUZZ100MS; // 0.4sec
}
void Sound_Killed(void){
  buzzerDuration=5*BUZZ100MS; // 0.5sec
}
void Sound_Explosion(void){
  buzzerDuration=10*BUZZ100MS; // 1sec
}
void Sound_MissileRocket(void){
  buzzerDuration=2*BUZZ100MS; // 0.2sec
}
void SoundThread(void){ // runs at about 1.4kHz
  buzzerDuration=0;
  while(1){
    if(buzzerDuration){
      BSP_Buzzer_Toggle(); // 700 Hz sound
      buzzerDuration--;
      Task_sleep(714 / Clock_tickPeriod);  /* We want to sleep for 713 microseconds */
    }else{
      Task_sleep(10000 / Clock_tickPeriod);  /* We want to sleep for 10ms */
    }
  }
}

uint32_t FindFreeRocket(void){
  uint32_t i;
  i = ROCKETMIN;
  while(Things[i].life){
    i++;
    if(i>ROCKETMAX) return 0; // no more player rockets can be created
  }
  return i;
}
uint32_t FindFreeEnemy(void){
  uint32_t i;
  i = ENEMYMIN;
  while(Things[i].life){
    i++;
    if(i>ENEMYMAX) return 0; // no more enemies can be created
  }
  return i;
}
uint32_t FindFreeEMissile(void){
  uint32_t i;
  i = EMISSILEMIN;
  while(Things[i].life){
    i++;
    if(i>EMISSILEMAX) return 0; // no more enemy missiles can be created
  }
  return i;
}
#define MAXTERRAIN 512
// Landscape numbers go from 5 to MaxLandHeight (level)
// drawn on bottom of screen as y-coor starting at 122
unsigned char Landscape[MAXTERRAIN];
unsigned char LandCount;    // counter to wait for next movement
unsigned char LandDuration; // number of screen frames for every movement

//------------------------------------------------------------------------
// move terrain left, rotating with random, prepopulated terrain
//------------------------------------------------------------------------
void MoveLand(void){
  uint32_t i,first;
  LandCount--; // decrement counter to wait for next movement
  // it's time to move land along Y axis
  if(LandCount == 0){
    LandCount = LandDuration;
    // scrolls actual terrain to the left
    first = Landscape[0];
    for(i=0; i<MAXTERRAIN-1; i++){
      Landscape[i] = Landscape[i+1];
    }
    Landscape[MAXTERRAIN-1] = first+Random()/64;  // rotate image
  }
}
//------------------------------------------------------------------------
// create a initial terrain
// inputs:
//    X speed for terrain
//  max height
//  city=1 for cityscape
//------------------------------------------------------------------------
void CreateLand(uint32_t duration, uint32_t max, int32_t city){
  uint32_t i; uint32_t Rnd,change,Range; int32_t t;
  Landscape[0] = 5; Range = 5;
  for(i=1; i<MAXTERRAIN-32; i++){
    if(city){
      if((i%Range)==0){
        Rnd = ((max-5)*Random16())/65536+5; // 5 to max
        Range = (4*Random16())/65536+5; // 5 to 9
      }
      Landscape[i] = Rnd;
    }else{

      Rnd = Random16();
      if((i%5)==0){
        Range = Random16(); // used to make terrain creation more random
      }
      // random Y screen coordinate, limited to Y=60 as max altitude
      change = (unsigned char) Rnd%(Range%3+2); // random number
      Landscape[i] = Landscape[i-1] + change - 1;
      if (Landscape[i] > max) {
        Landscape[i] = max;
      }
      if (Landscape[i] < 5 ) {
        Landscape[i] = 5;
      }
    }
  }
  for(i=MAXTERRAIN-32; i<MAXTERRAIN; i++) {
    t = ((i-(MAXTERRAIN-32))*(Landscape[0]-Landscape[MAXTERRAIN-33]))/32
       +Landscape[MAXTERRAIN-33] + (Random()/64);
    Landscape[i] = t - 1;
    if (Landscape[i] > max) {
      Landscape[i] = max;
    }
    if (Landscape[i] < 5 ) {
      Landscape[i] = 5;
    }
  }
  LandDuration = duration; // number of frames before moving
  LandCount = duration;
}
// returns maximum of |x1-x2| and |y1-y2| of centers of objects
uint32_t Distance(uint32_t n1, uint32_t n2){  // put your answer here
  int32_t x1,x2,y1,y2,d1,d2;
  x1 = Things[n1].x+Things[n1].w/2;
  y1 = Things[n1].y-Things[n1].h/2;
  x2 = Things[n2].x+Things[n2].w/2;
  y2 = Things[n2].y-Things[n2].h/2;
  if(x1>x2){
    d1 = x1-x2;
  }else{
    d1 = x2-x1;
  }
  if(y1>y2){
    d2 = y1-y2;
  }else{
    d2 = y2-y1;
  }
  if(d1>d2) return d1;
  return d2;
}

//------------------------------------------------------------------------
// draw the land on the screen
//------------------------------------------------------------------------
void ReDrawLand(uint16_t color, uint16_t bgcolor){
  int i;
  BSP_LCD_DrawFastVLine(0, 0, SCREENHEIGHT, bgcolor);
  for(i=1; i<SCREENWIDTH; i=i+1){
    BSP_LCD_DrawFastVLine(i, SCREENHEIGHT-Landscape[i], Landscape[i], color);
    BSP_LCD_DrawFastVLine(i, 0, SCREENHEIGHT-Landscape[i]-1, bgcolor);
  }
}
void DrawLand(uint16_t color, uint16_t bgcolor){
  int i; int16_t lasty,newy;
  for(i=1; i<SCREENWIDTH; i=i+1){
    lasty = SCREENHEIGHT-Landscape[i-1];
    newy = SCREENHEIGHT-Landscape[i];
    if(lasty < newy){ // up
      BSP_LCD_DrawFastVLine(i, lasty,  newy-lasty, bgcolor);
    }
    if(lasty > newy){ // down
      BSP_LCD_DrawFastVLine(i, newy+1, lasty-newy, color);
    }
  }
}

void DrawSprites(void){int i;
  Semaphore_pend(MutexHandle, BIOS_WAIT_FOREVER);
  for(i=0; i<NUMSPRITES; i++){
    if(Things[i].life){
      BSP_LCD_DrawBitmap(Things[i].x, Things[i].y, Things[i].ImagePt[Things[i].AnimationIndex], Things[i].w,Things[i].h);
      Things[i].AnimationCount--;
      if(Things[i].AnimationCount == 0){
        Things[i].AnimationCount = Things[i].AnimationDuration; // how many frames before change image
        Things[i].AnimationIndex ^= 0x01; // index goes 0,1,0,1,0,1...
      }
    }
  }
  Semaphore_post(MutexHandle);
}
void MissileHitsShip(void){ // check for enemy missiles hitting player ship
  uint32_t i,d;
  for(i=EMISSILEMIN; i<=EMISSILEMAX; i++){
    if(Things[i].life){
      d = Distance(SHIP,i); // distance from ship to missile
      if(d <= MISSILE2SHIP){
        if(Things[SHIP].life) Things[SHIP].life--; // lose one life
        if(Score > 10){
          Score = Score-10;
        }else{
          Score = 0;
        }
        Sound_Explosion();
        Things[i].life = 0;
        BSP_LCD_DrawBitmap(Things[i].x, Things[i].y, Things[i].DeathImagePt, Things[i].w,Things[i].h);
      }
    }
  }
}
void EnemyHitsShip(void){ // check for enemies hitting player ship
  uint32_t i,d;
  for(i=ENEMYMIN; i<=ENEMYMAX; i++){
    if(Things[i].life){
      d = Distance(SHIP,i); // distance from ship to enemy
      if(d <= ENEMY2SHIP){
        if((CurrentLevel==0)&&(KillsThisLevel==0)){
          Score = Score+100;  // good thing
        //  Sound_EyesOfTexasPieces();
        }else{
          if(Things[SHIP].life) Things[SHIP].life--; // lose one life
          if(Score > 10){
            Score = Score-10;
          }else{
            Score = 0;
          }
          Sound_Explosion();
        }
        Things[i].life = 0;
        BSP_LCD_DrawBitmap(Things[i].x, Things[i].y, Things[i].DeathImagePt, Things[i].w,Things[i].h);
      }
    }
  }
}
void ShipHitsLand(void){ // check for ship hitting land
  uint32_t t;
  int i;
  t = 0;
  for(i=(Things[SHIP].x); i<(Things[SHIP].x+Things[SHIP].w); i++){
    if((i >= 0) && (i < MAXTERRAIN)){
      if(Landscape[i] > t){ // find highest land under ship
        t = Landscape[i];   // max land Y under ship
      }
    }
  }
  t = SCREENHEIGHT -t; // screen position of highest land
  if(Things[SHIP].y >= t){
    Things[SHIP].fy = FIX*(t-1); // hit land, bounce off
    Things[SHIP].y = Things[SHIP].fy/FIX;
    Things[SHIP].vy = 0;
    if(Score >= 5){
      Score = Score-5;  // loose points for hitting land
    }else{
      Score = 0;
    }
    Sound_Explosion();
  }
}
void RocketHitsMissile(void){ // check for rockets hitting missiles
  uint32_t i,j,d;
  for(i=ROCKETMIN; i<=ROCKETMAX; i++){
    if(Things[i].life){
      for(j=EMISSILEMIN; j<=EMISSILEMAX; j++){
        if(Things[j].life){
          d = Distance(j,i); // distance from rocket to missile
          if(d <= MISSILE2ROCKET){
            Things[i].life = 0;      // both destroyed on collision
            Things[j].life = 0;
            Sound_MissileRocket();
            BSP_LCD_DrawBitmap(Things[i].x, Things[i].y, Things[i].DeathImagePt, Things[i].w,Things[i].h);
            BSP_LCD_DrawBitmap(Things[j].x, Things[j].y, Things[j].DeathImagePt, Things[j].w,Things[j].h);
          }
        }
      }
    }
  }
}
void RocketHitsEnemy(void){ // check for rockets hitting enemies
  uint32_t i,j,d;
  for(i=ROCKETMIN; i<=ROCKETMAX; i++){
    if(Things[i].life){
      for(j=ENEMYMIN; j<=ENEMYMAX; j++){
        if(Things[j].life){
          d = Distance(j,i); // distance from rocket to enemy
          if(d <= ROCKET2ENEMY){
            Things[j].life--;
            if(Things[j].life == 0){
              Score = Score+10;
              KillsThisLevel = KillsThisLevel+1;
              Sound_Killed();
              Things[j].life = 0;
              BSP_LCD_DrawBitmap(Things[j].x, Things[j].y, Things[j].DeathImagePt, Things[j].w,Things[j].h);
            }
            BSP_LCD_DrawBitmap(Things[i].x, Things[i].y, Things[i].DeathImagePt, Things[i].w,Things[i].h);
            Things[i].life = 0;
          }
        }
      }
    }
  }
}

void MoveSprites(void){int i;
  Semaphore_pend(MutexHandle, BIOS_WAIT_FOREVER);
  for(i=0; i<NUMSPRITES; i++){
    if(Things[i].life){
      Things[i].fx = Things[i].fx+Things[i].vx;
      Things[i].x = Things[i].fx/FIX;
      if((Things[i].x<0)||(Things[i].x>127)){
        Things[i].life = 0;
      }
      Things[i].fy = Things[i].fy+Things[i].vy;
      Things[i].y = Things[i].fy/FIX;
      if((Things[i].y<0)||(Things[i].y>127)){
        Things[i].life = 0;
      }
      if(Things[i].life==0){
        BSP_LCD_DrawBitmap(Things[i].x, Things[i].y, Things[i].DeathImagePt, Things[i].w,Things[i].h);
      }
    }
  }
  Semaphore_post(MutexHandle);
}
void CreateSprite(int i,
  const unsigned short *livePt, const unsigned short *livePt2,
  uint32_t animationDuration,
  const unsigned short *deadPt,
  short initx, short inity,
  short width, unsigned height,
  short initvx, short initvy, int alive){
  Semaphore_pend(MutexHandle, BIOS_WAIT_FOREVER);
  Things[i].ImagePt[0] = livePt;
  Things[i].ImagePt[1] = livePt2;
  Things[i].AnimationIndex = 0;
  Things[i].AnimationCount = animationDuration;
  Things[i].AnimationDuration = animationDuration;
  Things[i].DeathImagePt = deadPt;
  Things[i].x = initx;
  Things[i].y = inity;
  Things[i].fx = FIX*initx;
  Things[i].fy = FIX*inity;
  Things[i].w = width;
  Things[i].h = height;
  Things[i].vx = initvx;
  Things[i].vy = initvy;
  Things[i].life  = alive;
  Semaphore_post(MutexHandle);
}

int abs(int x){
  if(x<0) return -x;
  return x;
}


#define SCREENCHARCOLS 26                 // total number of character columns in the screen
#define SCREENCHARROWS 13                 // total number of character rows in the screen
uint16_t RevealScreen[SCREENCHARCOLS];    // set a bit in this array to reveal the character at corresponding row; LSB = row0
// Hide all characters for next call to revealchar().
// Input: none
// Output: none
void hideallscreen(void){
  uint16_t mask = ((1<<13) - 1);  // all rows
  int i;
  for(i=0; i<SCREENCHARCOLS; i=i+1){
    RevealScreen[i] &= ~mask;
  }
}
// Reveal all characters for next call to revealchar().
// Input: none
// Output: none
void revealallscreen(void){
  uint16_t mask = ((1<<13) - 1);  // all rows
  int i;
  for(i=0; i<SCREENCHARCOLS; i=i+1){
    RevealScreen[i] |= mask;
  }
}
// Attempt to reveal a character at the given row and
// column.  If the character has already been revealed, it
// will be printed.  If not, a random 16-bit number will be
// compared with the 'revealChance' parameter.  If the
// random number is smaller, the character will be revealed
// and printed.  Otherwise, the character will remain
// hidden, and a random character will be printed in the
// 16-bit color 'falseColor'.
// Input: x            columns from the left edge (0 to 25)
//        y            rows from the top edge (0 to 12)
//        pt           pointer to a character to be printed
//        textColor    16-bit color of the revealed character
//        falseColor   16-bit color of the hidden character
//        revealChance 16-bit chance of revealing character
// bgColor is Black and size is 1
// Output: none
void revealchar(uint16_t x, uint16_t y, char *pt, int16_t textColor, int16_t falseColor, uint16_t revealChance){
  if((x >= SCREENCHARCOLS) || (y >= SCREENCHARROWS)){
    return;
  }
  if(RevealScreen[x]&(1<<y)){
    // character is revealed
    BSP_LCD_DrawChar(x*6, y*10, *pt, textColor, LCD_BLACK, 1);
  } else{
    // character is hidden
    if(Random16() < revealChance){
      // reveal the character
      RevealScreen[x] |= (1<<y);
      BSP_LCD_DrawChar(x*6, y*10, *pt, textColor, LCD_BLACK, 1);
    } else{
      // character is still hidden
      BSP_LCD_DrawChar(x*6, y*10, ('0' + (Random16()%75)), falseColor, LCD_BLACK, 1);
    }
  }
}
#define NUMRANDCOLORS 12                  // length of random color array
const uint16_t RandomColors[NUMRANDCOLORS] = {
  0x801F,  // 255, 0, 128
  0x001F,  // 255, 0, 0
  0x041F,  // 255, 128, 0
  0x07FF,  // 255, 255, 0
  0x07F0,  // 128, 255, 0
  0x07E0,  // 0, 255, 0
  0x87E0,  // 0, 255, 128
  0xFFE0,  // 0, 255, 255
  0xFC00,  // 0, 128, 255
  0xF800,  // 0, 0, 255
  0xF810,  // 128, 0, 255
  0xF81F,  // 255, 0, 255
};
// Reveal a left vertical string on the given row.
// Input: y         rows from the top edge (0 to 12)
//        pt        pointer to a null terminated string to be printed
//        textColor 16-bit color of the characters
// bgColor is Black and size is 1
// Output: number of characters printed
uint32_t revealstring(uint16_t y, char *pt, int16_t textColor){
  uint32_t count = 0;
  uint16_t x = 0;
  char *c = pt;
  while(*c){
    x = x + 1;
    c = c + 1;
  }
  x = 0; //13 - x/2;
  if(y>12) return 0;
  while(*pt){
    revealchar(x, y, pt, textColor, RandomColors[Random()%NUMRANDCOLORS], 6554);
    pt++;
    x = x+1;
    if(x>25) return count;  // number of characters printed
    count++;
  }
  return count;  // number of characters printed
}
// Pause the game and display a message for intermission.
// Pause is crudely implemented by turning off interrupts
// and busy-waiting.
// Input: delay10ms  number of 10ms delays while message is on screen
//        level      level message index (<NUMLEVELS for level victory)
// Output: none
void Intermission(uint32_t delay10ms, int32_t level){
  int i;
  IntermissionFlag=0; // game engine stops, sounds continue
  hideallscreen();
  BSP_LCD_FillScreen(0x0000);            // set screen to black
  for(i=0; i<12; i=i+1){
    if(i == 11){
      revealallscreen();                // show any still hidden characters
    }
    if(level < 0){ // limit to 21 characters
      revealstring(3,  "The World Shapers",     LCD_WHITE);
      revealstring(5,  "We do love to fly",     LCD_WHITE);
      revealstring(7,  "On wallpapers",         LCD_WHITE);
      revealstring(9,  "Of beautiful sky",      LCD_WHITE);
    } else if(level == 0){
      revealstring(4,  "Oh, my dearest friend", LCD_YELLOW);
      revealstring(6,  "Why must we defend?",   LCD_YELLOW);
    } else if(level == 1){
      revealstring(5,  "You're shooting at us", LCD_ORANGE);
      revealstring(7,  "You're shooting at us", LCD_ORANGE);
      revealstring(9,  "Why make such a fuss?", LCD_ORANGE);
    } else if(level == 2){
      revealstring(3,  "MONSTERS MONSTERS",     LCD_LIGHTGREEN);
      revealstring(5,  "CAN WE DISCUSS!",       LCD_LIGHTGREEN);
      revealstring(7,  "SADDNESS HORRORS!",     LCD_LIGHTGREEN);
      revealstring(9,  "SOMEONE SAVE US!",      LCD_LIGHTGREEN);
    } else if(level == 3){
      revealstring(3,  "WALLS YOU GO THROUGH",  LCD_CYAN);
      revealstring(5,  "FINE CITIES AND THUS",  LCD_CYAN);
      revealstring(7,  "OH, WHAT CAN WE DO?",   LCD_CYAN);
      revealstring(9,  "SOMEONE SAVE US!",      LCD_ORANGE);
    } else if(level == 4){
      revealstring(2,  "YOU EVIL HUMAN!",       LCD_WHITE);
      revealstring(4,  "HAVE DESTROYED",        LCD_WHITE);
      revealstring(6,  "OUR FLEET AND ARE",     LCD_WHITE);
      revealstring(8,  "INVADING OUR HOME!",    LCD_WHITE);
      revealstring(10, "WE ARE VANQUISHED!",    LCD_WHITE);
    } else if(level == 5){
      revealstring(2,  "Oh, my dear friend",    LCD_YELLOW);
      revealstring(4,  "To you we now share",   LCD_YELLOW);
      revealstring(6,  "The special dividend",  LCD_YELLOW);
      revealstring(8,  "Of our best software!", LCD_YELLOW);
    } else if(level > 5){
      revealstring(1,  "YOU CAME WITH CAPRICE", LCD_WHITE);
      revealstring(3,  "YOU ACTED SO BADLY",    LCD_WHITE);
      revealstring(5,  "WE OFFERED YOU PEACE",  LCD_WHITE);
      revealstring(7,  "YOU FIRED SO GLADLY",   LCD_WHITE);
      revealstring(9,  "WE GIVE YOU RELEASE",   LCD_WHITE);
      revealstring(11, "VANQUISHED SO SADLY",   LCD_WHITE);
    }
    /* We want to sleep for 120000 microseconds */
      Task_sleep(120000 / Clock_tickPeriod);
  }
  if(level >= 4){
    Score_OutHorizontal(Score,122,141);
    while(1){};
  }
  /* We want to sleep for delay10ms*10ms */
  Task_sleep(10000*delay10ms / Clock_tickPeriod);
  BSP_LCD_FillScreen(0x0000);            // set screen to black
  // clear all player rockets and enemy missiles
  for(i=ROCKETMIN; i<=ROCKETMAX; i=i+1){
    Things[i].life = 0;
  }
  for(i=EMISSILEMIN; i<=EMISSILEMAX; i=i+1){
    Things[i].life = 0;
  }
  CreateLand(8,Levels[level+1].MaxLandHeight,Levels[level+1].City);
  ReDrawLand(Levels[level+1].Landcolor, LCD_BLACK);
  DrawSprites();
  Score_OutVertical(Things[SHIP].life,80,6);
  Score_OutVertical(Score,101,6);
  IntermissionFlag=1;            // game engine restarts, sounds continue
}

void GameThread(void){ // runs at about 30 Hz
  uint16_t x,y; uint8_t button;
  CreateSprite(SHIP,ship0,ship1,2,ship3,0,DesiredPlace,18,13,0,0,10);
  Intermission(300, -1);
  while(Things[SHIP].life){
    Task_sleep(33333 / Clock_tickPeriod);  /* We want to sleep for 33333 microseconds */

    if(IntermissionFlag){
      BSP_Joystick_Input(&x,&y,&button);
      DesiredPlace = ((113*(1023-y))>>10)+5; // 0 to 1023 mapped to 5 to 117
      if(DesiredPlace > Things[SHIP].y){
        if(DesiredPlace > Things[SHIP].y+1){
          Things[SHIP].vy = 2*FIX;  // move down fast
        }else{
          Things[SHIP].vy = FIX;    // move down
        }
      }else if(DesiredPlace < Things[SHIP].y){
        if(DesiredPlace < Things[SHIP].y-1){
          Things[SHIP].vy = -2*FIX;  // move up fast
        }else{
          Things[SHIP].vy = -FIX;    // move up
        }
      }else{
      Things[SHIP].vy = 0;  // stay put
      }
  //if joystick pushed to right and not too close to right, set velocity to go right
  // else if not too far left, set velocity go left
  // if at left velocity =0
      Things[SHIP].vx = 0;  // stay put
      if(x>650){
        if((Things[SHIP].x < (SCREENWIDTH - 1)) && (Things[SHIP].y > Landscape[Things[SHIP].x])){
          Things[SHIP].vx = FIX;  // move right
        }
      } else if(x<300){
        if((Things[SHIP].x > 10) && (Things[SHIP].y > Landscape[Things[SHIP].x])){
          Things[SHIP].vx = -FIX;  // move left
        }
      } else {
        if(Things[SHIP].x > 0){
          Things[SHIP].vx = -2*LandDuration;  // move left
        }
      }
      MoveSprites();
      MoveLand(); // terrain movement
      DrawLand(Levels[CurrentLevel].Landcolor, LCD_BLACK);
      EnemyHitsShip();
      RocketHitsMissile();
      RocketHitsEnemy();
      MissileHitsShip();
      ShipHitsLand();
      DrawSprites();
      Score_OutVertical(Things[SHIP].life,80,6);
      Score_OutVertical(Score,101,6);
      if((CurrentLevel==0)&&(Score>=1000)){
        Intermission(300, 5); // game win
      }
      if(KillsThisLevel >= Levels[CurrentLevel].KillCount){
        KillsThisLevel = 0;
        Things[SHIP].life++;
        Intermission(300, CurrentLevel);
        CurrentLevel = CurrentLevel + 1;
      }
    }
  }
  Intermission(300, 6); // end of game
  BIOS_exit(0);
}
//------------AddEnemy called to add an enemy to world***********
void AddEnemyTask(void){ uint32_t me; uint32_t t,j,initY,ok,trys;
  me = FindFreeEnemy();
  if(me==0){
    return; // no sprites left to use
  }
  t = 0;     // landscape 0 means bottom of screen
  for(j=SCREENWIDTH-20; j<SCREENWIDTH;j++){
     if(Landscape[j] > t){
       t = Landscape[j];  // max land height on right of screen
     }
  }
  t = SCREENHEIGHT-t; // screen y coordinate of highest land
  Things[me].x = 118; // start on right
  ok = 1;
  trys = 0;
  while(ok){
    ok = 0; // assume it is ok
    initY = ((t-15)*Random())/256+10; // 10 to t-5
    Things[me].y = initY;
    for(j=ENEMYMIN; j<=ENEMYMAX; j++){
      if(Things[j].life){
        if(Distance(me,j)<10) ok = 1; // too close to other enemies
      }
    }
    trys++;
    if(trys==40){
       return; // no room on screen
    }
  }
  if(Random() > 64){
    if(((CurrentLevel==0)&&(Score >= 500))||(CurrentLevel>3)){
      CreateSprite(me,big0,big1,10,big3,118,initY,21,21,-EnemySpeed,0,Levels[CurrentLevel].EnemyLife);
    }else{
      CreateSprite(me,bender0,bender1,10,bender3,118,initY,11,10,-EnemySpeed,0,Levels[CurrentLevel].EnemyLife);
    }
  }else{
    CreateSprite(me,cute0,cute1,10,cute3,118,initY,11,11,-CuteSpeed,0,Levels[CurrentLevel].EnemyLife);
  }
}
// AI of enemy, runs every 100ms
void MoveEnemiesThread(UArg arg0){uint32_t enemy; uint32_t t,j;
  while(1){
    Task_sleep(100000 / Clock_tickPeriod);  /* We want to sleep for 100000 microseconds */
    for(enemy=ENEMYMIN; enemy<=ENEMYMAX; enemy++){
       if(Things[enemy].life){ int dx,dy;
         if(CurrentLevel>1){ // move towards ship
           dx = Things[SHIP].fx - Things[enemy].fx;
           if(dx>0) dx=0; // can't back up
           dy = Things[SHIP].fy-8*FIX - Things[enemy].fy;
           while((dx*dx+dy*dy)>(EnemySpeed*EnemySpeed)){
              dx = (3*dx)/4;
              dy = (3*dy)/4;
           }
           Things[enemy].vx = dx;
           Things[enemy].vy = dy+(((FIX/2)*Random())/256 - FIX/4); // wiggles around
         }else{
           Things[enemy].vy = ((FIX)*Random())/256 - FIX/2; // wiggles around
         }
         t=0; // closest land to this enemy
         if(Things[enemy].x<=20){
           j = 0;
         }else{
           j = Things[enemy].x-20;
         }
         while(j <= (Things[enemy].x+Things[enemy].w)){
           if(Landscape[j] > t){
              t = Landscape[j];  // max land in front of enemy
           }
           j++;
         }
         t = SCREENHEIGHT - t ; // largest y allowed
         if(Things[enemy].y >= t-5){
           Things[enemy].vy = -FIX; // move up fast
         }else if(Things[enemy].y >= t-10){
           Things[enemy].vy = -FIX/2; // move up slower
        }
        if(Random16() < Levels[CurrentLevel].EnemyMissileThreshold){ // 0 to 65535
          j = FindFreeEMissile();
          if(j){
            int MissileSpeed = Levels[CurrentLevel].EnemyMissileSpeed;
            if(CurrentLevel<2){
              CreateSprite(j,missile0,missile1,10,missile3,Things[enemy].x-2,Things[enemy].y-2,5,5,-MissileSpeed,0,1);
            }else{ int dx,dy;
              dx = Things[SHIP].fx - Things[enemy].fx;
              dy = Things[SHIP].fy-8*FIX - Things[enemy].fy;
              while((dx*dx+dy*dy)>(MissileSpeed*MissileSpeed)){
                dx = (3*dx)/4;
                dy = (3*dy)/4;
              }
              CreateSprite(j,missile0,missile1,10,missile3,Things[enemy].x-2,Things[enemy].y-2,5,5,dx,dy,1);
            }
          }
        }
      }
    }
  }
}

//------------ButtonTask handles switch input -------
// *********ButtonTask*********
// periodic task, signaled on touch
//   with bouncing, may also be called on release
// checks the switches, creates player rockets
// Inputs:  none
// Outputs: none
void ButtonTask(void){uint32_t i;
uint8_t current;
uint8_t static last=1;
  current = BSP_Button1_Input();
  if((last)&&(current == 0)){     // Button1 was pressed
   // BSP_RGB_D_Set(1,0,0); // red
    i = FindFreeRocket();
    if(i){
      Sound_Shoot();
      if(Score > 1) Score--;
      CreateSprite(i,rocket0,rocket1,10,rocket3,Things[SHIP].x+8,Things[SHIP].y-5,8,3,RocketSpeed,0,1);
    }
  }
  if(current&&(last == 0)){ // release
  //  BSP_RGB_D_Set(0,0,0);
  }
  last = current;
}
//------------SlowPeriodicTask creates new enemies randomly, check for buttons -------
void SlowPeriodicTask(UArg arg0){
 // runs at about 10 Hz
  if(IntermissionFlag){      // halt game during intermissions
    if(Random16() < Levels[CurrentLevel].EnemyThreshold){ // 0 to 65535
       AddEnemyTask(); // create a new enemy if space, and if room
    }
    ButtonTask();
  }
}

/*
 *  ======== main ========
 */
int main(void){
  uint16_t x, y; uint8_t select; // joystick used to seed random number
    /* Construct BIOS Objects */
  Clock_Params clkParams;
  Task_Params tskParams;
  Semaphore_Params semParams;
  BSP_Clock_InitFastest();

  Board_initGeneral();  /* Call LaunchPad init functions */
  BSP_Button1_Init();
  BSP_Button2_Init();
  BSP_Joystick_Init();
  BSP_RGB_D_Init(0, 0, 0);
  BSP_Buzzer_Init2(); // simple version with no timer attached

  BSP_LCD_Init();
  BSP_LCD_FillScreen(BSP_LCD_Color565(0, 0, 0)); //black

  BSP_Joystick_Input(&x, &y,  &select);
  Random_Init(x+y+select);

// SlowPeriodicTask runs at 10 Hz
  Clock_Params_init(&clkParams);
  clkParams.period = 100000/Clock_tickPeriod; // 10 Hz GameTask;
  clkParams.startFlag = TRUE;  // run continuously
  Clock_construct(&clk1Struct, (Clock_FuncPtr)SlowPeriodicTask,
                    100000/Clock_tickPeriod, &clkParams);

  /* Construct GameThread thread */
  Task_Params_init(&tskParams);
  tskParams.stackSize = TASKSTACKSIZE;
  tskParams.stack = &tsk0Stack;
  tskParams.priority = 2; // middle priority
  Task_construct(&tsk0Struct, (Task_FuncPtr)GameThread, &tskParams, NULL);
  task0 = Task_handle(&tsk0Struct);  /* Obtain instance handle */

  /* Construct MoveEnemiesThread thread */
  Task_Params_init(&tskParams);
  tskParams.stackSize = TASKSTACKSIZE;
  tskParams.stack = &tsk1Stack;
  tskParams.priority = 1; // low priority
  Task_construct(&tsk1Struct, (Task_FuncPtr)MoveEnemiesThread, &tskParams, NULL);
  task1 = Task_handle(&tsk1Struct);  /* Obtain instance handle */

  /* Create sound with buzzer thread */
  Task_Params_init(&tskParams);
  tskParams.stackSize = TASKSTACKSIZE;
  tskParams.stack = &tsk2Stack;
  tskParams.priority = 3; // high priority
  Task_construct(&tsk2Struct, (Task_FuncPtr)SoundThread, &tskParams, NULL);
  task2 = Task_handle(&tsk2Struct);  /* Obtain instance handle */

  /* Construct a Semaphore object to be use as a resource lock, initial count 1 */
  Semaphore_Params_init(&semParams);
  Semaphore_construct(&MutexStruct, 1, &semParams);
  MutexHandle = Semaphore_handle(&MutexStruct);    /* Obtain instance handle */

  BIOS_start();    /* does not return */
  return(0);
}



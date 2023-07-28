/*
 * Copyright (c) 2020 by Miles C.
  An improved version of hill  run with interrupts to make code better.
  NOTICE DUCK_PIN IS 18 INSTEAD OF 4  because of interrupt capablilities!
  Hill run, a fun game where you jump over  hills and duck under crows.
  Wiring:
  Arduino +5V to breadboard power bus
  Arduino Ground to breadboard ground bus
  Jump button with 10k pull down to  ground, and to digital pin 2, and other side to +5V.
  Duck button with 10k pull  down to ground, and to digital pin 18, and other side to +5V.
  Passive buzzer  with one side on ground and one side on digital PWN pin 5.
  16x2 16-pin Lcd  screen with normal arduino wiring, as in the lesson for the lcd screen:
    -K  to ground
    -A to +5V
    -D4 through D7 to pins 9 through 12
    -E  to pin 8 
    -RW to ground
    -RS to pin 7
    -V0 to potentiometer output,  for brightness, potentiometer is connected to +5V and ground
    -VDD to +5V
    -VSS to ground
*/
#include <LiquidCrystal.h>
#include "pitches.h"
LiquidCrystal  lcd(7, 8, 9, 10, 11, 12);

const int JUMP_PIN = 2;
const int BUZZER_PIN  = 5;
const int DUCK_PIN = 18; //change to 3 if you want to use an UNO instead

const  int JUMP_PITCH = 2700; //sounds when button pressed
const int JUMP_PITCH_DURATION  = 50; //sounds when button pressed
const int DUCK_PITCH = 1350; //sounds when  button pressed
const int DUCK_PITCH_DURATION = 50; //sounds when button pressed
const  int DIE_PITCH = 200; //sounds on death
const int DIE_PITCH_DURATION = 500; //sounds  on death
const int TICKSPEED = 90; //ms per gametick, 1 gametick per hill move.
const  int JUMP_LENGTH = 3; //chars jumped over when jump is pressed.
const byte stickStep1[8]  = {
  B01110,
  B01110,
  B00101,
  B11111,
  B10100,
  B00110,
  B11001,
  B00001,
};
const byte stickStep2[8] = {
  B01110,
  B01110,
  B00101,
  B11111,
  B10100,
  B00110,
  B01011,
  B01000,
};
const  byte stickJump[8] = {
  B01110,
  B01110,
  B00100,
  B11111,
  B00100,
  B11111,
  B10001,
  B00000,
};
const byte stickDuck[8] = {
  B00000,
  B00000,
  B00000,
  B01110,
  B01110,
  B11111,
  B00100,
  B11111,
};
const  byte hill[8] = {
  B00000,
  B00100,
  B01010,
  B01110,
  B11101,
  B10101,
  B11001,
  B11111,
};
const byte crow1[8] = {
  B00111,
  B00100,
  B00110,
  B01111,
  B11111,
  B01111,
  B00110,
  B00111,
};
const  byte crow2[8] {
  B00111,
  B00110,
  B01111,
  B11111,
  B01111,
  B00110,
  B00110,
  B00111,
};

volatile int jumpPhase = JUMP_LENGTH  + 1;
int gameTick = 0;
int crowX = 40;
int hillX = 25;
bool playerY  = 0;
volatile bool ducking = LOW;
bool loopBreaker = 1;
bool crowGo = 0;
int  score = 0;

void setup() {
  pinMode(JUMP_PIN, INPUT);
  pinMode(BUZZER_PIN,  OUTPUT);
  lcd.begin(16, 2);
  lcd.createChar(0, hill);
  lcd.createChar(1,  stickStep1);
  lcd.createChar(2, stickStep2);
  lcd.createChar(3, stickJump);
  lcd.createChar(4, stickDuck);
  lcd.createChar(5, crow1);
  lcd.createChar(6,  crow2);
  attachInterrupt(digitalPinToInterrupt(JUMP_PIN), seeJumping, RISING);
  attachInterrupt(digitalPinToInterrupt(DUCK_PIN), seeDucking, CHANGE);
}

void  loop() {
  playerY = 0;
  if (jumpPhase < JUMP_LENGTH) {
    playerY =  1;
  }

  drawSprites();

  loopBreaker = 1;
  if (hillX < 16)  {
    if (crowX < hillX) {
      hillX += 8;
      loopBreaker = 0;
    }
    if (loopBreaker) {
      lcd.setCursor(hillX, 1);
      lcd.write((byte)0);
    }
  }
  if (hillX < 1) {
    if (jumpPhase < JUMP_LENGTH) {
      score++;
      hillX = 16 + rand() % 8;
    } else {
      endGame();
    }
  }
  if (crowX < 16) {
    lcd.setCursor(crowX, 0);
    if (gameTick %  8 < 4) {
      lcd.write((byte)5);
    } else {
      lcd.write((byte)6);
    }
  }
  if (crowX < 1) {
    if (ducking) {
      score++;
      crowX  = 24 + rand() % 16;
    } else {
      endGame();
    }
  }
  lcd.setCursor(0,  playerY);
  lcd.print(" ");
  jumpPhase++;
  hillX--;
  crowGo = !crowGo;
  crowX -= crowGo;
  gameTick++;
  delay(TICKSPEED);
}

void endGame()  {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Score: ");
  lcd.setCursor(7,  0);
  lcd.print(score);
  tone(BUZZER_PIN, DIE_PITCH, DIE_PITCH_DURATION);
  while (!digitalRead(JUMP_PIN)) {
    lcd.setCursor(0, 1);
    if (millis()  % 500 < 250) {
      lcd.print("Jump to Continue");
    } else {
      lcd.print("                ");
    }
  }
  lcd.clear();
  score = 0;
  hillX  = 25;
  crowX = 40;
}

void drawSprites() {
  lcd.setCursor(0, 1  - playerY);

  if (!ducking) {
    if (!playerY) {
      if ((gameTick  % 4) < 2 ) {
        lcd.write((byte)1);
      } else {
        lcd.write((byte)2);
      }
    } else {
      lcd.write((byte)3);
    }
  } else {
    lcd.write((byte)4);
  }
  lcd.setCursor(1, 1);
  lcd.print("               ");
  lcd.setCursor(1, 0);
  lcd.print("               ");
}
void seeJumping()  {
  if (jumpPhase > (JUMP_LENGTH + 2) && !ducking) {
    jumpPhase = 0;
    tone(BUZZER_PIN, JUMP_PITCH, JUMP_PITCH_DURATION);
  }

}
void seeDucking()  {
  ducking = digitalRead(DUCK_PIN);
  if (ducking) {
    jumpPhase = JUMP_LENGTH;
    tone(BUZZER_PIN, DUCK_PITCH, DUCK_PITCH_DURATION);
  }
}

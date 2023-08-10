#include <MIDI.h>
#include <LiquidCrystal.h>
#define NUM_ROWS 11
#define NUM_COLS 6
#define RELEASE_DELAY 230
#define NOTE_ON_VELOCITY 80
#define NOTE_OFF_VELOCITY 20

#define NUM_WA_ROWS 6
#define NUM_WA_COLUMNS 3

MIDI_CREATE_DEFAULT_INSTANCE();


byte lines[] = { B00000001, B00000010, B00000100, B00001000, B00010000, B00100000 };  // To control shift register, only one line powered at a time

// Pin Definitions

//I have a matrix of 11x6, where 11 are ROWS (READ) and 6 are COLUMNS (WRITE)
//Each Keyboard has a different matrix
//Colums use a pin for each line, since they need to check if

// Row Pins (They will READ if current is HIGH on their end)
const int row0Pin = A5;   //Cable 1
const int row1Pin = A4;   //Cable 2
const int row2Pin = 2;    //Cable 3
const int row3Pin = 3;    //Cable 4
const int row4Pin = 4;    //Cable 5
const int row5Pin = 5;    //Cable 6
const int row6Pin = 6;    //Cable 8
const int row7Pin = 7;    //Cable 10
const int row8Pin = 8;    //Cable 12
const int row9Pin = 9;    //Cable 14
const int row10Pin = 10;  //Cable 16

// Data Pins (They will provide power to the columns)
// 74HC595 pins
const int dataPin = 11;
const int latchPin = 12;
const int clockPin = 13;

const int dontCare = 3;


int fixIndex, fixArg;

int keyPressed[NUM_ROWS][NUM_COLS], keyLogMap[NUM_ROWS][NUM_COLS], keyToMidiMap[NUM_ROWS][NUM_COLS];
int* wrongActivation[NUM_WA_ROWS][NUM_WA_COLUMNS] = {
  { &keyPressed[3][4], &keyPressed[3][5], &keyPressed[3][0] },
  { &keyPressed[3][3], &keyPressed[3][4], &keyPressed[3][0] },
  { &keyPressed[3][2], &keyPressed[3][4], &keyPressed[3][0] },
  { &keyPressed[3][5], &keyPressed[3][0], &dontCare },
  { &keyPressed[3][1], &keyPressed[3][2], &dontCare },
  { &keyPressed[3][0], &keyPressed[3][1], &dontCare },
};


void setup() {
  Serial.begin(9600);
  Serial.println("Hello world");
  MIDI.begin();
  int note = 84;

  for (int rowCtr = 0; rowCtr < NUM_ROWS; ++rowCtr) {
    for (int colCtr = 0; colCtr < NUM_COLS; ++colCtr) {
      keyPressed[rowCtr][colCtr] = 0;
      // Keypressed: 2- Just pressed, 1- Still pressed, 0- Not pressed
      keyToMidiMap[rowCtr][colCtr] = note;
      keyLogMap[rowCtr][colCtr] = 0;
      note--;
    }
  }

  //Here I declare the keys that do not work correctly, if you dont have just delete keyLog completely
  keyLogMap[3][0] = 1;
  keyLogMap[3][1] = 1;
  keyLogMap[3][2] = 1;
  keyLogMap[3][3] = 1;
  keyLogMap[3][4] = 1;
  keyLogMap[3][1] = 1;





  // setup pins output/input mode
  pinMode(dataPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(latchPin, OUTPUT);


  pinMode(row0Pin, INPUT);
  digitalWrite(row0Pin, LOW);
  pinMode(row1Pin, INPUT);
  digitalWrite(row1Pin, LOW);
  pinMode(row2Pin, INPUT);
  pinMode(row3Pin, INPUT);
  pinMode(row4Pin, INPUT);
  pinMode(row5Pin, INPUT);
  pinMode(row6Pin, INPUT);
  pinMode(row7Pin, INPUT);
  pinMode(row8Pin, INPUT);
  pinMode(row9Pin, INPUT);
  pinMode(row10Pin, INPUT);
}

void loop() {
  for (int colCtr = 0; colCtr < NUM_COLS; colCtr++) {
    digitalWrite(latchPin, LOW);
    shiftOut(dataPin, clockPin, MSBFIRST, lines[colCtr]);
    digitalWrite(latchPin, HIGH);

    int rowValue[NUM_ROWS] = {
      digitalRead(row0Pin), digitalRead(row1Pin), digitalRead(row2Pin),
      digitalRead(row3Pin), digitalRead(row4Pin), digitalRead(row5Pin),
      digitalRead(row6Pin), digitalRead(row7Pin), digitalRead(row8Pin),
      digitalRead(row9Pin), digitalRead(row10Pin)
    };


    // process keys pressed
    for (int rowCtr = 0; rowCtr < NUM_ROWS; ++rowCtr) {
      if (keyPressed[rowCtr][colCtr] == 2) {
        keyPressed[rowCtr][colCtr] = 1;
        MIDI.sendNoteOn(keyToMidiMap[rowCtr][colCtr], NOTE_ON_VELOCITY, 1);
      }
      if (rowValue[rowCtr] != 0 && !keyPressed[rowCtr][colCtr]) {
        keyPressed[rowCtr][colCtr] = 2;
      } else if (rowValue[rowCtr] != 0 && keyPressed[rowCtr][colCtr] && keyLogMap[rowCtr][colCtr]) {
        keyLogMap[rowCtr][colCtr] = 1;  //This resets release countdown if active
      } else if (rowValue[rowCtr] == 0 && keyPressed[rowCtr][colCtr]) {
        if (keyLogMap[rowCtr][colCtr]) {  // check if key should be logged, keys with 0 will skip this
          if (keyLogMap[rowCtr][colCtr] > RELEASE_DELAY) {
            keyLogMap[rowCtr][colCtr] = 1;
            keyPressed[rowCtr][colCtr] = 0;
            MIDI.sendNoteOff(keyToMidiMap[rowCtr][colCtr], NOTE_OFF_VELOCITY, 1);

          } else {
            keyLogMap[rowCtr][colCtr]++;
          }
        } else {
          keyPressed[rowCtr][colCtr] = 0;
          MIDI.sendNoteOff(keyToMidiMap[rowCtr][colCtr], NOTE_OFF_VELOCITY, 1);
        }
      }
    }
  }
  for (fixIndex = 0; fixIndex < NUM_WA_ROWS; fixIndex++) {
    if (*wrongActivation[fixIndex][0] != 0) {
      int allPressed = 1;
      for (fixArg = 1; fixArg < NUM_WA_COLUMNS; fixArg++) {  //starts on the 2nd element of wrongActivation array
        if (*wrongActivation[fixIndex][fixArg] == 0) {       //It cancels it if conditions not fullfilled
          allPressed = 0;
          break;
        } else if (*wrongActivation[fixIndex][fixArg] == 3) {  // Reaches blank spaces and breaks loop
          break;
        }
      }
      if (allPressed) {
        while (fixArg > 0) {  //Set rest to false
          *wrongActivation[fixIndex][fixArg] = 0;
          fixArg--;
        }
      }
    }
  }
}

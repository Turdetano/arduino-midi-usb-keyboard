#include <MIDI.h>
#define NUM_ROWS 11
#define NUM_COLS 6
#define NOTE_ON_VELOCITY 80
#define NOTE_OFF_VELOCITY 20


MIDI_CREATE_DEFAULT_INSTANCE();


byte lines[] = { B00000001, B00000010, B00000100, B00001000, B00010000, B00100000 }; // To control shift register, to power one line at a time

// Pin Definitions

//I have a matrix of 11x6, where 11 are ROWS (READ) and 6 are COLUMNS (WRITE)
//Each Keyboard has a different matrix

// Row Pins (They will READ if current is HIGH on their end)
const int row0Pin = A0;   //Cable 1
const int row1Pin = A1;   //Cable 2
const int row2Pin = 2;    //Cable 3
const int row3Pin = 3;    //Cable 4
const int row4Pin = 4;    //Cable 5
const int row5Pin = 5;    //Cable 6
const int row6Pin = 6;    //Cable 8
const int row7Pin = 7;    //Cable 10
const int row8Pin = 8;    //Cable 12
const int row9Pin = 9;    //Cable 14
const int row10Pin = 10;  //Cable 16

// Data Pins (With the shift register we control the 6 column lines)
// 74HC595 pins
const int dataPin = 11;
const int latchPin = 12;
const int clockPin = 13;

boolean keyPressed[NUM_ROWS][NUM_COLS];
uint8_t keyToMidiMap[NUM_ROWS][NUM_COLS];

void setup() {
  MIDI.begin();
  int note = 84; // You'll likely need to change the starting note

  for (int rowCtr = 0; rowCtr < NUM_ROWS; ++rowCtr) {
    for (int colCtr = 0; colCtr < NUM_COLS; ++colCtr) {
      keyPressed[rowCtr][colCtr] = false;
      keyToMidiMap[rowCtr][colCtr] = note;     
      note--;
    }
  }

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
      if (rowValue[rowCtr] != 0 && !keyPressed[rowCtr][colCtr]) {
        keyPressed[rowCtr][colCtr] = true;
        MIDI.sendNoteOn(keyToMidiMap[rowCtr][colCtr], NOTE_ON_VELOCITY, 1);

      }

      else if (rowValue[rowCtr] == 0 && keyPressed[rowCtr][colCtr]) {
        keyPressed[rowCtr][colCtr] = false;
        MIDI.sendNoteOff(keyToMidiMap[rowCtr][colCtr], NOTE_OFF_VELOCITY, 1);
      }

      
    }
  }
}








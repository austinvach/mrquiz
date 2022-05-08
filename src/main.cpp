#include <Arduino.h>
#include <Keypad.h>
#define UPDATE_QUIZZES true
#ifdef UPDATE_QUIZZES
#include "update_quizzes.cpp"
#endif

const byte rows = 4;
const byte cols = 3;
char keys[rows][cols] = {
    {'1', '2', '3'},
    {'4', '5', '6'},
    {'7', '8', '9'},
    {'*', '0', '#'}};
byte rowPins[rows] = {5, 6, 7, 8};
byte colPins[cols] = {9, 10, 11};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, rows, cols);

void setup()
{
  Serial.begin(9600);
  if (UPDATE_QUIZZES == true)
  {
    updateQuizzes();
  }
}

void loop()
{
  char key = keypad.getKey();

  // put your main code here, to run repeatedly:
}
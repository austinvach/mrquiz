// TEST CODE IS 9986
// DISPLAY DIMENSIONS ARE 135 x 240
#include <ArduinoJson.h>
#include <TFT_eSPI.h>
#include <Keypad.h>
#include "Codes.h"
#define USB_POWER 1000 // battery percentage sentinel value to indicate USB power

TFT_eSPI tft = TFT_eSPI();

String keyVal;
String code;
String batteryPercentageText;
String currentScreen;
StaticJsonDocument<80> filter;
StaticJsonDocument<1600> doc;
JsonArray qaPairs;
bool isPretendSleeping = false;
bool isSleepEnabled = true;
bool secondaryTextVisible = false;
bool readyToPlay = false;
bool activeGame;
char key = 0;
int defaultTextSize = 2;
int currentQuestion = 0;
int totalQuestions;
int headerTextSize = defaultTextSize;
int headerTextYPosition = 10;
int primaryTextSize = 5;
int primaryTextYPosition = 40;
int secondaryTextSize = defaultTextSize;
int secondaryTextYPosition = 88;
int footerTextSize = defaultTextSize;
int footerTextYPosition = 116;
int sleepTimer = 90; // Time in seconds before the device goes to sleep
int32_t displayHeight = tft.width(); // Since we've rotated the screen 1/4 turn the height equals the width and visa versa
int32_t displayWidth = tft.height();  
long int lastBatteryCheck = 0;

long timeOfLastInteraction = millis();
const byte rows = 4;
const byte cols = 3;

char keys[rows][cols] = {
    {'1', '2', '3'},
    {'4', '5', '6'},
    {'7', '8', '9'},
    {'*', '0', '#'}};

byte rowPins[rows] = {21, 27, 26, 22}; //connect to the row pinouts of the keypad
byte colPins[cols] = {33, 32, 25};     //connect to the column pinouts of the keypad

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, rows, cols);

void updateBatteryStatus(bool force = false){
  // Serial.println("updateBatteryStatus()");
  if(!force && lastBatteryCheck != 0 && millis() - lastBatteryCheck < 5000) {
    return;
  }
  if(isPretendSleeping){
    return;
  }
  lastBatteryCheck = millis();
  const int batteryPercentage = getBatteryPercentage();
  batteryPercentageText = "";
  if (batteryPercentage == USB_POWER) {
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    batteryPercentageText = " USB";

  } else {
    if(batteryPercentage >= 60) {
      tft.setTextColor(TFT_GREEN, TFT_BLACK);
    } else if (batteryPercentage >= 20) {
      tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    } else {
      tft.setTextColor(TFT_RED, TFT_BLACK);
    }

    if(batteryPercentage != 100) {
      batteryPercentageText += " ";
      if (batteryPercentage < 10) {
        batteryPercentageText += " ";
      }
    }
    batteryPercentageText += String(batteryPercentage) + "%";
  }
  tft.setTextSize(2);
  tft.setCursor(192, headerTextYPosition);
  tft.print(batteryPercentageText);
}

unsigned int getBatteryPercentage(){
  // Serial.println("getBatteryPercentage()");
  const float batteryMaxVoltage = 4.2;
  const float batteryMinVoltage = 3.73;
  const float batteryAllowedRange = batteryMaxVoltage - batteryMinVoltage;
  const float batteryCurVAboveMin = getInputVoltage() - batteryMinVoltage;
  const int batteryPercentage = (int) (batteryCurVAboveMin / batteryAllowedRange * 100);
  if (batteryPercentage > 150) {
    return USB_POWER;
  }
  return max(min(batteryPercentage, 100), 0);
}

void reset(){
  // Serial.println("reset()");
  code = "";
  secondaryTextVisible = false;
  readyToPlay = false;
  // need to update to only reset active game by holding * for 3 seconds.
  activeGame = false;
  filter.clear();
  displayStartScreen();
}

bool isCodeValid (){
  // Serial.println("isCodeValid()");
  filter[String(code)] = true;
  // Deserialize the JSON document,
  DeserializationError error = deserializeJson(doc, codes, DeserializationOption::Filter(filter));
  // Test if parsing succeeds.
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
  }
  qaPairs = doc[String(code)].as<JsonArray>();
  if(qaPairs){
    readyToPlay = true;
    return true;  
  }
  else {
    return false;
  }
}

// Check whether the device should be put to sleep and put it to sleep if it should
void maybeSleepDevice() {
  // Serial.println("maybeSleepDevice()");
  if(isSleepEnabled && !isPretendSleeping) {
    long currentTime = millis();
    
    if(currentTime > (timeOfLastInteraction + sleepTimer * 1000)) {
      clearAllExceptBattery();
      reset();
      sleep();
      // The device wont charge if it is sleeping, so when charging, do a pretend sleep
      if(isPoweredExternally()) {
        isLilyGoKeyboard();
        isPretendSleeping = true;
        tft.fillScreen(TFT_BLACK);
      }
      else {
        if(isLilyGoKeyboard()) {
          esp_sleep_enable_ext0_wakeup(GPIO_NUM_25,1); //1 = High, 0 = Low
        } else {
          // Configure Touchpad as wakeup source
          touchAttachInterrupt(T3, callback, 40);
          esp_sleep_enable_touchpad_wakeup();
        }
        esp_deep_sleep_start();
      }
    }
  }
}

void callback(){
  // Serial.println("callback()");
}

float getInputVoltage(){
  // Serial.println("getInputVoltage()");
  delay(100);
  const uint16_t v1 = analogRead(34);
  return ((float) v1 / 4095.0f) * 2.0f * 3.3f * (1100.0f / 1000.0f);
}
// Does the device have external or internal power?
bool isPoweredExternally(){
  // Serial.println("isPoweredExternally()");
  float inputVoltage = getInputVoltage();
  if(inputVoltage > 4.5)
  {
    return true;
  }
  else
  {
    return false;
  } 
}

// Get the keypad type
boolean isLilyGoKeyboard(){
  // Serial.println("isLilyGoKeyboard()");
  if(colPins[0] == 33) {
    return true;
  }
  return false;
}

void clearHeader(){
  // Serial.println("clearHeader()");
  tft.setTextSize(headerTextSize);
  tft.fillRect(0, headerTextYPosition, 180, tft.fontHeight(), TFT_BLACK);
}

void setHeaderText(String s){
  // Serial.println("setHeaderText()");
  clearHeader();
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setCursor(0, headerTextYPosition);
  tft.print(s);
}

void clearPrimaryText(){
  // Serial.println("clearPrimaryText()");
  tft.setTextSize(primaryTextSize);
  tft.fillRect(0, primaryTextYPosition, displayWidth, tft.fontHeight(), TFT_BLACK);
}

void setPrimaryText(String s, uint16_t c = TFT_BLUE){
  // Serial.println("setPrimaryText()");
  clearPrimaryText();
  tft.setTextColor(c, TFT_BLACK);
  tft.setCursor(0, primaryTextYPosition);
  tft.print(s);
}

void clearSecondaryText(){
  // Serial.println("clearSecondaryText()");
  tft.setTextSize(secondaryTextSize);
  tft.fillRect(0, secondaryTextYPosition, displayWidth, tft.fontHeight(), TFT_BLACK);
}

void setSecondaryText(String s){
  // Serial.println("setSecondaryText()");
  clearSecondaryText();
  tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
  tft.setCursor(0, secondaryTextYPosition);
  tft.print(s);
}

void clearFooter(){
  // Serial.println("clearFooter()");
  tft.setTextSize(footerTextSize);
  tft.fillRect(0, footerTextYPosition, displayWidth, tft.fontHeight(), TFT_BLACK);
}

void setFooterText(String s){
  // Serial.println("setFooterText()");
  clearFooter();
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setCursor(0, footerTextYPosition);
  tft.print(s);
}

void clearAllExceptBattery(){
  // Serial.println("clearAllExceptBattery()");
  clearHeader();
  clearPrimaryText();
  clearSecondaryText();
  clearFooter();
}

void startGame(){
  // Serial.println("sleep()");
  activeGame = true;
  currentQuestion = 0;
  totalQuestions = qaPairs.size();
  clearAllExceptBattery();
  tft.setTextSize(5);
  tft.setTextColor(TFT_BLUE, TFT_BLACK);
  tft.setCursor(0, primaryTextYPosition);
  int i = 0;
  while(i < 8){
    tft.print("#");
    delay(50);
    i++;
  }
  clearAllExceptBattery();
  
  Serial.println("Quiz " + code + " has " + qaPairs.size() + " questions.");
  setHeaderText("QUESTION");
  setPrimaryText("01"); // Switch this to the header when they press a button.
  // setSecondaryText("PRESS * TO CLEAR"); // Switch this to the header when they press a button.
  setFooterText("KEY IN YOUR ANSWER");
  // for(JsonArray qaPair : qaPairs) {
  //   Serial.print(qaPair[0].as<int>());
  //   Serial.print(" : ");
  //   Serial.println(qaPair[1].as<int>());
  // }
}

void sleep(){
  // Serial.println("sleep()");
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(5);
  tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
  tft.setCursor(0, primaryTextYPosition);
  tft.print("  ");
  int i = 0;
  while(i < 4){
    tft.print("Z");
    delay(250);
    i++;
  }
  clearPrimaryText();
}

void setBatteryPercentage(){
  // Serial.println("setBatteryPercentage()");
  updateBatteryStatus(true);
}

void displayStartScreen(){
  // Serial.println("displayStartScreen()");
  currentScreen = "startScreen";
  setBatteryPercentage();
  setHeaderText("");
  setPrimaryText("MR.QUIZ");
  setSecondaryText("LEARNING COMPANION");
  setFooterText("ENTER CODE TO BEGIN");
}

void displayCodeEntryScreen(){
  // Serial.println("displayCodeEntryScreen()");
  if(currentScreen != "codeEntryScreen"){
    currentScreen = "codeEntryScreen";
    setHeaderText("CODE");
    setPrimaryText("");
    setSecondaryText("");
    setFooterText("PRESS * TO RESET");
    tft.setCursor(0, primaryTextYPosition);
  }
  if(code.length() < 4){
    code = code + key;
    setPrimaryText(code);
  }
  if (code.length() == 4){
    if(secondaryTextVisible != true){
      secondaryTextVisible = true;
      if(isCodeValid()){
        setPrimaryText(code,TFT_GREEN);
        setSecondaryText("IS VALID");
        setFooterText("PRESS # TO START");
      }
      else {
        setPrimaryText(code, TFT_RED);
        setSecondaryText("IS INVALID");
      }
    }
    else {
      Serial.println("IGNORING " + keyVal);
    }
  } 
}

void setup(){
  Serial.begin(115200);
  // Serial.println("setup()");
  tft.init();
  tft.setRotation(1);
  tft.invertDisplay(true);
  displayStartScreen();
}

void loop(){
  if (keypad.getKeys()){
    // Scan the whole key list.
    for (int i=0; i<LIST_MAX; i++){
      // Check that a key's state has changed to PRESSED.
      if (keypad.key[i].stateChanged && keypad.key[i].kstate == PRESSED){
        // Set 'key' variable to the value of the key that was pressed.
        key = keypad.key[i].kchar;
        keyVal = String(key);
        // Set 'timeOfLastInteraction' to current time in ms.
        timeOfLastInteraction = millis();
        // Check if the device is sleeping.
        if(isPretendSleeping == true){
          // If it is wake it up.
          isPretendSleeping = false;
          displayStartScreen();
        }
        else if (key == '*'){
          if(currentScreen != "startScreen"){
            reset();
          }
        }
        else if (key == '#'){
          if(readyToPlay && !activeGame){
            startGame();
          }
          else {
            Serial.println("IGNORING " + keyVal);
          }
        }
        else {
          displayCodeEntryScreen();
        }
      }
    }   
  }
  maybeSleepDevice();
  updateBatteryStatus();
}
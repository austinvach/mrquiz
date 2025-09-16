#include <ArduinoJson.h>
#include <TFT_eSPI.h>
#include <Keypad.h>
#include "Codes.h"
#define USB_POWER 1000 // battery percentage sentinel value to indicate USB power
#define HAPTIC_PIN 17  // Pin for haptic feedback
#define PIEZO_PIN 2   // Pin for piezo speaker

TFT_eSPI tft = TFT_eSPI();

String keyVal;
String code;
String userInput;
String expectedResponse;
String batteryPercentageText;
String currentScreen;
String currentQuestion;
JsonDocument filter;
JsonDocument temp;
JsonDocument doc;
JsonArray qaPairs;
bool geoSafariMode = false;
bool pretendSleeping;
bool secondaryTextVisible;
bool readyToPlay;
bool activeGame;
bool readyForNewQuestion;
bool readyForNewInput;
char key = 0;
int attempts;
int currentQuestionIndex;
int totalQuestions;
int headerTextSize = 2;
int headerTextYPosition = 10;
int primaryTextSize = 5;
int primaryTextYPosition = 40;
int secondaryTextSize = 2;
int secondaryTextYPosition = 88;
int footerTextSize = 2;
int footerTextYPosition = 116;
int sleepTimer = 60; // Time in seconds before the device goes to sleep
int32_t displayHeight = tft.width();
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
byte rowPins[rows] = {21, 27, 26, 22};
byte colPins[cols] = {33, 32, 25};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, rows, cols);

// Variables for non-blocking haptic feedback
unsigned long hapticStartTime = 0;
bool hapticActive = false;
const unsigned long HAPTIC_DURATION = 200; // Haptic pulse duration in ms

// Variables for non-blocking keypress sound
unsigned long keySoundStartTime = 0;
bool keySoundActive = false;
const unsigned long KEY_SOUND_DURATION = 200; // Keypress sound duration in ms

// Variables for non-blocking valid input sound
unsigned long validSoundStartTime = 0;
bool validSoundPending = false;

// Variables for non-blocking invalid input sound
unsigned long invalidSoundStartTime = 0;
bool invalidSoundPending = false;

// Variables for non-blocking transition handling
bool transitionActive = false;
int animIndex = 0;
unsigned long animNextStep = 0;
int soundNoteIndex = 0;
unsigned long soundNextNote = 0;
bool soundActive = false;
const int transitionMelody[] = { 
  622,  // E5# (F5b)
  554,  // C5# (D5b)
  698,  // F5
  831,  // G5# (A5b)
  622,  // D5# (E5b)
  494,  // B4
  523,  // C5
  784,  // G5
  988,  // B5
  880,  // A5
  932,  // A5# (B5b)
  587,  // D5
  740   // F5# (G5b)
};
const unsigned long NOTE_DURATION = 70;  // ms
const unsigned long SILENCE_DURATION = 10;  // ms
const unsigned long ANIM_STEP_DURATION = 130;  // ms

class QAP {
  public:
    int questionNumber;
    int geoSafariNumber;
    int answer;
};

QAP objects[26];

void updateBatteryStatus(bool force = false){
  if (transitionActive) {
    return; // Skip battery update during animation to avoid cursor interference
  }
  if(!force && lastBatteryCheck != 0 && millis() - lastBatteryCheck < 5000) {
    return;
  }
  if(pretendSleeping){
    return;
  }
  lastBatteryCheck = millis();
  const int batteryPercentage = getBatteryPercentage();
  batteryPercentageText = "";
  if(batteryPercentage < 5){
    tft.setTextColor(TFT_RED, TFT_BLACK);
    batteryPercentageText = " LOW";
  }
  tft.setTextSize(2);
  tft.setCursor(192, headerTextYPosition);
  tft.print(batteryPercentageText);
}

unsigned int getBatteryPercentage(){
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

void resetVariables(){
  code = "";
  secondaryTextVisible = false;
  readyToPlay = false;
  activeGame = false;
  attempts = 0;
  filter.clear();
}

bool isCodeValid(){
  filter[String(code)] = true;
  DeserializationError error = deserializeJson(doc, codes, DeserializationOption::Filter(filter));
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
  }
  qaPairs = doc[String(code)].as<JsonArray>();
  if(qaPairs){
    int numObjects = qaPairs.size();
    for (int i = 0; i < numObjects; i++) {
      QAP newObj;
      newObj.questionNumber = i + 1;
      newObj.geoSafariNumber = qaPairs[i][0].as<int>();
      newObj.answer = qaPairs[i][1].as<int>();
      objects[i] = newObj;
    }
    shuffleQAPairs(objects, numObjects);
    readyToPlay = true;
    return true;
  }
  return false;
}

void maybeSleepDevice(){
  if(!pretendSleeping) {
    long currentTime = millis();
    if(currentTime > (timeOfLastInteraction + sleepTimer * 1000)) {
      resetVariables();
      sleep();
      if(isPoweredExternally()) {
        isLilyGoKeyboard();
        pretendSleeping = true;
        tft.fillScreen(TFT_BLACK);
      }
      else {
        if(isLilyGoKeyboard()) {
          esp_sleep_enable_ext0_wakeup(GPIO_NUM_25,1);
        } else {
          touchAttachInterrupt(T3, callback, 40);
          esp_sleep_enable_touchpad_wakeup();
        }
        esp_deep_sleep_start();
      }
    }
  }
}

void callback(){}

float getInputVoltage(){
  delay(100);
  const uint16_t v1 = analogRead(34);
  return ((float) v1 / 4095.0f) * 2.0f * 3.3f * (1100.0f / 1000.0f);
}

bool isPoweredExternally(){
  float inputVoltage = getInputVoltage();
  if(inputVoltage > 4.5)
  {
    return true;
  }
  return false;
}

bool isLilyGoKeyboard(){
  if(colPins[0] == 33) {
    return true;
  }
  return false;
}

void clearHeader(){
  tft.setTextSize(headerTextSize);
  tft.fillRect(0, headerTextYPosition, 180, tft.fontHeight(), TFT_BLACK);
}

void setHeaderText(String s){
  clearHeader();
  tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
  tft.setCursor(0, headerTextYPosition);
  tft.print(s);
}

void clearPrimaryText(){
  tft.setTextSize(primaryTextSize);
  tft.fillRect(0, primaryTextYPosition, displayWidth, tft.fontHeight(), TFT_BLACK);
}

void setPrimaryText(String s, uint16_t c = TFT_BLUE){
  clearPrimaryText();
  tft.setTextColor(c, TFT_BLACK);
  tft.setCursor(0, primaryTextYPosition);
  tft.print(s);
}

void clearSecondaryText(){
  tft.setTextSize(secondaryTextSize);
  tft.fillRect(0, secondaryTextYPosition, displayWidth, tft.fontHeight(), TFT_BLACK);
}

void setSecondaryText(String s){
  clearSecondaryText();
  tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
  tft.setCursor(0, secondaryTextYPosition);
  tft.print(s);
}

void setSecondaryTextWithStarAction(String s){
  clearSecondaryText();
  tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
  tft.setCursor(0, secondaryTextYPosition);
  tft.print("PRESS ");
  tft.print("* ");
  tft.print("TO " + s);
}

void clearFooter(){
  tft.setTextSize(footerTextSize);
  tft.fillRect(0, footerTextYPosition, displayWidth, tft.fontHeight(), TFT_BLACK);
}

void setFooterText(String s){
  clearFooter();
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setCursor(0, footerTextYPosition);
  tft.print(s);
}

void setFooterTextWithStarAction(String s){
  clearFooter();
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setCursor(0, footerTextYPosition);
  tft.print("PRESS ");
  tft.print("* ");
  tft.print("TO " + s);
}

void setFooterTextWithPoundAction(String s){
  clearFooter();
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setCursor(0, footerTextYPosition);
  tft.print("PRESS ");
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.print("# ");
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.print("TO " + s);
}

void clearAllExceptBattery(){
  clearHeader();
  clearPrimaryText();
  clearSecondaryText();
  clearFooter();
}

void startGame(){
  activeGame = true;
  currentQuestionIndex = 0;
  totalQuestions = qaPairs.size();
  startQuestionTransition();
}

void playQuestionTransitionSound(){
  const int melody[] = { 
    622,  // E5# (F5b)
    554,  // C5# (D5b)
    698,  // F5
    831,  // G5# (A5b)
    622,  // D5# (E5b)
    494,  // B4
    523,  // C5
    784,  // G5
  988,  // B5
  880,  // A5
  932,  // A5# (B5b)
  587,  // D5
  740   // F5# (G5b)
};
const int duration = 75; // Duration for all notes in milliseconds
for (int i = 0; i < 13; i++) {
  tone(PIEZO_PIN, melody[i], duration);
  delay(duration + 10);
}
noTone(PIEZO_PIN);
}

void startTransitionSound() {
soundNoteIndex = 0;
soundActive = true;
unsigned long now = millis();
if (soundNoteIndex < 13) {
  tone(PIEZO_PIN, transitionMelody[soundNoteIndex], NOTE_DURATION);
  soundNextNote = now + NOTE_DURATION + SILENCE_DURATION;
}
}

void showQuestionScreen(){
if(currentQuestionIndex < totalQuestions){
  attempts = 0;
  currentScreen = "questionScreen";
  clearAllExceptBattery();
  setHeaderText("QUESTION");
  QAP current = objects[currentQuestionIndex];
  if(geoSafariMode){
    currentQuestion = current.geoSafariNumber;
  }
  else {
    currentQuestion = current.questionNumber;
  }
  expectedResponse = current.answer;
  setPrimaryText(currentQuestion);
  setFooterText("KEY IN THE ANSWER");
}
else if (currentQuestionIndex == totalQuestions){
  playEndOfGameSound();
  currentScreen = "endScreen";
  clearAllExceptBattery();
  setPrimaryText("THE END");
  setFooterTextWithStarAction("RESET");
}
}

void playEndOfGameSound(){
Serial.println("END OF GAME");
}

void sleep(){
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

void showStartScreen(){
currentScreen = "startScreen";
setHeaderText("");
setPrimaryText("MR.QUIZ");
if(geoSafariMode == true){
  setSecondaryText("GEOSAFARI MODE");
}
else{
  setSecondaryText("LEARNING TOGETHER");
}
setFooterText("ENTER CODE TO BEGIN");
playStartUpSound();
}

void showCodeEntryScreen(){
currentScreen = "codeEntryScreen";
setHeaderText("CODE");
setPrimaryText("");
setSecondaryText("");
setFooterTextWithStarAction("RESET");
}

void printCodeToScreen(){
if(code.length() < 4){
  code = code + key;
  setPrimaryText(code, TFT_WHITE);
}
if (code.length() == 4 && !secondaryTextVisible){
  if(isCodeValid()){
    setPrimaryText(code, TFT_GREEN);
    setSecondaryText("IS VALID");
    setFooterTextWithPoundAction("START");
    validSoundPending = true;
    validSoundStartTime = millis() + KEY_SOUND_DURATION + 100;
  }
  else {
    noTone(PIEZO_PIN); // Stop any ongoing sound
    keySoundActive = false; // Reset key sound state
    setPrimaryText(code, TFT_RED);
    setSecondaryText("IS INVALID");
    playInvalidInputSound(); // Play invalid sound immediately
    secondaryTextVisible = true;
  }
  secondaryTextVisible = true;
}
}

void printUserInputToScreen(){
if(userInput.length() == 0){
  setHeaderText("QUESTION " + currentQuestion);
  setSecondaryTextWithStarAction("CLEAR");
  setFooterTextWithPoundAction("SUBMIT");
}
if(userInput.length() < 2){
  userInput = userInput + key;
  setPrimaryText(userInput, TFT_WHITE);
}
}

void setup(){
Serial.begin(115200);
keypad.addEventListener(keypadEvent);
tft.init();
tft.setRotation(1);
tft.invertDisplay(true);
updateBatteryStatus(true);
tft.fillScreen(TFT_BLACK);
pinMode(PIEZO_PIN, OUTPUT);
pinMode(HAPTIC_PIN, OUTPUT);
showStartScreen();
}

void playStartUpSound(){
noTone(PIEZO_PIN); // Stop any ongoing sound
const int melody[] = {740, 587};
const int durations[] = {140, 200};
for (int i = 0; i < 2; i++) {
  tone(PIEZO_PIN, melody[i], durations[i]);
  delay(durations[i] + 10);
}
noTone(PIEZO_PIN);
}

void playKeyPressSound() {
tone(PIEZO_PIN, 440, KEY_SOUND_DURATION); // Play 440 Hz for 200 ms
keySoundStartTime = millis();
keySoundActive = true;
triggerHapticResponse();
}

void loop(){
unsigned long now = millis();

// Handle pending valid input sound
if (validSoundPending && now >= validSoundStartTime) {
  playValidInputSound();
  validSoundPending = false;
}

// Handle pending invalid input sound
if (invalidSoundPending && now >= invalidSoundStartTime) {
  playInvalidInputSound();
  invalidSoundPending = false;
}

// Non-blocking transition handling
if (transitionActive) {
  // Advance animation
  if (now >= animNextStep) {
    if (animIndex < 8) {
      tft.setTextSize(5);
      tft.setTextColor(TFT_BLUE, TFT_BLACK);
      tft.print("#");
      animIndex++;
      animNextStep = now + ANIM_STEP_DURATION;
    }
  }

  // Advance sound
  if (soundActive && now >= soundNextNote) {
    soundNoteIndex++;
    if (soundNoteIndex < 13) {
      tone(PIEZO_PIN, transitionMelody[soundNoteIndex], NOTE_DURATION);
      soundNextNote = now + NOTE_DURATION + SILENCE_DURATION;
    } else {
      noTone(PIEZO_PIN);
      soundActive = false;
    }
  }

  // Check if both are done
  if (!soundActive && animIndex >= 8) {
    transitionActive = false;
    currentScreen = "questionScreen";
    attempts = 0;
    if (currentQuestionIndex < totalQuestions) {
      QAP current = objects[currentQuestionIndex];
      if (geoSafariMode) {
        currentQuestion = current.geoSafariNumber;
      } else {
        currentQuestion = current.questionNumber;
      }
      expectedResponse = current.answer;
      clearAllExceptBattery();
      setHeaderText("QUESTION");
      setPrimaryText(currentQuestion);
      setFooterText("KEY IN THE ANSWER");
    } else if (currentQuestionIndex == totalQuestions) {
      playEndOfGameSound();
      currentScreen = "endScreen";
      clearAllExceptBattery();
      setPrimaryText("THE END");
      setFooterTextWithStarAction("RESET");
    }
  }
}

// Manage haptic feedback timing
if (hapticActive && now - hapticStartTime >= HAPTIC_DURATION) {
  digitalWrite(HAPTIC_PIN, LOW);
  hapticActive = false;
}

// Manage keypress sound timing
if (keySoundActive && now - keySoundStartTime >= KEY_SOUND_DURATION) {
  noTone(PIEZO_PIN); // Stop the keypress sound
  keySoundActive = false;
}

// Check if keys have been pressed
if (keypad.getKeys()){
  for (int i = 0; i < LIST_MAX; i++){
    if (keypad.key[i].stateChanged && keypad.key[i].kstate == PRESSED){
      key = keypad.key[i].kchar;
      keyVal = String(key);
      timeOfLastInteraction = millis();
      if (key == '*'){
        if (currentScreen == "codeEntryScreen"){
          resetVariables();
          showStartScreen();
        }
        else if (currentScreen == "questionScreen"){
          playKeyPressSound();
          if (readyForNewInput) {
            readyForNewInput = false;
          }
          if(!readyForNewQuestion){
            userInput = "";
            setHeaderText("QUESTION");
            setPrimaryText(currentQuestion);
            setSecondaryText("");
            setFooterText("KEY IN THE ANSWER");
          }
        }
        else if (currentScreen == "endScreen"){
          resetVariables();
          clearAllExceptBattery();
          playTransitionAnimation();
          showStartScreen();
        }
      }
      else if (key == '#'){
        if (currentScreen == "codeEntryScreen" && readyToPlay){
          startGame();
        }
        else if (currentScreen == "questionScreen"){
          if (readyForNewQuestion) {
            readyForNewQuestion = false;
            startQuestionTransition();
          }
          else if (userInput == expectedResponse){
            setPrimaryText(userInput, TFT_GREEN);
            setSecondaryText("THAT'S CORRECT!");
            readyForNextQuestion();
            playCorrectAnswerSound();
          }
          else if (userInput.length() > 0){
            playInvalidInputSound();
            attempts++;
            setPrimaryText(userInput, TFT_RED);
            if (attempts < 3) {
              setSecondaryText("TRY AGAIN");
              setFooterTextWithStarAction("CLEAR");
              readyForNewInput = true;
              userInput = "";
            }else{
              setSecondaryText("THE ANSWER IS " + expectedResponse);
              readyForNextQuestion();
            }
          }
        }
      }
      else {
        if (currentScreen == "startScreen"){
          showCodeEntryScreen();
          printCodeToScreen();
          playKeyPressSound();
        }
        else if (currentScreen == "codeEntryScreen"){
          String tempCode = code + key; // Temporary code to check length
          if (tempCode.length() < 4) {
            printCodeToScreen();
            playKeyPressSound();
          } else {
            // For the 4th digit, check validity before playing keypress sound
            code = tempCode; // Update code to include new key
            printCodeToScreen();
            if (isCodeValid()) {
              playKeyPressSound(); // Play A4 for valid code
            } else {
              // Invalid code: keypress sound is skipped, invalid sound plays in printCodeToScreen
            }
          }
        }
        else if (currentScreen == "questionScreen"){
          if(!readyForNewQuestion && !readyForNewInput){
            printUserInputToScreen();
            playKeyPressSound();
          }
        }
      }
    }
  }
}
updateBatteryStatus();
}

void triggerHapticResponse(){
digitalWrite(HAPTIC_PIN, HIGH); // Turn on haptic motor
hapticStartTime = millis();
hapticActive = true;
}

void playCorrectAnswerSound(){
noTone(PIEZO_PIN); // Stop any ongoing sound
int melody[] = {440, 587, 659, 740};
int durations[] = {120, 120, 120, 200};
for (int i = 0; i < 4; i++) {
  tone(PIEZO_PIN, melody[i], durations[i]);
  delay(durations[i] + 10);
}
noTone(PIEZO_PIN);
}

void playValidInputSound(){
noTone(PIEZO_PIN); // Stop any ongoing sound
int melody[] = {523, 659};
int durations[] = {120, 200};
for (int i = 0; i < 2; i++) {
  tone(PIEZO_PIN, melody[i], durations[i]);
  delay(durations[i] + 10);
}
noTone(PIEZO_PIN);
}

void playInvalidInputSound() {
tone(PIEZO_PIN, 220, 200); // Play A3 (220 Hz) for 200 ms
delay(210); // Wait for note duration plus a small gap
noTone(PIEZO_PIN); // Stop sound
}

void readyForNextQuestion(){
setFooterTextWithPoundAction("CONTINUE");
readyForNewQuestion = true;
userInput = "";
currentQuestionIndex++;
}

void playTransitionAnimation(){
clearAllExceptBattery();
tft.setTextSize(5);
tft.setTextColor(TFT_BLUE, TFT_BLACK);
tft.setCursor(0, primaryTextYPosition);
int i = 0;
while(i < 8){
  tft.print("#");
  delay(40);
  i++;
}
}

void startTransitionAnimation() {
clearAllExceptBattery();
tft.setTextSize(5);
tft.setTextColor(TFT_BLUE, TFT_BLACK);
tft.setCursor(0, primaryTextYPosition);
animIndex = 0;
if (animIndex < 8) {
  tft.print("#");
  animIndex++;
}
animNextStep = millis() + ANIM_STEP_DURATION;
}

void startQuestionTransition() {
startTransitionSound();
startTransitionAnimation();
transitionActive = true;
}

void keypadEvent(KeypadEvent key){
if (keypad.getState() == HOLD && key == '*' && currentScreen != "startScreen"){
  resetVariables();
  playTransitionAnimation();
  showStartScreen();
}
if (keypad.getState() == HOLD && key == '#' && currentScreen == "startScreen"){
  geoSafariMode = !geoSafariMode;
  if(geoSafariMode == true){
    setSecondaryText("GEOSAFARI MODE");
  }
  else{
    setSecondaryText("LEARNING COMPANION");
  }
}
}

void shuffleQAPairs(QAP objects[], int numObjects){
for (int i = numObjects - 1; i > 0; i--) {
  int j = random(i + 1);
  QAP temp = objects[i];
  objects[i] = objects[j];
  objects[j] = temp;
}
}
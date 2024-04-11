let keyVal = "";
let code = "";
let userInput = "";
let expectedResponse = "";
let currentScreen = "";
let currentQuestion = "";
let filter = {}; // JSON object
let temp = {}; // JSON object
let doc = {}; // JSON object
let qaPairs = []; // JSON array

// I need to find a way to preserve geoSafariMode state in durable memory.
let geoSafariMode = false;
let secondaryTextVisible = false;
let readyToPlay = false;
let activeGame = false;
let readyForNewQuestion = false;
let readyForNewInput = false;
let key = 0;
let attempts;
let currentQuestionIndex;
let totalQuestions;
let headerTextSize = 2;
let headerTextYPosition = 10;
let primaryTextSize = 5;
let primaryTextYPosition = 40;
let secondaryTextSize = 2;
let secondaryTextYPosition = 88;
let footerTextSize = 2;
let footerTextYPosition = 116;
let sleepTimer = 60; // Time in seconds before the device goes to sleep
let displayHeight; // Since we've rotated the screen 1/4 turn the height equals the width and visa versa
let displayWidth;  
let lastBatteryCheck = 0;
let timeOfLastInteraction = Date.now();
const rows = 4;
const cols = 3;

window.onload = function() {
    document.getElementById('codeEntryInput').focus();
};

class QAP {
    constructor() {
        this.questionNumber;
        this.geoSafariNumber;
        this.answer;
    }
}

let objects = Array(26).fill().map(() => new QAP());

function resetVariables() {
    // console.log("resetVariables()");
    code = "";
    secondaryTextVisible = false;
    readyToPlay = false;
    activeGame = false;
    attempts = 0;
    filter = {};
}

function isCodeValid() {
    // console.log("isCodeValid()");
    filter[code] = true;
    // Deserialize the JSON document,
    let error = false;
    try {
        doc = JSON.parse(codes, (key, value) => {
            if (filter[key]) {
                return value;
            }
        });
    } catch (e) {
        console.log(`JSON.parse() failed: ${e}`);
        error = true;
    }
    qaPairs = doc[code];
    if (qaPairs) {
        let numObjects = qaPairs.length;
        for (let i = 0; i < numObjects; i++) {
            let newObj = new QAP();
            newObj.questionNumber = i + 1;
            newObj.geoSafariNumber = qaPairs[i][0];
            newObj.answer = qaPairs[i][1];
            objects[i] = newObj;
        }
        // Shuffle the array of objects
        shuffleQAPairs(objects, numObjects);
        readyToPlay = true;
        return true;
    }
    return false;
}

function clearHeader() {
    // console.log("clearHeader()");
    tft.setTextSize(headerTextSize);
    tft.fillRect(0, headerTextYPosition, 180, tft.fontHeight(), "black");
}

function setHeaderText(s) {
    // console.log("setHeaderText()");
    clearHeader();
    tft.setTextColor("darkgrey", "black");
    tft.setCursor(0, headerTextYPosition);
    tft.print(s);
}

function clearPrimaryText() {
    // console.log("clearPrimaryText()");
    tft.setTextSize(primaryTextSize);
    tft.fillRect(0, primaryTextYPosition, displayWidth, tft.fontHeight(), "black");
}

function setPrimaryText(s, c = "blue") {
    // console.log("setPrimaryText()");
    clearPrimaryText();
    tft.setTextColor(c, "black");
    tft.setCursor(0, primaryTextYPosition);
    tft.print(s);
}

function clearSecondaryText() {
    // console.log("clearSecondaryText()");
    tft.setTextSize(secondaryTextSize);
    tft.fillRect(0, secondaryTextYPosition, displayWidth, tft.fontHeight(), "black");
}

function setSecondaryText(s) {
    // console.log("setSecondaryText()");
    clearSecondaryText();
    tft.setTextColor("darkgrey", "black");
    tft.setCursor(0, secondaryTextYPosition);
    tft.print(s);
}

function setSecondaryTextWithStarAction(s) {
    clearSecondaryText();
    tft.setTextColor("darkgrey", "black");
    tft.setCursor(0, secondaryTextYPosition);
    tft.print("PRESS ");
    tft.print("* ");
    tft.print("TO " + s);
}

function clearFooter() {
    tft.setTextSize(footerTextSize);
    tft.fillRect(0, footerTextYPosition, displayWidth, tft.fontHeight(), "black");
}

function setFooterText(s) {
    clearFooter();
    tft.setTextColor("white", "black");
    tft.setCursor(0, footerTextYPosition);
    tft.print(s);
}

function setFooterTextWithStarAction(s) {
    clearFooter();
    tft.setTextColor("white", "black");
    tft.setCursor(0, footerTextYPosition);
    tft.print("PRESS ");
    tft.print("* ");
    tft.print("TO " + s);
}

function setFooterTextWithPoundAction(s) {
    clearFooter();
    tft.setTextColor("white", "black");
    tft.setCursor(0, footerTextYPosition);
    tft.print("PRESS ");
    tft.setTextColor("green", "black");
    tft.print("# ");
    tft.setTextColor("white", "black");
    tft.print("TO " + s);
}

function clearAllExceptBattery() {
    clearHeader();
    clearPrimaryText();
    clearSecondaryText();
    clearFooter();
}

function startGame() {
    activeGame = true;
    currentQuestionIndex = 0;
    totalQuestions = qaPairs.length;
    playTransitionAnimation();
    showQuestionScreen();
}

function playQuestionTransitionSound() {
    console.log("QUESTION TRANSITION");
}

function showQuestionScreen() {
    playQuestionTransitionSound();
    if(currentQuestionIndex < totalQuestions) {
        attempts = 0;
        currentScreen = "questionScreen";
        clearAllExceptBattery();
        setHeaderText("QUESTION");
        let current = objects[currentQuestionIndex];
        if(geoSafariMode) {
            currentQuestion = current.geoSafariNumber;
        } else {
            currentQuestion = current.questionNumber;
        }
        expectedResponse = current.answer;
        setPrimaryText(currentQuestion);  
        setFooterText("KEY IN THE ANSWER");
    } else if (currentQuestionIndex === totalQuestions) {
        playEndOfGameSound();
        currentScreen = "endScreen";
        clearAllExceptBattery();
        setPrimaryText("THE END");
        setFooterTextWithStarAction("RESET");
    }
}

function playEndOfGameSound() {
    console.log("END OF GAME");
}

function sleep() {
    tft.fillScreen("black");
    tft.setTextSize(5);
    tft.setTextColor("darkgrey", "black");
    tft.setCursor(0, primaryTextYPosition);
    tft.print("  ");
    let i = 0;
    while(i < 4) {
        tft.print("Z");
        delay(250);
        i++;
    }
    clearPrimaryText();
}

function showStartScreen() {
    currentScreen = "startScreen";
    setHeaderText("");
    setPrimaryText("MR.QUIZ");
    if(geoSafariMode === true) {
        setSecondaryText("GEOSAFARI MODE");
    } else {
        setSecondaryText("LEARNING TOGETHER");
    }
    setFooterText("ENTER CODE TO BEGIN");
}

function showCodeEntryScreen() {
    currentScreen = "codeEntryScreen";
    setHeaderText("CODE");
    setPrimaryText("");
    setSecondaryText("");
    setFooterTextWithStarAction("RESET");
}

function printCodeToScreen() {
    if(code.length < 4) {
        code = code + key;
        setPrimaryText(code, "white");
    }
    if (code.length === 4) {
        if(secondaryTextVisible !== true) {
            if(isCodeValid()) {
                playValidInputSound();
                setPrimaryText(code, "green");
                setSecondaryText("IS VALID");
                setFooterTextWithPoundAction("START");
            } else {
                playInvalidInputSound();
                setPrimaryText(code, "red");
                setSecondaryText("IS INVALID");
            }
            secondaryTextVisible = true;
        }
    } 
}

function printUserInputToScreen() {
    if(userInput.length === 0) {
        setHeaderText("QUESTION " + currentQuestion);
        setSecondaryTextWithStarAction("CLEAR");
        setFooterTextWithPoundAction("SUBMIT");
    }
    if(userInput.length < 2) {
        userInput = userInput + key;
        setPrimaryText(userInput, "white");
    }
}

function setup() {
    console.log("setup()");
    keypad.addEventListener('keypress', keypadEvent); // Add an event listener for this keypad
    tft.init();
    tft.setRotation(1);
    tft.invertDisplay(true);
    updateBatteryStatus(true);
    tft.fillScreen("black");
    showStartScreen();
    playStartUpSound();
}

function playStartUpSound() {
    console.log("START UP");
}

function playKeyPressSound() {
    console.log("KEY PRESS");
}

function loop() {
    // Check if keys have been pressed
    if (keypad.getKeys()) {
        // Scan the whole key list.
        for (let i = 0; i < LIST_MAX; i++) {
            // Find the keys whose state has changed to PRESSED.
            if (keypad.key[i].stateChanged && keypad.key[i].kstate === PRESSED) {
                // Set 'key' variable to the value of the key that was pressed.
                key = keypad.key[i].kchar;
                keyVal = String(key);
                // Set 'timeOfLastInteraction' to current time in ms.
                timeOfLastInteraction = Date.now();
                if (key === '*') {
                    playKeyPressSound();
                    if (currentScreen === "codeEntryScreen") {
                        resetVariables();
                        showStartScreen();
                    } else if (currentScreen === "questionScreen") {
                        if (readyForNewInput) {
                            readyForNewInput = false;
                        }
                        if (!readyForNewQuestion) {
                            userInput = "";
                            setHeaderText("QUESTION");
                            setPrimaryText(currentQuestion);
                            setSecondaryText("");
                            setFooterText("KEY IN THE ANSWER");
                        }
                    } else if (currentScreen === "endScreen") {
                        resetVariables();
                        clearAllExceptBattery();
                        playTransitionAnimation();
                        showStartScreen();
                    }
                } else if (key === '#') {
                    if (currentScreen === "codeEntryScreen" && readyToPlay) {
                        startGame();
                    } else if (currentScreen === "questionScreen") {
                        if (readyForNewQuestion) {
                            readyForNewQuestion = false;
                            showQuestionScreen();
                        } else if (userInput === expectedResponse) {
                            playCorrectAnswerSound();
                            setPrimaryText(userInput, "green");
                            setSecondaryText("THAT'S CORRECT!");
                            readyForNextQuestion();
                        } else if (userInput.length > 0) {
                            playInvalidInputSound();
                            attempts++;
                            setPrimaryText(userInput, "red");
                            if (attempts < 3) {
                                setSecondaryText("TRY AGAIN");
                                setFooterTextWithStarAction("CLEAR");
                                readyForNewInput = true;
                                userInput = "";
                            } else {
                                setSecondaryText("THE ANSWER IS " + expectedResponse);
                                readyForNextQuestion();
                            }
                        }
                    }
                } else {
                    playKeyPressSound();
                    if (currentScreen === "startScreen") {
                        showCodeEntryScreen();
                        printCodeToScreen();
                    } else if (currentScreen === "codeEntryScreen") {
                        printCodeToScreen();
                    } else if (currentScreen === "questionScreen") {
                        if (!readyForNewQuestion && !readyForNewInput) {
                            printUserInputToScreen();
                        }
                    }
                }
            }
        }
    }
    updateBatteryStatus();
}

function playCorrectAnswerSound() {
    console.log("CORRECT ANSWER");
}

function playValidInputSound() {
    console.log("VALID INPUT");
}

function playInvalidInputSound() {
    console.log("INVALID INPUT");
}

function readyForNextQuestion() {
    setFooterTextWithPoundAction("CONTINUE");
    readyForNewQuestion = true;
    userInput = "";
    currentQuestionIndex++;
}

function playTransitionAnimation() {
    clearAllExceptBattery();
    tft.setTextSize(5);
    tft.setTextColor("blue", "black");
    tft.setCursor(0, primaryTextYPosition);
    let i = 0;
    while (i < 8) {
        tft.print("#");
        setTimeout(() => {}, 40);
        i++;
    }
}

function keypadEvent(key) {
    if (keypad.getState() === HOLD && key === '*' && currentScreen !== "startScreen") {
        resetVariables();
        playTransitionAnimation();
        showStartScreen();
    }
    if (keypad.getState() === HOLD && key === '#' && currentScreen === "startScreen") {
        geoSafariMode = !geoSafariMode;
        if (geoSafariMode === true) {
            setSecondaryText("GEOSAFARI MODE");
        } else {
            setSecondaryText("LEARNING COMPANION");
        }
    }
}

function shuffleQAPairs(objects, numObjects) {
    // Shuffle the array using the Fisher-Yates algorithm
    for (let i = numObjects - 1; i > 0; i--) {
        let j = Math.floor(Math.random() * (i + 1));
        let temp = objects[i];
        objects[i] = objects[j];
        objects[j] = temp;
    }
}
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
let inputAlert = false;
let attempts;
let currentQuestionIndex;
let totalQuestions;
let validCodes = [];
const inputElement = document.getElementById('inputElement');
const submitButton = document.getElementById('submitButton');
const questionScreen = document.getElementById('questionScreen');
const inputLabel = document.getElementById('inputLabel');
let audio;

class QuestionAnswerPair {
    constructor() {
        this.questionNumber;
        this.geoSafariNumber;
        this.answer;
    }
}

window.onload = function() {
    console.log("onload()");
    fetchCodes();
    initializeAudio();
    loadStartScreen();
    document.addEventListener('keydown', function(event) {
        handleKeyPress(event.key);
    });
    inputElement.addEventListener('input', function() {
        validateInput(inputElement.value);
    })
    inputElement.focus();
};

// Prevent inputElement from losing focus
document.addEventListener('mousedown', function(event) {
    if (event.target !== inputElement) {
        event.preventDefault();
        inputElement.focus();
    }
});

function fetchCodes() {
    console.log("fetchCodes()");
    fetch('codes.json')
        .then(response => response.json())
        .then(data => {
            validCodes = Object.keys(data);
            console.log('Valid Codes:', validCodes);
        })
        .catch(error => console.error('Error fetching codes:', error));
}

function initializeAudio() {
    console.log("initializeAudio()");
    audioPop = new Audio('assets/pop3.mp3');
    audio = new Audio('assets/pop3.mp3');
}

function validateInput(inputValue) {
    console.log("validateInput()");
    if (currentScreen === "startScreen") {
        if (inputValue.length < 4 && inputAlert) {
            resetView();
        } else if (inputValue.length === 4) {
            isCodeValid(inputValue);
        }
    }
}

function resetView() {
    console.log("resetView()");
    inputElement.classList.remove('input-success');
    inputElement.classList.remove('input-error');
    submitButton.classList.remove('btn-success');
    submitButton.disabled = true;
    inputLabel.classList.add('invisible');
    if (currentScreen === "startScreen") {
        setPrimaryText("ENTER CODE TO BEGIN");
    }
    
    inputAlert = false;
}

let objects = Array(26).fill().map(() => new QuestionAnswerPair());

function resetVariables() {
    console.log("resetVariables()");
    code = "";
    secondaryTextVisible = false;
    readyToPlay = false;
    activeGame = false;
    attempts = 0;
    filter = {};
}

function isCodeValid(code) {
    console.log("isCodeValid()");
    if (validCodes.includes(code)) {
        inputElement.classList.add('input-success')
        inputLabel.textContent = 'Press SUBMIT to Begin';
        inputLabel.classList.remove('invisible');
        submitButton.disabled = false;
        submitButton.classList.add('btn-success');
        fetchQAPairs(code); 
    } else {
        inputElement.classList.add('input-error')
        inputLabel.textContent = 'TRY AGAIN';
        submitButton.disabled = true;
        submitButton.classList.remove('btn-success');
    }
    inputAlert = true;
}

function setPrimaryText(s, c = "blue") {
    console.log("setPrimaryText()");
    const primaryText = document.getElementById('primaryText');
    primaryText.textContent = s;
}

function setInputLabel(s) {
    console.log("setInputLabel()");
    const inputLabel = document.getElementById('inputLabel');
    inputLabel.textContent = s;
}

function startGame() {
    console.log("startGame()");
    activeGame = true;
    currentQuestionIndex = 0;
    totalQuestions = qaPairs.length;
    // playTransitionAnimation();
    showQuestionScreen();
}

function showQuestionScreen() {
    console.log("showQuestionScreen()");
    // Remove the hidden class from the questionScreen element
    playQuestionTransitionSound();
    if(currentQuestionIndex < totalQuestions) {
        attempts = 0;
        currentScreen = "questionScreen";
        // setHeaderText("QUESTION");
        let current = objects[currentQuestionIndex];
        if(geoSafariMode) {
            currentQuestion = current.geoSafariNumber;
        } else {
            currentQuestion = current.questionNumber;
        }
        expectedResponse = current.answer;
        setPrimaryText("QUESTION", currentQuestion);
        setInputLabel("KEY IN THE ANSWER");  
        // setFooterText("KEY IN THE ANSWER");
    } else if (currentQuestionIndex === totalQuestions) {
        playEndOfGameSound();
        currentScreen = "endScreen";
        setPrimaryText("THE END");
        setFooterTextWithStarAction("RESET");
    }
}



function sleep() {
    console.log("sleep()");
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
}

function loadStartScreen() {
    console.log("loadStartScreen()");
    playStartUpSound();
    currentScreen = "startScreen";
    setPrimaryText("ENTER CODE TO BEGIN");
    if(geoSafariMode === true) {
        setInputLabel("GEOSAFARI MODE");
    }
}

function printCodeToScreen() {
    console.log("printCodeToScreen()");
    code = inputElement.value;
    // inputElement.classList.remove('input-success');
    // inputElement.classList.remove('input-error');
    // submitButton.classList.remove('btn-success');
    // submitButton.disabled = true;
    // inputLabel.classList.add('invisible');
    // // Check if the input value is in the array of valid codes
    // if (inputValue.length === 4) {
    //     isCodeValid();
    // }

    if(code.length < 4) {
        console.log("Code: " + code);
        // setPrimaryText(code, "white");
    }
    if (code.length === 4) {
        console.log("Code: " + code);
        if(secondaryTextVisible !== true) {
            if(isCodeValid()) {
                playValidInputSound();
                setPrimaryText("WAHOO!");
                setInputLabel("PRESS SUBMIT TO BEGIN");
            } else {
                playInvalidInputSound();
                setPrimaryText(code, "red");
                setSecondaryText("THAT CODE IS INVALID. TRY AGAIN");
            }
            secondaryTextVisible = true;
        }
    } 
}

function printUserInputToScreen() {
    console.log("printUserInputToScreen()");
    if(userInput.length === 0) {
        setPrimaryText("QUESTION " + currentQuestion);
        // setHeaderText("QUESTION " + currentQuestion);
        setSecondaryTextWithStarAction("CLEAR");
        setFooterTextWithPoundAction("SUBMIT");
    }
    if(userInput.length < 2) {
        userInput = userInput + key;
        setPrimaryText(userInput, "white");
    }
}

function readyForNextQuestion() {
    console.log("readyForNextQuestion()");
    setFooterTextWithPoundAction("CONTINUE");
    readyForNewQuestion = true;
    userInput = "";
    currentQuestionIndex++;
}

function playTransitionAnimation() {
    console.log("playTransitionAnimation()");
}

function keypadEvent(key) {
    if (keypad.getState() === HOLD && key === '*' && currentScreen !== "startScreen") {
        resetVariables();
        playTransitionAnimation();
        loadStartScreen();
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

function fetchQAPairs(code) {
    console.log("fetchQAPairs()");
    fetch('codes.json')
        .then(response => response.json())
        .then(data => {
            if (data[code]) {
                qaPairs = data[code];
                let numObjects = qaPairs.length;
            for (let i = 0; i < numObjects; i++) {
                let newObj = new QuestionAnswerPair();
                newObj.questionNumber = i + 1;
                newObj.geoSafariNumber = qaPairs[i][0];
                newObj.answer = qaPairs[i][1];
                objects[i] = newObj;
            }
            // Shuffle the array of objects
            shuffleQAPairs(objects, numObjects);
            readyToPlay = true;
            setPrimaryText("READY TO PLAY");
            } else {
                console.log('No Q&A Pairs found for code:', code);
            }
        })
        .catch(error => console.error('Error fetching codes:', error));
}

function shuffleQAPairs(objects, numObjects) {
    console.log("shuffleQAPairs()");
    // Shuffle the array using the Fisher-Yates algorithm
    for (let i = numObjects - 1; i > 0; i--) {
        let j = Math.floor(Math.random() * (i + 1));
        let temp = objects[i];
        objects[i] = objects[j];
        objects[j] = temp;   
    }
}

function handleKeyPress(key) {
    console.log("handleKeyPress()");
    if (key === 'Enter') {
        if(currentScreen === "codeEntryScreen" && submitButton.classList.contains('btn-success')) {
            startGame();
            printQAPairs();
        } else if (currentScreen === "startScreen") {
            showCodeEntryScreen();
        }
    }
    else if (key === '*') {
        // playKeyPressSound();
        if (currentScreen === "codeEntryScreen") {
            resetVariables();
            loadStartScreen();
        } else if (currentScreen === "questionScreen") {
            if (readyForNewInput) {
                readyForNewInput = false;
            }
            if (!readyForNewQuestion) {
                userInput = "";
                // setHeaderText("QUESTION");
                setPrimaryText("QUESTION", currentQuestion);
                setInputLabel("KEY IN THE ANSWER");
                // setSecondaryText("");
                // setFooterText("KEY IN THE ANSWER");
            }
        } else if (currentScreen === "endScreen") {
            resetVariables();
            playTransitionAnimation();
            loadStartScreen();
        }
    }
    else if (key === '#') {
        if (currentScreen === "codeEntryScreen" && readyToPlay) {
            startGame();
        } else if (currentScreen === "questionScreen") {
            if (readyForNewQuestion) {
                readyForNewQuestion = false;
                showQuestionScreen();
            } else if (userInput === expectedResponse) {
                playCorrectAnswerSound();
                setPrimaryText(userInput, "green");
                setInputLabel("THAT'S CORRECT!");
                // setSecondaryText("THAT'S CORRECT!");
                readyForNextQuestion();
            } else if (userInput.length > 0) {
                playInvalidInputSound();
                attempts++;
                setPrimaryText(userInput, "red");
                if (attempts < 3) {
                    setInputLabel("TRY AGAIN");
                    // setSecondaryText("TRY AGAIN");
                    setFooterTextWithStarAction("CLEAR");
                    readyForNewInput = true;
                    userInput = "";
                } else {
                    setInputLabel("THE ANSWER IS " + expectedResponse);
                    // setSecondaryText("THE ANSWER IS " + expectedResponse);
                    readyForNextQuestion();
                }
            }
        }
    }
}

///////////////////////////
// SOUNDS
///////////////////////////

function playQuestionTransitionSound() {
    console.log("playQuestionTransitionSound()");
    audioPop.play();
}

function playEndOfGameSound() {
    console.log("playEndOfGameSound()");
}

function playStartUpSound() {
    console.log("playStartUpSound()");
}

function playKeyPressSound() {
    console.log("playKeyPressSound()");
    // audioPop.play();
}

function playCorrectAnswerSound() {
    console.log("playCorrectAnswerSound()");
}

function playValidInputSound() {
    console.log("playValidInputSound()");
}

function playInvalidInputSound() {
    console.log("playInvalidInputSound()");
}


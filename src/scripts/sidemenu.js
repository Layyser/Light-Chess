// -- Handles the sidemenu settings logic --


// Get elements
const sidemenu = document.getElementById('sidemenu');
const openMenuBtn = document.getElementById('open-menu');
const closeMenuBtn = document.getElementById('close-menu');
const startGameBtn = document.getElementById('start-game');
const restartGameBtn = document.getElementById('restart-game');
const overlay = document.getElementById('overlay-menu');

const colorInput = document.getElementById('play-black');
const eloInput = document.getElementById('elo-select');
const stockfishInput = document.getElementById('disable-stockfish');

var elo;
var stockfishLevel;
var stockfishEnabled;
var playerColor;


// First time setup:
if (localStorage.getItem("useBlack") == null) {
    localStorage.setItem("useBlack", 'false');
    localStorage.setItem("elo", '1500');
    localStorage.setItem("disableStockfish", 'false');
}


// Returns the respective stockfish level for the elo
function getStockfishLevel(elo) {
    switch (elo) {
        case '1000': return 2;
        case '1500': return 5;
        case '2000': return 7;
        case '2500': return 10;
        case '3000': return 12;
        case '3500': return 15;
        case '4000': return 17;
        case '4500': return 20;
        default: 
            console.error("Elo not supported, setting elo: 2000");
            return 7;
    }
}


// Open the side menu
openMenuBtn.addEventListener('click', () => {
    // Load values
    colorInput.checked = localStorage.getItem("useBlack") === 'true';
    eloInput.value = localStorage.getItem("elo");
    stockfishInput.checked = localStorage.getItem("disableStockfish") === 'true';

    sidemenu.classList.add('open');
    overlay.classList.add('active');
});


// Close the side menu
closeMenuBtn.addEventListener('click', () => {
    sidemenu.classList.remove('open');
    overlay.classList.remove('active');
});


// Close the side menu if clicking outside of it
document.addEventListener('click', (event) => {
    if (!sidemenu.contains(event.target) && event.target !== openMenuBtn) {
        sidemenu.classList.remove('open');
        overlay.classList.remove('active');
    }
});


// Restart game button logic
restartGameBtn.addEventListener('click', () => {
    Module.ccall('startGame');
    initializeChessboard();
    moves = [];
    movesStockfish = [];
    turn = 'white'; 
    promotion = false;
    currentMove = 1;
    moveIndex = -1;

    clearTable();
    if (playerColor != turn && stockfishEnabled) askStockfish();
});


// Start game button logic
startGameBtn.addEventListener('click', () => {
    // Store values
    localStorage.setItem("useBlack", colorInput.checked);
    localStorage.setItem("elo", eloInput.value);
    localStorage.setItem("disableStockfish", stockfishInput.checked);

    playerColor = (localStorage.getItem("useBlack") == 'false') ? 'white' : 'black';
    if (playerColor == 'black') chessboard.style.backgroundImage = "url(src/assets/chessboardlabeled-reverse.svg)"
    else chessboard.style.backgroundImage = "url(src/assets/chessboardlabeled.svg)"
    initializeChessboard(); // In case the board needs to be flipped

    // Set stockfish properties
    stockfishColor = (playerColor == 'white' ? 'black' : 'white');
    stockfishEnabled = (localStorage.getItem("disableStockfish") == 'false');
    elo = localStorage.getItem("elo");
    stockfishLevel = getStockfishLevel(elo);
    stockfish.postMessage(`setoption name Skill Level value ${stockfishLevel}`);
    console.log(`setoption name Skill Level value ${stockfishLevel}`);

    if (playerColor != turn && stockfishEnabled) askStockfish();
    
    sidemenu.classList.remove('open');
    overlay.classList.remove('active');
});


// Handles the logic of loading settings when the website is loaded
document.addEventListener('DOMContentLoaded', function() {
    // Set player data
    playerColor = (localStorage.getItem("useBlack") == 'false') ? 'white' : 'black';
    if (playerColor == 'black') chessboard.style.backgroundImage = "url(src/assets/chessboardlabeled-reverse.svg)"
    else chessboard.style.backgroundImage = "url(src/assets/chessboardlabeled.svg)"

    // Set stockfish data
    elo = localStorage.getItem("elo");
    stockfishLevel = getStockfishLevel(elo);
    stockfish.postMessage(`setoption name Skill Level value ${stockfishLevel}`);
    stockfishEnabled = (localStorage.getItem("disableStockfish") == 'false');
    stockfishColor = (playerColor == 'white' ? 'black' : 'white');

    if (playerColor != turn && stockfishEnabled) askStockfish();
});
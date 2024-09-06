// TODO:  add sound when check, (add button to swap the board side, and button to reset the board)?


// -- Handles the logic of the chessboard and stockfish --

var wasmSupported = typeof WebAssembly === 'object' && WebAssembly.validate(Uint8Array.of(0x0, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00));
var stockfish = new Worker(wasmSupported ? 'src/wasm/stockfish.wasm.js' : 'src/wasm/stockfish.js');
var stockfishColor;

// Stockfish event listener
stockfish.addEventListener('message', function (e) {
    console.log(e.data);

    if (e.data.startsWith('bestmove')) {
        const bestMove = e.data.split(' ')[1];
        console.log("Best move: ", bestMove);

        movePiece(null, bestMove);
    }
});

stockfish.postMessage('uci');
stockfish.postMessage('ucinewgame');


const chessboard = document.querySelector('.chessboard');
const movePieceAudio = new Audio('src/assets/pieces/movepiece.mp3');

const colors = ['black', 'white', 'none'];
const kinds = ['empty', 'promote', 'bishop', 'king', 'knight', 'pawn', 'queen', 'rook'];
const pieceToNumber = { // Check PieceKind enum in chess.hh
    'q': 6, // Queen
    'r': 7, // Rook
    'b': 3, // Bishop
    'n': 4  // Knight
};

let moves = [];
let movesStockfish = [];
let turn = 'white'; 
let promotion = false;
let currentMove = 1;
let moveIndex = -1;


// Sets the piece in row, col with the color and kind. Used to print the board
function setPiece(row, col, color, kind) {
    const square = document.querySelector(`div[row="${row}"][col="${col}"]`);
    square.className = 'square';
    if (kind !== "empty") {
        square.classList.add('piece');
        square.setAttribute("piece", `${color}-${kind}`);
        square.addEventListener("mousedown", onMouseDownDrag);
        square.removeEventListener("mousedown", removeSelected);
    } 
    else {
        square.classList.add('blank');
        square.removeAttribute("piece");
        square.addEventListener("mousedown", removeSelected);
    }
}


// Prints the current C++ board
function printBoard() {
    for (let i = 0; i < 8; ++i) {
        for (let j = 0; j < 8; ++j) {
            const ptr = Module.ccall('askBoardSquare', 'number', ['number', 'number'], [i, j]);
            const size = Module.HEAP32[ptr / 4];
            if (size > 0) { // Size should be always greater than 0 or 0 if startRow/startCol is not valid
                const color = Module.HEAP32[(ptr / 4) + 1];
                const kind = Module.HEAP32[(ptr / 4) + 2];
                setPiece(i, j, colors[color], kinds[kind]);
            }
        }
    }
}


// Initialitzes the chessboard div
function initializeChessboard() {
    // Clear the existing chessboard content
    chessboard.innerHTML = '';

    // Create the squares and store them in an array
    const squares = [];
    for (let row = 0; row < 8; row++) {
        for (let col = 0; col < 8; col++) {
            const square = document.createElement('div');
            square.setAttribute('row', row);
            square.setAttribute('col', col);
            square.classList.add('square');
            squares.push(square);
        }
    }

    // Append the squares to the chessboard with flipping if needed
    if (playerColor === 'black') {
        // Flip the board: reverse both rows and columns
        for (let row = 0; row < 8; row++) {
            for (let col = 0; col < 8; col++) {
                const originalRow = 7 - row;
                const originalCol = 7 - col;
                const index = originalRow * 8 + originalCol;
                chessboard.appendChild(squares[index]);
            }
        }
    } else {
        // Append the squares in normal order
        for (let i = 0; i < 64; i++) {
            chessboard.appendChild(squares[i]);
        }
    }

    // Add the glow effect
    const glow = document.createElement('div');
    glow.classList.add('glow');
    chessboard.appendChild(glow);

    // Initialize the board appearance and dragging cursors
    printBoard();
    setDraggingCursors();
}


// Promotes  with the kind[n] in {BISHOP, KNIGHT, QUEEN, ROOK}
function promote(id) {
    const promoted = Module.ccall('doPromote', 'number', ['number'], [id]);
    if (!promoted) return;

    const resultPointer = Module.ccall('getChessNotation', 'number');
    const promotedString = Module.UTF8ToString(resultPointer);
    console.log(turn, "just promoted with:", promotedString);
    moves.pop();
    moves.push(promotedString);

    const resultPointerSF = Module.ccall('getStockfishNotation', 'number')
    const movementStringSF = Module.UTF8ToString(resultPointerSF);
    movesStockfish.pop();
    movesStockfish.push(movementStringSF);

    printBoard();

    turn = Module.ccall('askTurn', 'number') ? 'white' : 'black';
    tableDelete();
    const tableCol = (turn === 'black') ? 0 : 1;
    tableInsert(tableCol, promotedString);

    askStockfish();
}


// Creates the promotion popup
function createPromotionPopup(row, col) {
    const selector = `div[row="${row}"][col="${col}"]`;
    const targetDiv = document.querySelector(selector);
    if (!targetDiv) {
        console.error('Target div not found');
        return;
    }
    
    const overlayDiv = document.createElement('div');
    overlayDiv.classList.add('overlay-div');

    // Remove backwards while promoting
    buttonBackward.disabled = true;

    // Create each button to promote
    const buttonIds = [0, 1, 2, 3];
    buttonIds.forEach(id => {
        const button = document.createElement('button');
        button.setAttribute("button-id", id);
        let color;
        if (row == 0) color = "white";
        else color = "black";

        button.setAttribute("data-color", color);
        button.addEventListener('click', () => {
            promote(id);
            overlayDiv.remove();
            buttonBackward.disabled = false;
            setDraggingCursors();
        });
        overlayDiv.appendChild(button);
    });

    // Position the popup on top of the promoting pawn
    overlayDiv.style.left = `calc(max(-13.125vw, -13.125vh))`; // -8.75*2 + 8.75/2
    overlayDiv.style.top = `calc(max(-8.75vw, -8.75vh))`;

    // Fix the position to not go outside the screen
    targetDiv.appendChild(overlayDiv);
    let rect = overlayDiv.getBoundingClientRect();
    if (rect.left < 0) {
        let positive = rect.left * (-1);
        overlayDiv.style.left = `calc(max(-13.125vw, -13.125vh) + ${positive}px)`; // -8.75*2 + 8.75/2
    }
    if (rect.top < 0) {
        let positive = rect.top * (-1);
        overlayDiv.style.left = `calc(max(-13.125vw, -13.125vh) + ${positive}px)`; // -8.75*2 + 8.75/2
    }
}

function translateRow(row) {
    return 8 - parseInt(row);
}

function translateCol(col) {
    return col.charCodeAt(0) - 'a'.charCodeAt(0);
}

function translatePiece(piece) {
    if (pieceToNumber.hasOwnProperty(piece)) {
        return pieceToNumber[piece];
    } else {
        throw new Error('Invalid piece notation');
    }
}

// Moves the piece to the square with cardinal or by stockfish with his move
function movePiece(cardinal, stockfishMove) {
    if (cardinal == null && stockfishMove == null) return;
    buttonBackward.disabled = false;

    movePieceAudio.pause();
    movePieceAudio.currentTime = 0;
    movePieceAudio.play();
    moves.splice(moveIndex+1);
    movesStockfish.splice(moveIndex+1);

    let result;
    if (cardinal != null) {
        // Returns 0 if the movement could not be done, 1 if it was done, 2 if the next move is a promotion, 3 if it was long castling, 4 if it was short castling
        // However, we will only use (2)
        result = Module.ccall('doMovement', 'number', ['number'], [cardinal]);
        printBoard();
    }
    else {
        chars = stockfishMove.split('');
        let promotion = 0;
        if (chars.length == 5) promotion = translatePiece(chars[4]);
        result = Module.ccall('doStockfishMovement', 'number', ['number', 'number', 'number', 'number','number'], 
            [translateRow(chars[1]), translateCol(chars[0]), translateRow(chars[3]), translateCol(chars[2]), promotion]);
        printBoard();
    }

    const resultPointer = Module.ccall('getChessNotation', 'number')
    const movementString = Module.UTF8ToString(resultPointer);
    ++moveIndex;
    moves.push(movementString);

    const resultPointerSF = Module.ccall('getStockfishNotation', 'number')
    const movementStringSF = Module.UTF8ToString(resultPointerSF);
    movesStockfish.push(movementStringSF);
    console.log(movesStockfish);

    let col = 0;
    if (turn == "black") col = 1;
    tableInsert(col, movementString);

    // (2) Promotion
    if (result == 2 && cardinal != null) {
        const selector = `div[cardinal="${cardinal}"]`;
        let div = document.querySelector(selector);
        let endRow = div.getAttribute("row");
        let endCol = div.getAttribute("col");
        createPromotionPopup(endRow, endCol);
        turn = 'none';
    }

    if (turn == 'black' || turn == 'white' || turn == 'any') {
        let askTurn = Module.ccall('askTurn', 'number');
        if (askTurn != 0) {
            currentMove++;
            turn = 'white';
        } 
        else turn = 'black';
    }

    if (turn == stockfishColor && stockfishEnabled) {
        askStockfish();
    }

    buttonForward.disabled = true;
    

    console.log("Now it's", turn, "turn");
    setDraggingCursors();
    removeCardinals();
    setRemove();
}


// Sets the cardinal for every possible move of the piece in row, col
function askMovs(row, col) {
    console.log("Asking moves of row:", row, "col:", col);
    // Clear previously selected movements
    removeSelected();
    removeCardinals();

    clickedPieceCol = col;
    clickedPieceRow = row;

    let ptr = Module.ccall('askMovements', 'number', ['number', 'number'], [row, col]); // Returns a list of possible movements of the selected piece

    // Read the size of the returned array
    let size = Module.HEAP32[ptr / 4];
    console.log(size);
    let movements = [];
    for (let i = 0; i < size; i++) {
        let posRow = Module.HEAP32[(ptr / 4) + 1 + (i * 2)];
        let posCol = Module.HEAP32[(ptr / 4) + 1 + (i * 2) + 1];
        movements.push({ row: posRow, col: posCol });
    }
    console.log(movements);
    let i = 0;
    movements.forEach(move => {
        const selector = `div[row="${move.row}"][col="${move.col}"]`;
        const divs = document.querySelectorAll(selector);
        divs.forEach(div => {
            div.setAttribute('cardinal', i);
            if (div.classList.contains('piece')) {
                div.classList.add('to-capture');
            } else {
                div.classList.add('to-move');
            }
            ++i;
            // Add event listener for squares with 'to-move' or 'to-capture' (check events.js)
            div.removeEventListener("mousedown", removeSelected);
            div.addEventListener('click', onMouseClickMovable);
        });
    });


    setRemove();
}


function askStockfish() {
    // Wait for a response and then send more commands
    setTimeout(function() {
        // Set the position on the board (e.g., starting position)
        let message = 'position startpos moves';
        console.log(movesStockfish);
        for (let i = 0; i < movesStockfish.length; ++i) message = message + ' ' + movesStockfish[i];

        stockfish.postMessage(message);
    
        // Ask Stockfish to calculate the best move (for a given depth)
        stockfish.postMessage('go depth 10');
    }, 1000);
}
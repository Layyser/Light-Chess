// -- Handles the logic of the piece dragging and piece clicking events, check chessboard.js --


let draggingPiece = null;
let draggingPieceRow = null;
let draggingPieceCol = null;
let clickedPieceRow = null;
let clickedPieceCol = null;
let offsetX, offsetY;
let diffCenterX, diffCenterY;


function onMouseDownDrag(event) {
    event.preventDefault();

    // Only allow dragging if no piece is currently being dragged
    if (!draggingPiece) {
        draggingPiece = this;
        draggingPieceRow = parseInt(draggingPiece.getAttribute("row"), 10);
        draggingPieceCol = parseInt(draggingPiece.getAttribute("col"), 10);

        // Calculate the offset between the cursor and the top-left corner of the piece
        offsetX = event.clientX;
        offsetY = event.clientY;

        const rect = draggingPiece.getBoundingClientRect();
        let centerPieceX = (rect.left + rect.right)/2;
        let centerPieceY = (rect.top + rect.bottom)/2;
        diffCenterX = event.clientX - centerPieceX;
        diffCenterY = event.clientY - centerPieceY;

        // Highlight possible moves for the piece
        let pieceColor = 'white';

        if (this.classList.contains("blank")) return;

        if (this.getAttribute("piece").startsWith('black')) {
            pieceColor = 'black';
        }

        if ((pieceColor == playerColor && turn == playerColor) || (pieceColor == turn && !stockfishEnabled)) {
            // Add Glow
            let glowingRow = draggingPieceRow;
            let glowingCol = draggingPieceCol;
            console.log("clicking row:", draggingPieceRow, "col:",  draggingPieceCol);
            if (playerColor == 'black') {
                glowingRow = 7 - glowingRow;
                glowingCol = 7 - glowingCol;
            }
            document.querySelector('.glow').style.top = `calc(${12.5 * (glowingRow - 8)}%)`;
            document.querySelector('.glow').style.left = `calc(${12.5 * (glowingCol)}%)`;
            
            askMovs(draggingPieceRow, draggingPieceCol);

            // Update the visual position of the dragging piece
            updateDraggingPiecePosition(event.clientX, event.clientY);

            // Attach event listeners to the document
            document.addEventListener('mousemove', onMouseMoveDrag);
            document.addEventListener('mouseup', onMouseUpDrag);
            
            document.querySelector('.glow').style.backgroundColor = `rgb(255, 255, 255, 0.18)`;
            document.querySelector('.glow').style.boxShadow = `0px 0px 40px 10px rgb(255, 255, 255, 0.18)`;
            

        }
        else {
            draggingPiece = null;
            draggingPieceRow = null;
            draggingPieceCol = null;
        }
    }
}


function updateDraggingPiecePosition(clientX, clientY) {
    const x = clientX - offsetX + diffCenterX;
    const y = clientY - offsetY + diffCenterY;
    draggingPiece.style.cursor = "grabbing";
    draggingPiece.style.transform = `translate(${x}px, ${y}px)`;
    draggingPiece.style.zIndex = 1000;
}


function onMouseMoveDrag(event) {
    event.preventDefault();

    // Update the visual position of the dragging piece
    updateDraggingPiecePosition(event.clientX, event.clientY);
}


function onMouseUpDrag(event) {
    event.preventDefault();

    // Find the square where the piece was dropped
    const dropTargets = document.elementsFromPoint(event.clientX, event.clientY);
    let dropTarget = dropTargets[0];
    let found = false;
    for (let i = 0; i < dropTargets.length; ++i) {
        dropTarget = dropTargets[i];
        if (dropTarget.classList.contains('to-move') || dropTarget.classList.contains('to-capture')) {
            found = true;
            break;
        }
    }

    if (found && dropTarget && dropTarget.classList.contains('square')) {
        const dropRow = parseInt(dropTarget.getAttribute('row'), 10);
        const dropCol = parseInt(dropTarget.getAttribute('col'), 10);

        console.log(`Dragging (${draggingPieceRow},${draggingPieceCol}) to (${dropRow},${dropCol})`);

        // Clear highlighted squares
        removeSelected();
        let cardinal = dropTarget.getAttribute('cardinal');
        movePiece(cardinal);
    }

    // Reset dragging variables
    if (draggingPiece) {
        draggingPiece.style.cursor = "grab";
        draggingPiece.style.transform = 'none';
        draggingPiece.style.zIndex = 100;
        clickedPieceRow = draggingPieceRow;
        clickedPieceCol = draggingPieceCol;
    }

    draggingPiece = null;
    draggingPieceRow = null;
    draggingPieceCol = null;

    // Remove event listeners from the document
    document.removeEventListener('mousemove', onMouseMoveDrag);
    document.removeEventListener('mouseup', onMouseUpDrag);
}


function removeOnMouseClickListeners() {
    const squares = chessboard.querySelectorAll('.square');
    squares.forEach(square => {
        square.removeEventListener('click', onMouseClickMovable);
    });
}


function onMouseClickMovable(event) {
    const eventRow = parseInt(this.getAttribute('row'), 10);
    const eventCol = parseInt(this.getAttribute('col'), 10);
    console.log(`Clicking piece from (${clickedPieceRow},${clickedPieceCol}) to (${eventRow},${eventCol})`);

    if (clickedPieceRow != eventRow || clickedPieceCol != eventCol) {
        console.log("Hey");
        // Clear highlighted squares
        removeSelected();
        const selector = `div[row="${eventRow}"][col="${eventCol}"]`;
        console.log(`selecting piece (${eventRow},${eventCol})`);
        const div = document.querySelector(selector);
        let cardinal = div.getAttribute('cardinal');
        movePiece(cardinal);
    }

    removeOnMouseClickListeners();

    // Reset dragging variables
    draggingPiece = null;
    draggingPieceRow = null;
    draggingPieceCol = null;
    clickedPieceCol = null;
    clickedPieceRow = null;
}

// Updates the cursors to only be drag in top of the turn ones
function setDraggingCursors() {
    console.log("updatin");
    console.log("its ", turn, "turn");
    chessboard.querySelectorAll('.square').forEach(square => {
        square.style.cursor = 'default';
        if (square.hasAttribute("piece")) {
            if (square.getAttribute("piece").startsWith('black') && (('black' == playerColor && turn == playerColor) || ('black' == turn && !stockfishEnabled))) square.style.cursor = 'grab';
            else if (square.getAttribute("piece").startsWith('white') && (('white' == playerColor && turn == playerColor) || ('white' == turn && !stockfishEnabled))) square.style.cursor = 'grab';
        }
    });
}

// Clear possible movements, used before selecting other piece or clicking in an empty square
function removeSelected() {
    chessboard.querySelectorAll('.square').forEach(square => {
        square.classList.remove('to-move', 'to-capture');
        square.removeEventListener('click', onMouseClickMovable);
    });
    document.querySelector('.glow').style.background = `transparent`;
    document.querySelector('.glow').style.boxShadow = `0px 0px 40px 10px rgb(255, 255, 255, 0)`;
}

function removeCardinals() {
    chessboard.querySelectorAll('.square').forEach(square => {
        square.removeAttribute('cardinal');
    });
}


// Add an event listener for every square that is empty and cannot be moved to
function setRemove() {
    chessboard.querySelectorAll('.blank').forEach(square => {
        if (!square.classList.contains('to-move') && !square.classList.contains('to-capture')) {
            square.addEventListener("mousedown", removeSelected);
            square.removeEventListener("mousedown", onMouseDownDrag);
        }
    });
}


// Event listener for mouse up on document for ending drag operation
document.addEventListener('mouseup', onMouseUpDrag);


const github = document.getElementById("github-link");

github.addEventListener('click', () => {
    window.open( "https://github.com/Layyser");
});
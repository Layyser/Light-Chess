// -- Handles the logic of the table of movements and the buttons --


// Deletes the whole table
function clearTable() {
    var table = document.getElementById("t");
    while (table.rows.length > 1) table.deleteRow(1);
    buttonBackward.disabled = true;
    buttonForward.disabled = true;
}


// Deletes the last cell from the table
function tableDelete() {
    let table = document.getElementById("t").getElementsByTagName('tbody')[0];
    if (table.rows.length == 0) return;
    let lastRowIndex = table.rows.length - 1;
    let lastRow = table.rows[lastRowIndex];
    let cells = lastRow.cells;
    if (cells.length == 3 && lastRow.cells[lastRow.cells.length - 1].innerHTML == "") table.deleteRow(lastRowIndex);
    else lastRow.cells[lastRow.cells.length - 1].innerHTML = "";
}


// Inserts data into a new cell in col
function tableInsert(col, data) {
    let table = document.getElementById("t").getElementsByTagName('tbody')[0];
    
    if (col == 0) {
        let newRow = table.insertRow();
        let cellTurn = newRow.insertCell(0);
        cellTurn.innerHTML = `${currentMove}.`;
        let newCell = newRow.insertCell(1);
        newCell.innerHTML = data;
        newRow.insertCell(2);
    }
    else if (col == 1) {
        let lastRow = table.rows[table.rows.length - 1];
        let lastCell = lastRow.cells[lastRow.cells.length - 1];   
        lastCell.innerHTML = data;
    }
}


const buttonBackward = document.getElementById('backwards');
const buttonForward = document.getElementById('forward');

// Undoes the last movement
buttonBackward.addEventListener('click', function() {
    let result = Module.ccall('undoGameMovement', 'number');
    --moveIndex;
    printBoard();
    tableDelete();
    turn = Module.ccall('askTurn', 'number') ? 'white' : 'black';
    
    if (turn == 'black') --currentMove;
    console.log(result);

    if (currentMove == 1 && turn == 'white') this.disabled = true;
    buttonForward.disabled = false;

    document.querySelector('.glow').style.background = `transparent`;
    document.querySelector('.glow').style.boxShadow = `0px 0px 40px 10px rgb(255, 255, 255, 0)`;
    setDraggingCursors();

    console.log(movesStockfish);
});

// Redoes the following movement
buttonForward.addEventListener('click', function() {
    let result = Module.ccall('redoGameMovement', 'number');
    ++moveIndex;
    printBoard();
    let col = 0;
    if (turn == 'black') col = 1;
    tableInsert(col, moves[moveIndex]);

    turn = Module.ccall('askTurn', 'number') ? 'white' : 'black';
    
    if (turn == 'white') ++currentMove;
    console.log(result);

    if (moveIndex+1 == moves.length) buttonForward.disabled = true;
    buttonBackward.disabled = false;

    document.querySelector('.glow').style.background = `transparent`;
    document.querySelector('.glow').style.boxShadow = `0px 0px 40px 10px rgb(255, 255, 255, 0)`;
    setDraggingCursors();
});
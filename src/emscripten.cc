#include "chess.hh"
#include <emscripten.h>


// -- Example of how to use chess.cc functions to handle a game --

// These vectors are used for temporary storage
static vector<int> resultBuffer;
static vector<Move> moves;

static vector<Move> game;           // Stores the movements done in the game
static int gameIndex;               // Stores the index of the game movement, starts in -1
static char* movementChessNotation; // Stores the chessNotation of the last movement


EMSCRIPTEN_KEEPALIVE
void startGame() {
    moves.clear();
    game.clear();
    gameIndex = -1;
    iniChessboard();
    turn = WHITE;
}

EMSCRIPTEN_KEEPALIVE
int* askMovements(int row, int col) {
    resultBuffer.clear();
    
    Position p = {row, col};
    if (!validPos(p) || c.board[row][col].color != turn) {
        resultBuffer.push_back(0);
        return resultBuffer.data();
    }

    moves = validMoves(p);
    
    // Store the size of the positions first
    resultBuffer.push_back(moves.size());
    
    // Store each position's x and y
    for (int i = 0; i < moves.size(); ++i) {
        resultBuffer.push_back(moves[i].end.row);
        resultBuffer.push_back(moves[i].end.col);
    }
    
    return resultBuffer.data();
}

EMSCRIPTEN_KEEPALIVE
char* getChessNotation() {
    if (gameIndex < 0 || gameIndex > game.size()) return {};
    string notation = movementToChessNotation(game[gameIndex]);
    movementChessNotation = &notation[0];
    return movementChessNotation;
}

EMSCRIPTEN_KEEPALIVE
char* getStockfishNotation() {
    if (gameIndex < 0 || gameIndex > game.size()) return {};
    Move m = game[gameIndex];

    string notation = translatePos(m.start) + translatePos(m.end);

    if (m.promote != EMPTY && m.promote != PROMOTE) notation += NAMES[m.promote];

    movementChessNotation = &notation[0];
    return movementChessNotation;
}

EMSCRIPTEN_KEEPALIVE
int doStockfishMovement(int startRow, int startCol, int endRow, int endCol, PieceKind promotion) {
    bool enPassant = (c.board[startRow][startCol].kind == PAWN && (startCol != endCol) && c.board[endRow][endCol].kind == EMPTY);
    bool castle = (c.board[startRow][startCol].kind == KING && abs(startCol - endCol) > 1);
    Move m = {{startRow, startCol}, {endRow, endCol}, c.board[endRow][endCol], castle, promotion, enPassant};

    doMove(m, gameIndex, game, true);

    printBoard();
    
    turn = (turn == WHITE) ? BLACK : WHITE;
    return 1;
}

EMSCRIPTEN_KEEPALIVE
int doMovement(int n) {
    if (n < 0 || n > moves.size()) return 0;
    Move m = moves[n];

    doMove(m, gameIndex, game, true);

    if (m.castle) {
        turn = (turn == WHITE) ? BLACK : WHITE;
        if (moves[n].end.col == 2) return 3; // Long castle (Left)
        else return 4; // Short castle (Right)
    }

    printBoard();

    // Check for promotion
    if (isPromotion(m)) {
        c.promotion = true;
        return 2;
    }
    
    turn = (turn == WHITE) ? BLACK : WHITE;
    return 1;
}

EMSCRIPTEN_KEEPALIVE
bool undoGameMovement() {
    if (gameIndex < 0 || gameIndex >= game.size()) return false;

    undoMove(gameIndex, game);

    if (!c.promotion) turn = (turn == WHITE) ? BLACK : WHITE;
    else c.promotion = false;
    
    return true;
}

EMSCRIPTEN_KEEPALIVE
int redoGameMovement() {
    if (gameIndex < -1 || gameIndex >= (int) game.size()) return 0; // WOW (int) is necessary

    ++gameIndex;
    Move m = game[gameIndex];
    --gameIndex;

    doMove(m, gameIndex, game, false);

    if (m.castle) {
        turn = (turn == WHITE) ? BLACK : WHITE;
        if (m.end.col == 2) return 3; // Long castle (Left)
        else return 4; // Short castle (Right)
    }

    printBoard();

    // Check for promotion
    if (isPromotion(m)) {
        c.promotion = true;
        return 2;
    }
    
    turn = (turn == WHITE) ? BLACK : WHITE;
    return 1;
}

EMSCRIPTEN_KEEPALIVE
bool doPromote(int n) {
    vector<PieceKind> kinds = {BISHOP, KNIGHT, QUEEN, ROOK};
    // 1. Check if the promotion is valid
    if (!c.promotion || n > kinds.size()) return false;

    // 2. Promote
    bool promoted = promote(kinds[n], gameIndex, game);

    if (!promoted) return false;

    turn = (turn == WHITE) ? BLACK : WHITE;

    return true;
}

EMSCRIPTEN_KEEPALIVE
int* askBoardSquare(int row, int col) {
    resultBuffer.clear();
    
    Position p = {row, col};
    if (!validPos(p)) {
        resultBuffer.push_back(0);
        return resultBuffer.data();
    }
    
    // Store the size of the position first
    resultBuffer.push_back(1);

    // Store the color and the kind of the piece
    resultBuffer.push_back(c.board[row][col].color);
    resultBuffer.push_back(c.board[row][col].kind);
    
    return resultBuffer.data();
}

EMSCRIPTEN_KEEPALIVE
int isEnd() {
    bool cannotMove = checkFinal(turn);
    bool check = isCheck(turn);
    
    if (cannotMove) return check ? 1 : 2;
    return check ? 3 : 0;
}

EMSCRIPTEN_KEEPALIVE
int askTurn() {
    return (turn == WHITE);
}
#ifndef CHESS_H
#define CHESS_H

#include <string>
#include <vector>

using namespace std;


// -- Enum declarations --
enum PieceKind {
    EMPTY,   // Used for empty squares
    PROMOTE, // Used when the user did not choose the promoted piece yet
    BISHOP,
    KING,
    KNIGHT,
    PAWN,
    QUEEN,
    ROOK
};

enum Color {
    BLACK,
    WHITE,
    NONE
};


// -- Struct declarations --
struct Piece {
    Color color;
    PieceKind kind;

    bool operator==(const Piece& other) const {
        return color == other.color && kind == other.kind;
    }

    bool operator!=(const Piece& other) const {
        return !(*this == other);
    }
};

struct Position {
    int row;
    int col;

    bool operator==(const Position& other) const {
        return row == other.row && col == other.col;
    }
};

struct Move {
    Position start;
    Position end;
    Piece targetPiece;  // A move captures if ((targetPiece.kind != EMPTY && targetPiece.kind != PROMOTE) || enPassant)
    bool castle;
    PieceKind promote;
    bool enPassant;     // If a move is en passant, a pawn will be captured
};

struct Chessboard {
    Piece board[8][8];  // [0][0] being top left
    Position kingPosition[2];
    Position enPassant; // Stores the position that could be captured using en passant
    bool promotion;

    // If 0, they have not moved, otherwise, it stores the number of the game index in which they have first moved
    int kingMoved[2];   // Track if either king has moved kingMoved[color]
    int rookMoved[2][2];// Track if each rook has moved (rookMoved[color][rookIndex]) left = 0, right = 1;
};


// These vars could also be passed as arguments instead of declared as extern
extern Chessboard c;
extern Color turn;

// Translations for every PieceKind for chess notation
extern string NAMES[];
/*
    EMPTY - ""
    PROMOTE - ""
    BISHOP - "B"
    KING - "K"
    KNIGHT - "N"
    PAWN - ""
    QUEEN - "Q"
    ROOK - "R"
*/


// -- Function declarations for chess.cc --

// -- Operator overloads for Move and int vectors --

vector<Move>& operator+=(vector<Move>& vec1, const vector<Move>& vec2);
vector<Move> operator+(const vector<Move>& vec1, const vector<Move>& vec2);
vector<int>& operator+=(vector<int>& vec1, const vector<int>& vec2);



// -- Translations --

// Given a PieceKind, returns its chess letter
string translatePiece(const PieceKind kind);

// Given a row, returns its board number
string translateRow(const int row);

// Given a col, returns its board letter
string translateCol(const int col);

// Given a position, returns its chess notation
string translatePos(const Position pos);



// -- Execute moves --

// Returns true if the position is inside the chessboard
bool validPos(const Position pos);

// Returns true if the move (already done) leds to a promotion
bool isPromotion(Move m);

// Executes the new move and stores it in game[gameIndex+1] or executes the move (should be called using move = game[gameIndex])
void doMove(Move move, int& gameIndex, vector<Move>& game, bool newMove);

// Undoes game[gameIndex] without deleting it from game and sets gameIndex to gameIndex-1
void undoMove(int& gameIndex, vector<Move>& game);

// Returns true if the promotion has been done, false otherwise
// Promotes the piece using the movement in gameIndex
bool promote(PieceKind kind, int& gameIndex, vector<Move>& game);



// -- Checks and game validation --

// Returns whether the current position is check or not
bool isCheck(Color kingColor);

// Temporary movement to check for checks
void tryMove(Move move);

// Undo the temporary movement
void untryMove(Move move);

// Returns true if the move would cause a check, therefore it is illegal
bool checkIfCheck(const Move &move, Color kingColor);

// Returns false if the game continues
// Returns true if there is not any legal move
bool checkFinal(Color turnColor);



// -- Movements --

// Returns the moves of the castlings for the king with color movingColor
vector<Move> kingCastling(Color movingColor);

// Returns the movements of the king
vector<Move> kingMoves(Position pos, Color movingColor);

// Returns the movements of the knight
vector<Move> knightMoves(Position pos, Color movingColor);

// Returns the movements of the pawn
vector<Move> pawnMoves(Position pos, Color movingColor);

// Returns the sliding movements (rook or bishop)
vector<Move> slideMoves(Position pos, Color movingColor, const vector<Position>& directions);

// Returns the movements of the rook
vector<Move> rookMoves(Position pos, Color movingColor);

// Returns the movements of the bishop
vector<Move> bishopMoves(Position pos, Color movingColor);

// Returns the movements of the queen
vector<Move> queenMoves(Position pos, Color movingColor);

// Returns every valid movement that the piece in movingPos can perform
vector<Move> validMoves(Position movingPos);



// -- Chess notation --

// It returns the positions that contain a piece of the same kind and color that could reach pos (pawn is handled differently). Used for chess notation only
vector<Position> findPossibles(Color movingColor, PieceKind kind, Position pos);

// To be called after performing the movement
// Returns the chess notation of move
// REF: https://rcc.fide.com/appendixc/
string movementToChessNotation(Move move);



// -- Board --

// Initialitzes the chessboard for the start of the game
void iniChessboard();

// Prints the chessboard (used for debugging)
void printBoard();






// -- Function declarations for emscripten.cc --

extern "C" {
    // Initialitzes the chessboard and clears the variables
    void startGame();

    // Returns a list of possible movements of the selected piece
    int* askMovements(int row, int col);

    // To be called right after doMovement or doPromote, returns the chess notation of the last move
    char* getChessNotation();

    // To be called right after doMovement or doPromote, returns the stockfish notation
    char* getStockfishNotation();

    // Does the stockfish movement given the start position and the end position
    int doStockfishMovement(int startRow, int startCol, int endRow, int endCol, PieceKind promotion);

    // Returns 1 if the movement has been done, 0 if not, 
    // 2 if the next move is a promotion, 3 if is long castling, 4 if short castling
    int doMovement(int n);

    // Returns true if the movement has been undone
    bool undoGameMovement();

    // Returns true if the movement has been redone
    int redoGameMovement();

    // Returns true if the promotion has been done
    // Promotes the piece to kinds[n]
    bool doPromote(int n);

    // Returns the kind of piece and the color of the square in position [row][col]
    int* askBoardSquare(int row, int col);

    // Returns 0 if it's not the end, 1 if it's checkmate, 2 if it's stalemate, 3 if it's just check
    int isEnd();

    // Returns 0 if it is black's turn, 1 otherwise
    int askTurn();
}

#endif

#include <stdio.h>
#include <iostream>
#include <cmath>

#include "chess.hh"

using namespace std;


// Standard pieces
Piece blackBishop = {BLACK, BISHOP};
Piece blackKing = {BLACK, KING};
Piece blackKnight = {BLACK, KNIGHT};
Piece blackPawn = {BLACK, PAWN};
Piece blackQueen = {BLACK, QUEEN};
Piece blackRook = {BLACK, ROOK};

Piece whiteBishop = {WHITE, BISHOP};
Piece whiteKing = {WHITE, KING};
Piece whiteKnight = {WHITE, KNIGHT};
Piece whitePawn = {WHITE, PAWN};
Piece whiteQueen = {WHITE, QUEEN};
Piece whiteRook = {WHITE, ROOK};

Piece emptyPiece = {NONE, EMPTY};


const int N_DIR_KNIGHT = 8;
const int N_DIR_BISHOP_ROOK = 4;
const int N_DIR_PAWN = 2;
const int BOARD_SIZE = 8;


string NAMES[] = {"", "", "B", "K", "N", "", "Q", "R"};
Chessboard c;
Color turn;



// -- Operator overloads for Move and int vectors --

vector<Move>& operator+=(vector<Move>& vec1, const vector<Move>& vec2) {
    vec1.insert(vec1.end(), vec2.begin(), vec2.end());
    return vec1;
}

vector<Move> operator+(const vector<Move>& vec1, const vector<Move>& vec2) {
    vector<Move> result = vec1;
    result.insert(result.end(), vec2.begin(), vec2.end());
    return result;
}

vector<int>& operator+=(vector<int>& vec1, const vector<int>& vec2) {
    vec1.insert(vec1.end(), vec2.begin(), vec2.end());
    return vec1;
}



// -- Translations --

string translatePiece(const PieceKind kind) {
    return NAMES[kind];
}

string translateRow(const int row) {
    return string(1, '8' - row);
}

string translateCol(const int col) {
    return string(1, 'a' + col);
}

string translatePos(const Position pos) {
    string position;
    position += translateCol(pos.col);
    position += translateRow(pos.row);
    return position;
}



// -- Execute moves --

bool validPos(const Position pos) {
    return (pos.row <= 7 && pos.col <= 7 && pos.row >= 0 && pos.col >= 0);
}

bool isPromotion(Move m) {
    // Don't check for colors, since pawns don't go backwards
    if ((m.end.row == 0 || m.end.row == 7) && c.board[m.end.row][m.end.col].kind == PAWN) return true;

    return false;
}

void doMove(Move move, int& gameIndex, vector<Move>& game, bool newMove) {
    ++gameIndex;
    if (newMove) { // 0.1 Wipe out every movement after the last done and ensure game[gameIndex] to exist but empty
        game.resize(gameIndex+1);
        game[gameIndex] = move;
    }   
    // 0.2 In case we execute an existing game movement, we won't lose the next moves

    Color movingColor = c.board[move.start.row][move.start.col].color;
    PieceKind movingPiece = c.board[move.start.row][move.start.col].kind;

    // 1. Do the move
    c.board[move.end.row][move.end.col] = c.board[move.start.row][move.start.col];
    c.board[move.start.row][move.start.col] = emptyPiece;

    // 2. If we are moving a king, update its position
    if (movingPiece == KING) {
        c.kingPosition[movingColor] = move.end;
        if (!c.kingMoved[movingColor]) c.kingMoved[movingColor] = gameIndex;
    }

    // 3. If we are moving a rook set the first rook movement to the current gameIndex
    if (movingPiece == ROOK) {
        if (move.start.col == 0 && !c.rookMoved[movingColor][0]) c.rookMoved[movingColor][0] = gameIndex;
        else if (!c.rookMoved[movingColor][1]) c.rookMoved[movingColor][1] = gameIndex;  
    }

    // 4. If we are performing a castle, we will also have to update the rook to its position. Since king is moving, we don't need to set c.rookMoved
    if (move.castle) {
        int oldRookRow = move.end.row;
        int oldRookCol = (move.end.col == 6) ? 7 : 0;
        int newRookCol = (move.end.col == 6) ? 5 : 3;
        
        c.board[oldRookRow][newRookCol] = {movingColor, ROOK};
        c.board[oldRookRow][oldRookCol] = emptyPiece;
    }

    // 5. If we are performing a promotion, we will also have to update the piece to its type
    if (move.promote != EMPTY && move.promote != PROMOTE) {
        c.board[move.end.row][move.end.col].kind = move.promote;
    }

    // 6. If we are performing a double forward as a pawn, set enPassant
    if (movingPiece == PAWN && abs(move.start.row - move.end.row) > 1) {
        int direction = (move.end.row - move.start.row > 0) ? 1 : -1;
        c.enPassant = {move.start.row + direction, move.start.col};
    }
    else c.enPassant = {-1, -1};
    
    // 7. If we are performing an en passant capture, remove the pawn
    if (move.enPassant) {
        int direction = move.start.row - move.end.row;
        c.board[move.end.row + direction][move.end.col] = emptyPiece;
    }
}

void undoMove(int& gameIndex, vector<Move>& game) {
    if (gameIndex < 0 || gameIndex >= game.size()) return;
    Move move = game[gameIndex]; 
    Color movingColor = c.board[move.end.row][move.end.col].color;
    PieceKind movingPiece = c.board[move.end.row][move.end.col].kind;

    // 0. If we are undoing a promotion, place the piece as a pawn again
    if (move.promote != EMPTY && move.promote != PROMOTE) {
        c.board[move.end.row][move.end.col].kind = PAWN;
    }

    // 1. Undo the move
    c.board[move.start.row][move.start.col] = c.board[move.end.row][move.end.col];
    c.board[move.end.row][move.end.col] = move.targetPiece;

    // 2. If we are aundoing a king move, update its position
    if (movingPiece == KING) {
        c.kingPosition[movingColor] = move.start;
        if (c.kingMoved[movingColor] == gameIndex) c.kingMoved[movingColor] = 0;
    }

    // 3. If we are aundoing a rook move, update its gameIndex
    if (movingPiece == ROOK) {
        if (c.rookMoved[movingColor][0] == gameIndex && move.start.col == 0) c.rookMoved[movingColor][0] = 0;
        else if (c.rookMoved[movingColor][1] == gameIndex && move.start.col == 7) c.rookMoved[movingColor][1] = 0;
    }

    // 4. We have just undone a castle , we will also have to undo the rook move
    if (game[gameIndex].castle) {
        int col = (movingColor == WHITE) ? 7 : 0;
        if (move.start.col == 0) c.rookMoved[movingColor][0] = 0;
        else c.rookMoved[movingColor][1] = 0;
        c.kingMoved[movingColor] = 0;
        
        int oldRookRow = move.end.row;
        int oldRookCol = (move.end.col == 6) ? 7 : 0;
        int newRookCol = (move.end.col == 6) ? 5 : 3;
        
        c.board[oldRookRow][newRookCol] = emptyPiece;
        c.board[oldRookRow][oldRookCol] = {movingColor, ROOK};
    }

    // 5. If we are undoing an en passant capture, set the pawn
    if (move.enPassant) {
        int direction = move.start.row - move.end.row;
        c.board[move.end.row + direction][move.end.col].kind = PAWN;
        Color notMoving = (movingColor == WHITE) ? BLACK : WHITE;
        c.board[move.end.row + direction][move.end.col].color = notMoving;
    }

    c.enPassant = {-1, -1};
    --gameIndex;

    // 6. Restore the previous c.enPassant using the last already done movement
    if (gameIndex < 0 || gameIndex >= game.size()) return; // There is no previous move

    Move prevMove = game[gameIndex];
    movingColor = c.board[prevMove.end.row][prevMove.end.col].color;
    movingPiece = c.board[prevMove.end.row][prevMove.end.col].kind;

    if (movingPiece == PAWN && abs(prevMove.start.row - prevMove.end.row) > 1) {
        int direction = (prevMove.end.row - prevMove.start.row > 0) ? 1 : -1;
        c.enPassant = {prevMove.start.row + direction, prevMove.start.col};
    }
}

bool promote(PieceKind kind, int& gameIndex, vector<Move>& game) {
    if (!c.promotion) return false;

    // 1. Store the last move that did not exchange the pawn
    Move m = game[gameIndex];

    // 2. Undo it
    undoMove(gameIndex, game);

    // 3. Redo it, changing the pawn to the corresponding piece
    m.promote = kind;
    doMove(m, gameIndex, game, true);

    c.promotion = false;
    return true;
}

bool isCheck(Color kingColor) {
    Position kingPosition = c.kingPosition[kingColor];

    // 1. Check if a moving like a rook/bishop you find an enemy rook/bishop/queen
    vector<Position> bishopDirections = {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}};
    vector<Position> rookDirections = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
    for (int i = 0; i < N_DIR_BISHOP_ROOK; ++i) {
        for (int step = 1; step < BOARD_SIZE; ++step) {
            Position newPos = {kingPosition.row + step*bishopDirections[i].row, kingPosition.col + step*bishopDirections[i].col};
            if (!validPos(newPos)) break;

            Piece& targetPiece = c.board[newPos.row][newPos.col];
            if ((targetPiece.kind == BISHOP || targetPiece.kind == QUEEN) && targetPiece.color != kingColor) return true;
            if (targetPiece != emptyPiece) break;
        }

        for (int step = 1; step < BOARD_SIZE; ++step) {
            Position newPos = {kingPosition.row + step*rookDirections[i].row, kingPosition.col + step*rookDirections[i].col};
            if (!validPos(newPos)) break;

            Piece& targetPiece = c.board[newPos.row][newPos.col];
            if ((targetPiece.kind == ROOK || targetPiece.kind == QUEEN) && targetPiece.color != kingColor) return true;
            if (targetPiece != emptyPiece) break;
        }
    }

    // 2. Check if moving like a knight you find an enemy knight
    vector<Position> knightDirections = {{-2, -1}, {-2, 1}, {-1, -2}, {-1, 2}, {1, -2}, {1, 2}, {2, -1}, {2, 1}};
    for (int i = 0; i < N_DIR_KNIGHT; ++i) {
        Position newPos = {kingPosition.row + knightDirections[i].row, kingPosition.col + knightDirections[i].col};
        if (!validPos(newPos)) continue;

        Piece& targetPiece = c.board[newPos.row][newPos.col];
        if (targetPiece.kind == KNIGHT && targetPiece.color != kingColor) return true;
    }

    // 3. Check if moving like a king you find an enemy king
    for (int drow = -1; drow <= 1; ++drow) {
        for (int dcol = -1; dcol <= 1; ++dcol) {
            if (drow == 0 && dcol == 0) continue;

            Position newPos = {kingPosition.row + drow, kingPosition.col + dcol};
            if (!validPos(newPos)) continue;

            Piece& targetPiece = c.board[newPos.row][newPos.col];
            if (targetPiece.kind == KING && targetPiece.color != kingColor) return true;
        }
    }

    // 4. Check if you have pawns in your corners
    int direction = (kingColor == WHITE) ? -1 : 1;
    vector<Position> pawnDirections = {{direction, 1}, {direction, -1}};
    for (int i = 0; i < N_DIR_PAWN; ++i) {
        Position newPos = {kingPosition.row + pawnDirections[i].row, kingPosition.col + pawnDirections[i].col};
        if (!validPos(newPos)) continue;

        Piece& targetPiece = c.board[newPos.row][newPos.col];
        if (targetPiece.kind == PAWN && targetPiece.color != kingColor) return true;
    }

    return false;
}

void tryMove(Move move) {
    // 1. Do the move
    c.board[move.end.row][move.end.col] = c.board[move.start.row][move.start.col];
    c.board[move.start.row][move.start.col] = emptyPiece;

    // 2. Update king to its new position
    if (c.board[move.end.row][move.end.col].kind == KING) {
        c.kingPosition[c.board[move.end.row][move.end.col].color] = move.end;
    }
}

void untryMove(Move move) {
    // 1. Undo the move and set the piece .moved of the piece
    c.board[move.start.row][move.start.col] = c.board[move.end.row][move.end.col];
    c.board[move.end.row][move.end.col] = move.targetPiece;

    // 2. Update king to its old position
    if (c.board[move.start.row][move.start.col].kind == KING) {
        c.kingPosition[c.board[move.start.row][move.start.col].color] = move.start;
    }
}

bool checkIfCheck(const Move &move, Color kingColor) {
    tryMove(move);
    bool result = isCheck(kingColor);
    untryMove(move);
    return result;
}

bool checkFinal(Color turnColor) {
    // 1. Check for valid movements
    for (int row = 0; row < BOARD_SIZE; ++row) {
        for (int col = 0; col < BOARD_SIZE; ++col) {
            if (c.board[row][col].color == turnColor) {
                Position pos = {row, col};
                vector<Move> moves = validMoves(pos);
                if (moves.size() > 0) return false;
            }
        }
    }

    return true;
}



// -- Movements --

vector<Move> kingCastling(Color movingColor) {
    vector<Move> kingCastlingMove;
    Position kingPosition = c.kingPosition[movingColor];

    // 1. Check if the king has already moved or is in check
    if (c.kingMoved[movingColor] || isCheck(movingColor)) return kingCastlingMove;

    // 2. For both left and right castling:
    int row = (movingColor == WHITE) ? 7 : 0;
    vector<int> castlingRookCols = {0, 7};
    vector<vector<int>> castlingEmptyCols = {{1, 2, 3}, {5, 6}};
    for (int i = 0; i < castlingRookCols.size(); ++i) {
        Position rookPos = {row, castlingRookCols[i]};
        Piece& rook = c.board[rookPos.row][rookPos.col];

        // 3. Check if the rook is in position and hasn't moved
        if (rook.kind == ROOK && rook.color == movingColor && !c.rookMoved[movingColor][i]) {
            bool pathClear = true;
            for (int j = 0; j < castlingEmptyCols[i].size(); ++j) {
                int col = castlingEmptyCols[i][j];
                // 4. Check if path is clear
                if (c.board[row][col] != emptyPiece) {
                    pathClear = false;
                    break;
                }

                // 5. Check if the king passes through a square that is attacked
                Position positionChecked = {row, col};
                Move move = {kingPosition, positionChecked, emptyPiece, false, EMPTY, false};
                if (checkIfCheck(move, movingColor)) {
                    pathClear = false;
                    break;
                }
            }

            // 6. If path is clear, add king castling move
            if (pathClear) {
                int newKingCol = (castlingRookCols[i] == 0) ? 2 : 6;
                Position newKingPos = {row, newKingCol};
                Move kingMove = {kingPosition, newKingPos, emptyPiece, true, EMPTY, false};
                kingCastlingMove.push_back(kingMove);
            }
        }
    }

    return kingCastlingMove;
}

vector<Move> kingMoves(Position pos, Color movingColor) {
    vector<Move> possibleMoves;

    // 1. Standard king moves
    for (int drow = -1; drow <= 1; ++drow) {
        for (int dcol = -1; dcol <= 1; ++dcol) {
            if (drow == 0 && dcol == 0) continue;

            Position newPos = {pos.row + drow, pos.col + dcol};
            if (!validPos(newPos)) continue;

            Piece& targetPiece = c.board[newPos.row][newPos.col];
            if (targetPiece == emptyPiece || targetPiece.color != movingColor) {
                Move move = {pos, newPos, targetPiece, false, EMPTY, false};
                if (!checkIfCheck(move, movingColor)) possibleMoves.push_back(move);
            }
        }
    }

    // 2. King castling
    if (!c.kingMoved[movingColor]) possibleMoves += kingCastling(movingColor);

    return possibleMoves;
}

vector<Move> knightMoves(Position pos, Color movingColor) {
    vector<Move> possibleMoves;

    // 1. Standard knight moves
    vector<Position> knightDirections = {{-2, -1}, {-2, 1}, {-1, -2}, {-1, 2}, {1, -2}, {1, 2}, {2, -1}, {2, 1}};
    for (int i = 0; i < N_DIR_KNIGHT; ++i) {
        Position newPos = {pos.row + knightDirections[i].row, pos.col + knightDirections[i].col};
        if (!validPos(newPos)) continue;

        Piece& targetPiece = c.board[newPos.row][newPos.col];
        if (targetPiece == emptyPiece || targetPiece.color != movingColor) {
            Move move = {pos, newPos, targetPiece, false, EMPTY, false};
            if (!checkIfCheck(move, movingColor)) possibleMoves.push_back(move);
        }
    }
    return possibleMoves;
}

vector<Move> pawnMoves(Position pos, Color movingColor) {
    vector<Move> possibleMoves;
    int direction = (movingColor == WHITE) ? -1 : 1;
    int lastRow = (movingColor == WHITE) ? 0 : 8;
    PieceKind promotionPiece = EMPTY;

    // 1. Single square move
    Position singleForward = {pos.row + direction, pos.col};
    if (validPos(singleForward) && c.board[singleForward.row][singleForward.col] == emptyPiece) {
        if (singleForward.row == lastRow) promotionPiece = PROMOTE;
        Move move = {pos, singleForward, emptyPiece, false, promotionPiece, false};
        if (!checkIfCheck(move, movingColor)) possibleMoves.push_back(move);
    }

    // 2. Two squares move from starting position
    int unmovedRow = (movingColor == WHITE) ? 6 : 1;
    if (pos.row == unmovedRow && c.board[pos.row + direction][pos.col].kind == EMPTY) {
        Position doubleForward = {pos.row + 2*direction, pos.col};
        if (validPos(doubleForward) && c.board[doubleForward.row][doubleForward.col] == emptyPiece) {
            Move doubleMove = {pos, doubleForward, emptyPiece, false, EMPTY, false};
            if (!checkIfCheck(doubleMove, movingColor)) possibleMoves.push_back(doubleMove);
        }
    }

    // 3. Capture moves
    vector<Position> captureDirections = {{direction, 1}, {direction, -1}};
    for (int i = 0; i < N_DIR_PAWN; ++i) {
        Position newPos = {pos.row + captureDirections[i].row, pos.col + captureDirections[i].col};
        if (validPos(newPos)) {
            Piece& targetPiece = c.board[newPos.row][newPos.col];
            if (targetPiece != emptyPiece && targetPiece.color != movingColor) {
                if (singleForward.row == lastRow) promotionPiece = PROMOTE;
                Move captureMove = {pos, newPos, targetPiece, false, promotionPiece, false};
                if (!checkIfCheck(captureMove, movingColor)) possibleMoves.push_back(captureMove);
            }
            // 3.1 Check for en passant
            if (newPos == c.enPassant) {
                Move captureMove = {pos, newPos, targetPiece, false, promotionPiece, true};
                if (!checkIfCheck(captureMove, movingColor)) possibleMoves.push_back(captureMove);
            }
        }
    }
    
    return possibleMoves;
}

vector<Move> slideMoves(Position pos, Color movingColor, const vector<Position>& directions) {
    vector<Move> possibleMoves;
    for (int i = 0; i < N_DIR_BISHOP_ROOK; ++i) {
        for (int step = 1; step < BOARD_SIZE; ++step) {
            Position newPos = {pos.row + step*directions[i].row, pos.col + step*directions[i].col};
            if (!validPos(newPos)) break;
            
            Piece& targetPiece = c.board[newPos.row][newPos.col];
            if (targetPiece == emptyPiece || targetPiece.color != movingColor) {
                Move move = {pos, newPos, targetPiece, false, EMPTY, false};
                if (!checkIfCheck(move, movingColor)) possibleMoves.push_back(move);
            }

            if (targetPiece != emptyPiece) break; // Sliding moves do not go through pieces
        }
    }
    return possibleMoves;
}

vector<Move> rookMoves(Position pos, Color movingColor) {
    // 1. Standard rook moves
    vector<Position> rookDirections = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
    return slideMoves(pos, movingColor, rookDirections);
}

vector<Move> bishopMoves(Position pos, Color movingColor) {
    // 1. Standard bishop moves
    vector<Position> bishopDirections = {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}};
    return slideMoves(pos, movingColor, bishopDirections);
}

vector<Move> queenMoves(Position pos, Color movingColor) {
    // 1. Standard queen moves
    return rookMoves(pos, movingColor) + bishopMoves(pos, movingColor);
}

vector<Move> validMoves(Position movingPos) {
    if (!validPos(movingPos)) return {};
    Color movingColor = c.board[movingPos.row][movingPos.col].color;
    PieceKind movingKind = c.board[movingPos.row][movingPos.col].kind;
    switch (movingKind) {
        case BISHOP:
            return bishopMoves(movingPos, movingColor);
        case KING: 
            return kingMoves(movingPos, movingColor);
        case KNIGHT: 
            return knightMoves(movingPos, movingColor);
        case PAWN: 
            return pawnMoves(movingPos, movingColor);
        case QUEEN: 
            return queenMoves(movingPos, movingColor);
        case ROOK: 
            return rookMoves(movingPos, movingColor);
        default:
            return {};
    }
}



// -- Chess notation --

vector<Position> findPossibles(Color movingColor, PieceKind kind, Position pos) {
    int pawnDirection = (movingColor == WHITE) ? 1 : -1;

    vector<Position> directions;
    switch (kind) {
        case QUEEN: directions = {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}, {1, 0}, {-1, 0}, {0, 1}, {0, -1}}; break;
        case BISHOP: directions = {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}}; break;
        case ROOK: directions = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}}; break;
        case KNIGHT: directions = {{-2, -1}, {-2, 1}, {-1, -2}, {-1, 2}, {1, -2}, {1, 2}, {2, -1}, {2, 1}}; break;
        default: return {};
    }

    int maxStep = BOARD_SIZE;
    if (kind == KNIGHT) maxStep = 2; // Knights and pawns do not have steps

    vector<Position> result;
    // 1. Check for the position if any piece of the same kind and color could reach that square
    for (int i = 0; i < directions.size(); ++i) {
        for (int step = 1; step < maxStep; ++step) {
            Position newPos = {pos.row + step*directions[i].row, pos.col + step*directions[i].col};
            if (!validPos(newPos)) break;
            Piece& targetPiece = c.board[newPos.row][newPos.col];
            if ((targetPiece.kind == kind) && targetPiece.color == movingColor) {
                result.push_back(newPos);
                break;
            }
            if (targetPiece != emptyPiece) break;
        }
    }

    return result;
}

string movementToChessNotation(Move move) {
    // 0. If we are translating a castling, return 0-0-0 or 0-0
    if (move.castle) {
        if (move.end.col == 2) return "0-0-0"; // Long castle
        else return "0-0"; // Short castle
    }

    Piece piece = c.board[move.end.row][move.end.col];
    if (move.promote != EMPTY && move.promote != PROMOTE) piece = {piece.color, PAWN};

    string pieceLetter = translatePiece(piece.kind);
    string end = translatePos(move.end);

    // Special case, if a PAWN takes, add its start columm
    if (piece.kind == PAWN && move.start.col != move.end.col) pieceLetter = translateCol(move.start.col);

    // 1. Search for other pieces (not pawn) to resolve ambiguity
    vector<Position> possibles;
    if (piece.kind != PAWN) possibles = findPossibles(piece.color, piece.kind, move.end);
    if (possibles.size() > 1) { // 1.1 We found more than this possible move, resolve ambiguity
        bool diffCol = false;
        bool diffRow = false;
        for (int i = 0; i < possibles.size(); ++i) {
            if (move.start.row != possibles[i].row) diffRow = true;
            if (move.start.col != possibles[i].col) diffCol = true;
        }
        string toAdd;
        if (diffCol && possibles.size() == 2) toAdd = translateCol(move.start.col);
        else if (diffRow && possibles.size() == 2) toAdd = translateRow(move.start.row);
        else if (possibles.size() > 2) toAdd = translateCol(move.start.col) + translateRow(move.start.row);
        pieceLetter += toAdd;
    }

    if (move.targetPiece.kind != EMPTY || move.enPassant) pieceLetter += "x";

    // 2. If we translate a promotion, add "=" + pieceLetter
    if (move.promote) {
        string toAdd = "=";
        if (move.promote != PROMOTE) toAdd += translatePiece(move.promote);
        end += toAdd;
    }

    // 3. If checkmate add "#", if stalemate add "½", if check add "+"
    Color notMoving = (piece.color == WHITE) ? BLACK : WHITE;
    
    bool cannotMove = checkFinal(notMoving);
    bool check = isCheck(notMoving);
        
    string toAdd;
    if (check) toAdd = cannotMove ? "#" : "+";
    else toAdd = cannotMove ? "½" : "";
    end += toAdd;

    // 4. If en passant, add "e.p."
    if (move.enPassant) end += " e.p.";

    return pieceLetter + end;
}



// -- Board --

void iniChessboard() {
    // 1. Empty spaces
    for (int row = 2; row < 6; ++row) {
        for (int col = 0; col < 8; ++col) {
            c.board[row][col] = emptyPiece;
        }
    }

    // 2. Place pawns
    for (int col = 0; col < BOARD_SIZE; col++) {
        c.board[1][col] = blackPawn;
        c.board[6][col] = whitePawn;
    }

    // 3. Place remaining pieces
    vector<Piece> blackPieces = {blackRook, blackKnight, blackBishop, blackQueen, blackKing, blackBishop, blackKnight, blackRook};
    vector<Piece> whitePieces = {whiteRook, whiteKnight, whiteBishop, whiteQueen, whiteKing, whiteBishop, whiteKnight, whiteRook};
    for (int i = 0; i < BOARD_SIZE; i++) {
        c.board[0][i] = blackPieces[i];
        c.board[7][i] = whitePieces[i];
    }

    // 4. Set kings positions
    c.kingPosition[WHITE] = {7, 4};
    c.kingPosition[BLACK] = {0, 4};

    // 5. Set the moved variables for castlings
    c.kingMoved[WHITE] = 0;
    c.kingMoved[BLACK] = 0;
    c.rookMoved[WHITE][0] = 0;
    c.rookMoved[WHITE][1] = 0;
    c.rookMoved[BLACK][0] = 0;
    c.rookMoved[BLACK][1] = 0;

    // 6. Set promotion as false
    c.promotion = false;
}

void printBoard() {
    for (int row = 0; row < BOARD_SIZE; ++row) {
        for (int col = 0; col < BOARD_SIZE; ++col) {
            Piece& piece = c.board[row][col];
            char symbol;
            switch (piece.kind) {
                case BISHOP: symbol = 'B'; break;
                case KING: symbol = 'K'; break;
                case KNIGHT: symbol = 'N'; break;
                case PAWN: symbol = 'P'; break;
                case QUEEN: symbol = 'Q'; break;
                case ROOK: symbol = 'R'; break;
                case EMPTY: symbol = '.'; break;
                case PROMOTE: symbol = '-'; break;
            }
            if (piece.color == BLACK) symbol = tolower(symbol);
            cout << symbol << ' ';
        }
        cout << endl;
    }
    cout << endl;
}


int main(int argc, char *argv[]) {
    // Not used in my WASM implementation

    return 0;
}
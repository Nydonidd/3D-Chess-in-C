#include "raylib.h"
#include <stdio.h>

#define BOARD_SIZE 8
#define ANIMATION_STEPS 30

// Global variables for board and selected piece
char board[BOARD_SIZE][BOARD_SIZE] = {
    {'r', 'n', 'b', 'q', 'k', 'b', 'n', 'r'},  // black pieces
    {'p', 'p', 'p', 'p', 'p', 'p', 'p', 'p'},  // black pawns
    {'.', '.', '.', '.', '.', '.', '.', '.'},  // empty squares
    {'.', '.', '.', '.', '.', '.', '.', '.'},
    {'.', '.', '.', '.', '.', '.', '.', '.'},
    {'.', '.', '.', '.', '.', '.', '.', '.'},
    {'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P'},  // white pawns
    {'R', 'N', 'B', 'Q', 'K', 'B', 'N', 'R'}   // white pieces
};

bool pieceSelected = false;
int selectedRow = -1, selectedCol = -1;
bool legalMoves[BOARD_SIZE][BOARD_SIZE] = { false };
bool whiteKingMoved = false;
bool blackKingMoved = false;
bool whiteRookMoved[2] = { false, false };
bool blackRookMoved[2] = { false, false };
bool attackedSquares[BOARD_SIZE][BOARD_SIZE] = { false };

bool isAnimating = false;
int animationStep = 0;
int startRow, startCol;
int endRow, endCol;
Vector3 currentPosition;

// Chess piece models
Model whitePawn, whiteRook, whiteKnight, whiteBishop, whiteQueen, whiteKing;
Model blackPawn, blackRook, blackKnight, blackBishop, blackQueen, blackKing;

Texture2D whiteTexture;

// Function declarations
void DrawChessBoard(Vector3 boardPosition, float squareSize);
Vector2 GetBoardPosition(Vector3 boardPosition, float squareSize, Vector3 hitPosition);
void DrawPiece(char piece, Vector3 position);
void HighlightLegalMoves(int row, int col);
bool IsOpponentPiece(int r, int c, bool isWhite);
bool IsValidSquare(int r, int c);

void HighlightRookMoves(int row, int col, bool isWhite);
void HighlightBishopMoves(int row, int col, bool isWhite);
void HighlightWhitePawnMoves(int row, int col);
void HighlightBlackPawnMoves(int row, int col);
void HighlightKnightMoves(int row, int col, bool isWhite);
void HighlightKingMoves(int row, int col, bool isWhite);
void MovePiece(int fromRow, int fromCol, int toRow, int toCol);

void AnimatePiece(int fromRow, int fromCol, int toRow, int toCol);

//void MarkAttackedSquares(bool isWhite);


int main(void) {
    InitWindow(1920, 1080, "3D Chess");
    SetTargetFPS(180);
    InitAudioDevice();

    // Define a top-down static camera
    Camera camera = { 0 };
    camera.position = (Vector3){ 5.0f, 12.0f, 8.0f }; // Move the camera to the right and slightly forward (4.0f is the new z position)
    camera.target = (Vector3){ 5.0f, 0.0f, 4.0f };   // Keep the target the same to look at the center of the board
    camera.up = (Vector3){ 0.0f, 0.0f, -1.0f };      // Up direction is inverted
    camera.fovy = 45.0f;                              // Field of view angle
    // Field of view angle

    // load sounds
    Sound moveSound = LoadSound("sounds/move.mp3");
    Sound castleSound = LoadSound("sounds/castle.mp3");
    Music backgroundMusic = LoadMusicStream("sounds/music.mp3");

    // Load chess piece models
    whitePawn = LoadModel("models_assets/WPawn.glb");
    whiteRook = LoadModel("models_assets/WRook.glb");
    whiteKnight = LoadModel("models_assets/WKnight.glb");
    whiteBishop = LoadModel("models_assets/WBishop.glb");
    whiteQueen = LoadModel("models_assets/WQueen.glb");
    whiteKing = LoadModel("models_assets/WKing.glb");

    blackPawn = LoadModel("models_assets/BPawn.glb");
    blackRook = LoadModel("models_assets/BRook.glb");
    blackKnight = LoadModel("models_assets/BKnight.glb");
    blackBishop = LoadModel("models_assets/BBishop.glb");
    blackQueen = LoadModel("models_assets/BQueen.glb");
    blackKing = LoadModel("models_assets/BKing.glb");

    whiteTexture = LoadTexture("models_assets/BetterWhiteTexture.png");

    // Assign texture to white pieces
    for (int i = 0; i < whitePawn.materialCount; i++) {
        whitePawn.materials[i].maps[MATERIAL_MAP_DIFFUSE].texture = whiteTexture;
        whiteRook.materials[i].maps[MATERIAL_MAP_DIFFUSE].texture = whiteTexture;
        whiteKnight.materials[i].maps[MATERIAL_MAP_DIFFUSE].texture = whiteTexture;
        whiteBishop.materials[i].maps[MATERIAL_MAP_DIFFUSE].texture = whiteTexture;
        whiteQueen.materials[i].maps[MATERIAL_MAP_DIFFUSE].texture = whiteTexture;
        whiteKing.materials[i].maps[MATERIAL_MAP_DIFFUSE].texture = whiteTexture;
    }

    // Chessboard settings
    Vector3 boardPosition = { 0.0f, 0.0f, 0.0f };    // Position of the chessboard
    float squareSize = 1.0f;                         // Each square is 1x1 in world units

    SetMusicVolume(backgroundMusic, 0.1f);
    PlayMusicStream(backgroundMusic);

    while (!WindowShouldClose()) {

        UpdateMusicStream(backgroundMusic);

        // Handle mouse input
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            // Get mouse ray
            Ray ray = GetMouseRay(GetMousePosition(), camera);

            // Check for intersection with the ground (chessboard plane, assuming y=0)
            float t = (0.0f - ray.position.y) / ray.direction.y;
            Vector3 hitPosition = {
                ray.position.x + t * ray.direction.x,
                0.0f,
                ray.position.z + t * ray.direction.z
            };

            // Convert the hit position to board coordinates
            Vector2 boardCoords = GetBoardPosition(boardPosition, squareSize, hitPosition);
            int row = (int)boardCoords.y;
            int col = (int)boardCoords.x;

            // Check if click is within the chessboard bounds
            if (row >= 0 && row < BOARD_SIZE && col >= 0 && col < BOARD_SIZE) {
                if (!pieceSelected) {
                    // Select the piece
                    if (board[row][col] != '.') {
                        pieceSelected = true;
                        selectedRow = row;
                        selectedCol = col;

                        HighlightLegalMoves(row, col);
                    }
                } else {
                    if (legalMoves[row][col]) {
                        // Call the MovePiece function to handle the move
                        AnimatePiece(selectedRow, selectedCol, row, col);
                        pieceSelected = false;

                        // Reset legal moves
                        for (int r = 0; r < BOARD_SIZE; r++) {
                            for (int c = 0; c < BOARD_SIZE; c++) {
                                legalMoves[r][c] = false;
                            }
                        }
                    } else {
                        pieceSelected = false; // Deselect piece if move is illegal

                        // Reset legal moves
                        for (int r = 0; r < BOARD_SIZE; r++) {
                            for (int c = 0; c < BOARD_SIZE; c++) {
                                legalMoves[r][c] = false;
                            }
                        }
                    }
                }
            }
        }

        if (isAnimating) {
            animationStep++;

            if (animationStep >= ANIMATION_STEPS) {
                isAnimating = false; // Animation finished
                board[endRow][endCol] = board[startRow][startCol]; // Move the piece on the board
                board[startRow][startCol] = '.'; // Empty the original square
                PlaySound(moveSound);
            }
            else {
                // Interpolate the position in a straight line between start and end
                float t = (float)animationStep / (float)ANIMATION_STEPS;

                // Start and end positions (center of squares)
                float startX = startCol + 0.5f;
                float startZ = startRow + 0.5f;
                float endX = endCol + 0.5f;
                float endZ = endRow + 0.5f;

                // Interpolate the current position along the X and Z axis
                currentPosition.x = startX + (endX - startX) * t; // X axis movement
                currentPosition.z = startZ + (endZ - startZ) * t; // Z axis movement
            }
        }

        // Draw the scene
        BeginDrawing();
        ClearBackground(RAYWHITE);

        BeginMode3D(camera);

        DrawChessBoard(boardPosition, squareSize);

        if (isAnimating) {
            DrawPiece(board[startRow][startCol], currentPosition);
        }

        EndMode3D();

        EndDrawing();
    }

    // Unload models
    UnloadModel(whitePawn);
    UnloadModel(whiteRook);
    UnloadModel(whiteKnight);
    UnloadModel(whiteBishop);
    UnloadModel(whiteQueen);
    UnloadModel(whiteKing);

    UnloadModel(blackPawn);
    UnloadModel(blackRook);
    UnloadModel(blackKnight);
    UnloadModel(blackBishop);
    UnloadModel(blackQueen);
    UnloadModel(blackKing);
    UnloadMusicStream(backgroundMusic);
    UnloadSound(castleSound);

    CloseAudioDevice();
    CloseWindow();
    return 0;
}

void DrawChessBoard(Vector3 boardPosition, float squareSize) {
    for (int row = 0; row < BOARD_SIZE; row++) {
        for (int col = 0; col < BOARD_SIZE; col++) {
            Vector3 position = {
                boardPosition.x + col * squareSize,
                boardPosition.y,
                boardPosition.z + row * squareSize
            };

            Color squareColor = ((row + col) % 2 == 0) ? LIGHTGRAY : DARKGRAY;

            if (legalMoves[row][col]) {
                squareColor = YELLOW;
            }

            if (pieceSelected && row == selectedRow && col == selectedCol) {
                squareColor = GREEN;
            }

            DrawCube(position, squareSize, 0.1f, squareSize, squareColor);

            // Draw the actual piece model if it exists
            DrawPiece(board[row][col], position);
        }
    }
}

// Convert hit position to board coordinates
Vector2 GetBoardPosition(Vector3 boardPosition, float squareSize, Vector3 hitPosition) {
    // Adjust the hit position to the center of the squares
    int col = (int)((hitPosition.x - boardPosition.x + (squareSize / 2.0f)) / squareSize);
    int row = (int)((hitPosition.z - boardPosition.z + (squareSize / 2.0f)) / squareSize);

    // Check if the calculated coordinates are valid
    if (col < 0 || col >= BOARD_SIZE || row < 0 || row >= BOARD_SIZE) {
        return (Vector2) { -1, -1 };  // Return invalid coordinates if the hit is outside the board
    }

    return (Vector2) { col, row };
}

// Function to draw the chess piece model at a given position
void DrawPiece(char piece, Vector3 position) {
    Model* modelToDraw = NULL;
    Color pieceColor = WHITE; // Default to white color

    switch (piece) {
    case 'P': modelToDraw = &whitePawn; break; // White pieces
    case 'R': modelToDraw = &whiteRook; break;
    case 'N': modelToDraw = &whiteKnight; break; // Knight (white)
    case 'B': modelToDraw = &whiteBishop; break;
    case 'Q': modelToDraw = &whiteQueen; break;
    case 'K': modelToDraw = &whiteKing; break;

    case 'p': modelToDraw = &blackPawn; break; // Black pieces
    case 'r': modelToDraw = &blackRook; break;
    case 'n': modelToDraw = &blackKnight; break; // Knight (black)
    case 'b': modelToDraw = &blackBishop; break;
    case 'q': modelToDraw = &blackQueen; break;
    case 'k': modelToDraw = &blackKing; break;

    default: break; // If there is no piece, do nothing
    }

    if (modelToDraw != NULL) {
        // Adjust the position for drawing (lifted slightly above the board)
        Vector3 drawPosition = (Vector3){ position.x, position.y + 0.5f, position.z };

        // Check if the piece is a knight
        if (piece == 'N') {
            // Rotate the white knight 90 degrees around the Y-axis to face forward
            DrawModelEx(*modelToDraw, drawPosition, (Vector3) { 0.0f, 1.0f, 0.0f }, -90.0f, (Vector3) { 0.4f, 0.4f, 0.4f }, pieceColor);
        }
        else if (piece == 'n') {
            // Rotate the black knight 90 degrees around the Y-axis to face forward
            DrawModelEx(*modelToDraw, drawPosition, (Vector3) { 0.0f, 1.0f, 0.0f }, 90.0f, (Vector3) { 0.4f, 0.4f, 0.4f }, pieceColor);
        }
        else {
            // Draw the model without rotation for other pieces
            DrawModel(*modelToDraw, drawPosition, 0.4f, pieceColor);
        }
    }
}

bool IsOpponentPiece(int r, int c, bool isWhite) {
    char p = board[r][c];
    return p != '.' && (isWhite ? p >= 'a' && p <= 'z' : p >= 'A' && p <= 'Z');
}

bool IsValidSquare(int r, int c) {
    return r >= 0 && r < BOARD_SIZE && c >= 0 && c < BOARD_SIZE;
}

void MovePiece(int fromRow, int fromCol, int toRow, int toCol) {
    // Move the piece to the new square
    board[toRow][toCol] = board[fromRow][fromCol];
    board[fromRow][fromCol] = '.'; // Empty the old square
}

void AnimatePiece(int fromRow, int fromCol, int toRow, int toCol) {
    // Store starting and ending positions (center of squares)
    startRow = fromRow;
    startCol = fromCol;
    endRow = toRow;
    endCol = toCol;

    // Set the current position at the center of the starting square
    currentPosition = (Vector3){ startCol + 0.5f, 0.5f, startRow + 0.5f };
    animationStep = 0;
    isAnimating = true; // Start the animation
}

void HighlightLegalMoves(int row, int col) {
    for (int r = 0; r < BOARD_SIZE; r++) {
        for (int c = 0; c < BOARD_SIZE; c++) {
            legalMoves[r][c] = false;
        }
    }

    char piece = board[row][col];

    switch (piece) {
    case 'P': // White Pawn
        HighlightWhitePawnMoves(row, col);
        break;
    case 'p': // Black Pawn
        HighlightBlackPawnMoves(row, col);
        break;

    case 'R': case 'r':// Rooks
        HighlightRookMoves(row, col, piece == 'R');
        break;

    case 'N': case 'n': // Knights
        HighlightKnightMoves(row, col, piece == 'N');
        break;

    case 'B': case 'b': // Bishops
        HighlightBishopMoves(row, col, piece == 'B');
        break;

    case 'Q': case 'q': // Queens
        HighlightRookMoves(row, col, piece == 'Q');
        HighlightBishopMoves(row, col, piece == 'Q');
        break;

    case 'K': case 'k': // Kings
        HighlightKingMoves(row, col, piece == 'K');
        break;
    }
}


void HighlightRookMoves(int row, int col, bool isWhite) {
    char piece = board[row][col];

    for (int i = row - 1; i >= 0; i--) { // UP
        if (board[i][col] == '.') legalMoves[i][col] = true;
        else {
            if (IsOpponentPiece(i, col, piece == 'R')) {
                legalMoves[i][col] = true;
            }
            break;
        }
    }

    for (int i = row + 1; i < BOARD_SIZE; i++) { // DOWN
        if (board[i][col] == '.') legalMoves[i][col] = true;
        else {
            if (IsOpponentPiece(i, col, piece == 'R')) {
                legalMoves[i][col] = true;
            }
            break;
        }
    }

    for (int i = col - 1; i >= 0; i--) { // LEFT
        if (board[row][i] == '.') legalMoves[row][i] = true;
        else {
            if (IsOpponentPiece(row, i, piece == 'R')) {
                legalMoves[row][i] = true;
            }
            break;
        }
    }

    for (int i = col + 1; i < BOARD_SIZE; i++) { // RIGHT
        if (board[row][i] == '.') legalMoves[row][i] = true;
        else {
            if (IsOpponentPiece(row, i, piece == 'R')) {
                legalMoves[row][i] = true;
            }
            break;
        }
    }
}


void HighlightBishopMoves(int row, int col, bool isWhite) {
    char piece = board[row][col];

    // UP LEFT
    for (int i = 1; i < BOARD_SIZE; i++) {
        int newRow = row - i, newCol = col - i;
        if (!IsValidSquare(newRow, newCol)) break;

        if (board[newRow][newCol] == '.') {
            legalMoves[newRow][newCol] = true;
        }
        else {
            if (IsOpponentPiece(newRow, newCol, isWhite)) {
                legalMoves[newRow][newCol] = true;
            }
            break;
        }
    }

    // UP RIGHT
    for (int i = 1; i < BOARD_SIZE; i++) {
        int newRow = row - i, newCol = col + i;
        if (!IsValidSquare(newRow, newCol)) break;

        if (board[newRow][newCol] == '.') {
            legalMoves[newRow][newCol] = true;
        }
        else {
            if (IsOpponentPiece(newRow, newCol, isWhite)) {
                legalMoves[newRow][newCol] = true;
            }
            break;
        }
    }

    // DOWN LEFT
    for (int i = 1; i < BOARD_SIZE; i++) {
        int newRow = row + i, newCol = col - i;
        if (!IsValidSquare(newRow, newCol)) break;

        if (board[newRow][newCol] == '.') {
            legalMoves[newRow][newCol] = true;
        }
        else {
            if (IsOpponentPiece(newRow, newCol, isWhite)) {
                legalMoves[newRow][newCol] = true;
            }
            break;
        }
    }

    // DOWN RIGHT
    for (int i = 1; i < BOARD_SIZE; i++) {
        int newRow = row + i, newCol = col + i;
        if (!IsValidSquare(newRow, newCol)) break;

        if (board[newRow][newCol] == '.') {
            legalMoves[newRow][newCol] = true;
        }
        else {
            if (IsOpponentPiece(newRow, newCol, isWhite)) {
                legalMoves[newRow][newCol] = true;
            }
            break;
        }
    }
}

void HighlightWhitePawnMoves(int row, int col) {
    if (IsValidSquare(row - 1, col) && board[row - 1][col] == '.') {
        legalMoves[row - 1][col] = true;
        if (row == 6 && board[row - 2][col] == '.') {
            legalMoves[row - 2][col] = true;
        }
    }

    if (IsValidSquare(row - 1, col - 1) && IsOpponentPiece(row - 1, col - 1, true)) {
        legalMoves[row - 1][col - 1] = true;
    }
    if (IsValidSquare(row - 1, col + 1) && IsOpponentPiece(row - 1, col + 1, true)) {
        legalMoves[row - 1][col + 1] = true;
    }
}

void HighlightBlackPawnMoves(int row, int col) {
    if (IsValidSquare(row + 1, col) && board[row + 1][col] == '.') {
        legalMoves[row + 1][col] = true;
        if (row == 1 && board[row + 2][col] == '.') {
            legalMoves[row + 2][col] = true;
        }
    }

    if (IsValidSquare(row + 1, col - 1) && IsOpponentPiece(row + 1, col - 1, false)) {
        legalMoves[row + 1][col - 1] = true;
    }
    if (IsValidSquare(row + 1, col + 1) && IsOpponentPiece(row + 1, col + 1, false)) {
        legalMoves[row + 1][col + 1] = true;
    }
}

void HighlightKnightMoves(int row, int col, bool isWhite) {
    {
        int knightMoves[8][2] = {
            {-2, -1}, {-2, 1}, {-1, -2}, {-1, 2},
            {1, -2}, {1, 2}, {2, -1}, {2, 1}
        };

        for (int i = 0; i < 8; i++) {
            int newRow = row + knightMoves[i][0];
            int newCol = col + knightMoves[i][1];
            if (IsValidSquare(newRow, newCol) &&
                (board[newRow][newCol] == '.' || IsOpponentPiece(newRow, newCol, isWhite))) {
                legalMoves[newRow][newCol] = true;
            }
        }
    }
}

void HighlightKingMoves(int row, int col, bool isWhite) {
    memset(legalMoves, false, sizeof(legalMoves)); // Resets the legalMoves array

    // Check all surrounding squares for legal moves
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            if (i == 0 && j == 0) continue; // Skip current position

            int newRow = row + i;
            int newCol = col + j;

            if (IsValidSquare(newRow, newCol)) {
                // Check if the square is empty or occupied by an opponent's piece
                if (board[newRow][newCol] == '.' || IsOpponentPiece(newRow, newCol, isWhite)) {
                    // Check if the square is not under attack
                    if (!attackedSquares[newRow][newCol]) {
                        legalMoves[newRow][newCol] = true; // Mark the square as a legal move
                    }
                }
            }
        }
    }

    // Handle castling
    if (isWhite && !whiteKingMoved) {
        // Kingside castling - white
        if (!whiteRookMoved[1] && board[7][5] == '.' && board[7][6] == '.'
            && !attackedSquares[7][5] && !attackedSquares[7][6]) {
            legalMoves[7][6] = true;
        }

        // Queenside castling - white
        if (!whiteRookMoved[0] && board[7][3] == '.' && board[7][2] == '.'
            && board[7][1] == '.' && !attackedSquares[7][3] && !attackedSquares[7][2]
            && !attackedSquares[7][1]) {
            legalMoves[7][2] = true;
        }
    }
    else if (!isWhite && !blackKingMoved) {
        // Kingside castling - black
        if (!blackRookMoved[1] && board[0][5] == '.' && board[0][6] == '.'
            && !attackedSquares[0][5] && !attackedSquares[0][6]) {
            legalMoves[0][6] = true;
        }

        // Queenside castling - black
        if (!blackRookMoved[0] && board[0][3] == '.' && board[0][2] == '.'
            && board[0][1] == '.' && !attackedSquares[0][3] && !attackedSquares[0][2]
            && !attackedSquares[0][1]) {
            legalMoves[0][2] = true;
        }
    }
}

// old code
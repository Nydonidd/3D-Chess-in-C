# 3D Chess in C

A 3D chess board written in C with [raylib](https://www.raylib.com/). Started while I was first learning C; the board, piece movement and move highlighting work, but full rules (check, checkmate, en passant, promotion) are not finished.

## What it does

- Renders an 8x8 board and GLB piece models with a fixed top-down camera
- Mouse picking: casts a ray from the cursor onto the board plane to work out which square was clicked
- Highlights legal moves for the selected piece (pawn, rook, knight, bishop, queen, king), including capture squares
- Tracks whether kings and rooks have moved so castling can be validated
- Animates pieces between squares over 30 frames rather than teleporting them
- Move and castle sound effects plus background music

## Building

Built with Visual Studio (solution file included). raylib needs to be installed and linked. Run from the repo root so the relative `models_assets/` and `sounds/` paths resolve.

## Status

Unfinished. I moved on to other projects before adding check detection and end-of-game states. Left up because the ray-casting and move-generation code is still a reasonable reference for anyone doing something similar in raylib.

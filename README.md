# Chess Project Documentation

## Introduction
This project is a **browser-based chess game** built using **C++** and **Emscripten**. It features a functional and user-friendly interface designed with **HTML**, **CSS**, and **JavaScript**. While the initial goal was to create a chess game entirely in C++, I decided to leverage **WebAssembly (WASM)** to bring the project to the web, creating an engaging visual experience.

The core chess logic resides in `chess.hh` and `chess.cc`, which can also be reused in other C++ chess applications. This project demonstrates a balance between performance and usability, integrating advanced features while maintaining simplicity.


## Features
- **Full Chess Rules**: All standard chess rules are implemented, including special moves like castling, en passant, and pawn promotion.
- **WebAssembly for Performance**: The game logic is compiled into **WebAssembly** for seamless execution in modern browsers.
- **Stockfish Integration**: A **Stockfish engine** (ported to WASM) is included, allowing users to play against the computer or adjust difficulty via a settings tab.
- **Responsive UI**: A clean and simple UI designed for both desktop and mobile browsers, with draggable pieces and a visual representation of the game state.

## Compile
To compile and run the project locally, follow these steps:

1. **Install Emscripten**: Ensure you have the Emscripten SDK installed. You can follow the instructions [here](https://emscripten.org/docs/getting_started/downloads.html).
   
2. **Compile the Code**: Run the following command to compile or directly use `chess.js` and `chess.wasm` the C++ code into WebAssembly:
   ```bash
   make
3. **Run Locally**: Open the generated chess.html file in any modern web browser to play the game locally.

## Documentation
This project utilizes a combination of open-source tools and assets. Below are links to the key components and resources used:
- [Emscripten documentation](https://emscripten.org) — Documentation for compiling C++ into WebAssembly.
- [Stockfish.js](https://github.com/lichess-org/stockfish.js) — A WebAssembly port of the Stockfish chess engine.
- [Chess Piece SVGs](https://commons.wikimedia.org/w/index.php?curid=1499811) — Chess piece graphics designed by Cburnett, used under a Creative Commons license.

## Acknowledgements
Special thanks to:
- **Lichess.org** for providing the WebAssembly port of the Stockfish engine, which enhances the gameplay experience.
- **Cburnett** for designing the beautiful chess piece SVGs that are used in this project.

## License
This project is licensed under the [GNU General Public License v3.0](LICENSE).
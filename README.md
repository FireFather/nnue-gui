# nnue-gui
This is a basic tool/aid for training and creating stockfish-nnue eval networks (nn.bin)
using my fork
https://github.com/FireFather/Stockfish-nnue

of nodchip's stockfish-nnue software:
https://github.com/nodchip/Stockfish/releases

Because of logic changes, version 1.2 no longer work with nodchip's sodtware.

It's a basic GUI to keep track of settings, paths, UCI options, command line parameters, etc.
It will also launch the various binaries needed for all 3 phases: gen training data, gen validation data,
and learning (training).

It's a native 64-bit Windows application, with no external dependencies, and should run on most systems.

![alt tag](https://raw.githubusercontent.com/FireFather/nnue-gui/master/nnue-gui.png)

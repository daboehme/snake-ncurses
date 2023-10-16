Snake (Ncurses)
===============

A simple snake game in C for UNIX consoles using the ncurses library.
Written as a programming exercise and ncurses example.
The game should be familiar to anyone who grew up with a Nokia phone
or still remembers the MS QBasic
[nibbles](https://en.wikipedia.org/wiki/Nibbles_(video_game))
example.

Build
---------------

You'll need the `ncurses-devel` package on most Linux distributions.
Then, clone the github repository, and then either simply build the
program with `make`:

    $ git clone https://github.com/daboehme/snake-ncurses.git
    $ cd snake-ncurses
    $ make

Or use the CMake build:

    $ cd snake-ncurses
    $ mkdir build && cd build
    $ cmake ..
    $ cmake --build .

Usage
---------------

Use the arrow keys or WASD to move. You can pause with SPACE and quit
the game with `q`.

You can select a difficulty (game speed) between 0 and 10 with
the `-d` argument, e.g.

    $ snake -d 6

The default difficulty is 5.

Copyright
---------------

Copyright (c) 2023 David Boehme.

This program is released under the MIT license. See [LICENSE](LICENSE) for
details.
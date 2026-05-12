# minesweeper
minesweeper for a computer science final

in order to run the file enter the folowing in the terminal
gcc -D_XOPEN_SOURCE=700 minesweeper.c -o minesweeper $(pkg-config --cflags --libs notcurses)
and then run the file with ./minesweeper

this program was made in an arch linux environment, it may be configured differently on other computers

this program uses the notcurses library to provide a TUI

this program features: 
configurable board sizes,
liar and checkerboard gamemodes and
mouse input

basic gameplay:
left click to reveal tiles, if one of them is a bomb, you lose
right click to flag an unrevealed tile, you need to correctly flag all the bombs to win
numbers appear based on how many mines are in their area eg:
0 + +
+ 4 0
0 0 +
(in this scenario, assume that + = mine and 0 = hidden)

checkerboard gameplay:
this gamemode turns every other cell into one where, if a bomb is placed there, will count for two eg:
0 O 0
+ 4 +
x O 0
(in this scenario, assume that 0 = empty odd, O = empty even, + = even mine and x = odd mine)

liar gameplay:
every number is off by exactly 1 in each direction

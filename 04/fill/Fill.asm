// This file is part of www.nand2tetris.org
// and the book "The Elements of Computing Systems"
// by Nisan and Schocken, MIT Press.
// File name: projects/04/Fill.asm

// Runs an infinite loop that listens to the keyboard input.
// When a key is pressed (any key), the program blackens the screen
// by writing 'black' in every pixel;
// the screen should remain fully black as long as the key is pressed. 
// When no key is pressed, the program clears the screen by writing
// 'white' in every pixel;
// the screen should remain fully clear as long as no key is pressed.

@8192
D=A
@R1
M=D

(LOOP)
	// detect key press
	@KBD
	D=M

	@WHITE
	D;JEQ

	@BLACK
	D;JNE

(BLACK)
	@n
	M=0
	@LOOPBLACK
	0;JMP

// blacken everything, starting from @SCREEN
(LOOPBLACK)
	// if n==R1, go to LOOP
	@n
	D=M
	@R1
	D=D-M
	@LOOP
	D;JEQ

	@SCREEN
	D=A
	@n
	A=D+M
	M=-1
	@n
	M=M+1
	@LOOPBLACK
	0;JMP

(WHITE)
	@n
	M=0
	@LOOPWHITE
	0;JMP

// whiten everything, starting from @SCREEN
(LOOPWHITE)
	// if n==R1, go to LOOP
	@n
	D=M
	@R1
	D=D-M
	@LOOP
	D;JEQ

	@SCREEN
	D=A
	@n
	A=D+M
	M=0
	@n
	M=M+1
	@LOOPWHITE
	0;JMP

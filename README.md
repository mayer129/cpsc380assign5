# cpsc380assign5
Assignment 5: Implementing a pseudo real-time scheduler
## Information
*  @author: Travis Mayer
*     ID: 002273275
*     Email: mayer129@mail.chapman.edu
*     Course: CPSC 380-01

## Program Purpose
"The objective of this assignment is to use POSIX-based semaphores to implement a simulated real-time scheduling algorithm."
## Steps to Compile & Run:
1. Path to the rmsched directory
2. type `gcc rmsched.c -o rmsched` to compile the program
3. type `./rmsched <nPeriods> <taskSet file> <schedule file>`
* This will compile the program. The program will take command line input in the form of the nPeriods, a taskSet file location, and a sdchedule file location.
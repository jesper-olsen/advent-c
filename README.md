# Advent - A Modern C Port of Colossal Cave Adventure 

This repo contains a modernised C version of Donald Knuth's implementation of the classic Colossal Cave Adventure ([Advent](https://www-cs-faculty.stanford.edu/~knuth/programs/advent.w.gz)). The original CWEB source has been translated and refactored using standard C23.

See also the [Rust](https://github.com/jesper-olsen/advent) port of the same program.

## Project Goals

* Modernisation: Update the codebase to use standard C (C23) and eliminate deprecated or non-standard patterns.
* Readability: Refactor the code into logical modules and improve formatting to make it easier to study and understand.
* Performance: Optimise critical paths, such as the command parser, using modern techniques.
* Faithfulness: Keep the game logic, room descriptions, and text identical to the original, preserving the classic experience.

## Technical Highlights

The primary technical philosophy of this port is to shift work from runtime to compile time, resulting in a faster, safer, and more robust executable. This was applied to the two most critical data structures in the game: the command vocabulary and the location data.

* Compile-Time Command Parser: The original game built a hash table in memory every time it started. This version replaces that with a compile-time perfect hash function.
   *  A custom generator program (`generator.c`) encodes the vocabulary's 5-letter words into unique 35-bit integers.
   * These integers are used as `case` labels in a massive, auto-generated C switch statement (`vocab_jmp.h`), creating an O(1) lookup with zero startup cost and zero dynamic memory allocation.
* Static Game Data Initialization: In the original, all static game data—including the long and short room descriptions—was loaded via dynamic initialization routines at startup. This has been completely replaced with const static arrays.
   * Increased Safety: This change places all game text into the read-only `.rodata` memory segment, protecting it from potential runtime corruption or buffer overflows.
   * Instant Startup: The data is part of the executable image and requires no setup, further reducing the time from launch to the first prompt.

## Build

Prerequisites:

* A C compiler supporting C23 (e.g., Clang17+ or GCC 13+).
* `make`

Build the game: 

```bash
make 
```

This will first build a small generator tool, use it to create vocab_jmp.h, and then compile the main 
`advent` executable.


## Run

```bash
./advent
```

Example session:

```text
Welcome to Adventure!!  Would you like instructions?
** y
Somewhere nearby is Colossal Cave, where others have found fortunes in
treasure and gold, though it is rumored that some who enter are never
seen again.  Magic is said to work in the cave.  I will be your eyes
and hands.  Direct me with commands of one or two words.  I should
warn you that I look at only the first five letters of each word, so
you'll have to enter "NORTHEAST" as "NE" to distinguish it from
"NORTH".  Should you get stuck, type "HELP" for some general hints.
For information on how to end your adventure, etc., type "INFO".
                        -  -  -
The first adventure program was developed by Willie Crowther.
Most of the features of the current program were added by Don Woods;
all of its bugs were added by Don Knuth.

You are standing at the end of a road before a small brick building.
Around you is a forest.  A small stream flows out of the building and
down a gully.
*
```

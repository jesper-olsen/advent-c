## Advent - Colossal Cave Adventure (C Version)

This repo is based on Donald Knuth's C implementation of [Advent](https://www-cs-faculty.stanford.edu/~knuth/programs/advent.w.gz), the classic Colossal Cave Adventure. It has been updated for modern C (C23) and lightly refactored for readability and maintainability.

See also the [Rust](https://github.com/jesper-olsen/advent) port of the same program.

## Build

```bash
make 
```

This uses Clang with -std=c23 and common warning flags.
You can adjust the compiler or options in the provided Makefile.

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

## Notes

* Written in standard C23 and compatible with Clang 17+.
* Includes minor modernization (e.g. bool, const, static inline, Makefile).
* Game logic and text remain faithful to the original.

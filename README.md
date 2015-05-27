# RocketScience

The *RocketScience* state-of-the-art model checker for sequential recursive recursive integer programs was developed during a masters project in the [Concurrency Theory Group](http://concurrency.informatik.uni-kl.de) at the [University of Kaiserslautern](http://cs.uni-kl.de) during a masters project.

It is implemented in C++ and integrates the following techniques:

* CEGAR loop
* Predicate Abstraction
* Procedure Summaries
* Reachability Analysis
* Refinement based on Craig Interpolants


## Dependencies

* Boost >= 1.57 (C++11 support)
* CMake >= 3.1.0
* [Z3](http://z3.codeplex.com) >= 4.3.2 (with C++ bindings)
* ~~[parserlib](/axilmar/parserlib)~~ (included)
* ~~[CUDD](http://vlsi.colorado.edu/~fabio/CUDD/)~~ (included)


## Build Instructions

Just `make` the project. The result can be found in the `build` folder

*Tested on Mac OS X Yosemite 10.10.3 with Command Line Tools 6.2.*


## Usage

After making the project, you can simply call `RocketScience` with a file containing a program. You can run a simple test via `make test` or specify your own program like so:
```
build/test/RocketScience path/to/program.c
```

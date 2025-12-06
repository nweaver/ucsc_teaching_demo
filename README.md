# Teaching Demo

This was initially demonstration code for Nicholas Weaver's UCSC Teaching
Professor interview, and has since been adapted as a more set of general 
code.

It contains code for doing a shortest path traversal in Kotlin, C++,
and Python.  For all three the code is designed to create an
iterator/sequence structure, enabling the lazy application of the
computation.

The C++ version is C++20, as C++ didn't even introduce "contains" into
its storage data types until C++20.

All three versions include testing code.  Python and Kotlin use their
respective common testing frameworks (pytest and junit5 respectively).
The C++ version uses Google Test with CMake.

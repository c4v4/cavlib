# cavlib
A collection of C++ utilities I frequently reuse in my projects.

All the utilities you find here have been built and tuned around my specific needs (hence the name). In the future, some components might disappear or be completely changed, so there is no guarantee about the stability of the code. For the same reason, this library assumes that it will be compiled using the C++20 standard with the clang/gcc compiler on Linux machines.

## List of content:
Only files and folder in `include/cav` are listed here, ignoring `experimental`.

### `comptime`
 - [`call_utils`](include/cav/comptime/call_utils.hpp): utilities for handling and manipulating function calls, lambda functions, and function pointers.
 - [`enum_name`](include/cav/comptime/enum_name.hpp): utilities for mapping and retrieving names of enum values at compile-time.
 - [`instance_of`](include/cav/comptime/instance_of.hpp): utilities for checking if a type is an instance of a specific template.
 - [`macros`](include/cav/comptime/macros.hpp): preprocessor macros for debug logging, function inlining, function flattening, stringification, etc.
 - [`mp_base`](include/cav/comptime/mp_base.hpp): basic building blocks for metaprogramming, including wrappers, pack, unique_wrap, value_wrap, and utilities for compile-time computations.
 - [`mp_utils`](include/cav/comptime/mp_utils.hpp): utilities for manipulating and querying type properties, type traits, type lists operations, and compile-time checks.
 - [`syntactic_sugars`](include/cav/comptime/syntactic_sugars.hpp): syntactic sugars for common C++ constructs, e.g., perfect forwarding, type manipulation, and conditional types, to reduce verbosity and boilerplate.
 - [`test`](include/cav/comptime/test.hpp): utilities for constexpr testing, including expression failure checks and block execution tests.
 - [`type_name`](include/cav/comptime/type_name.hpp): utilities for retrieving the name of a type at compile-time.

### `datastruct`
 - [`UnionFind`](include/cav/datastruct/UnionFind.hpp): data structure for disjoint-set operations.

### `mish`
 - [`Chrono`](include/cav/mish/Chrono.hpp): wrapper around C++ chrono library.
 - [`errors`](include/cav/mish/errors.hpp): macros and functions for handling exceptions and errors, with support for both exception-enabled and exception-disabled environments.
 - [`RaiiWrap`](include/cav/mish/RaiiWrap.hpp): RAII wrapper for managing resources, providing automatic cleanup when the wrapper goes out of scope.
 - [`util_functions`](include/cav/mish/util_functions.hpp): utility functions simple enought to be deemed reusable in multiple projects.

### `numeric`
 - [`limits`](include/cav/numeric/limits.hpp): wrapper around std::numeric_limits
 - [`random`](include/cav/numeric/random.hpp): random number generation utilities
 - [`ScaledInt`](include/cav/numeric/ScaledInt.hpp): fixed-point arithmetic with customizable scaling, base, rounding, and underlying integral type
 - [`sort`](include/cav/numeric/sort.hpp): hooks optimized for sorting short sequences.
 - [`sorting_networks`](include/cav/numeric/sorting_networks.hpp): sorting networks for small-sized inputs up to size 32.
 - [`TaggedScalar`](include/cav/numeric/TaggedScalar.hpp):  wraps a native arithmetic type, defining explicit conversions between different TaggedScalars and implicit conversions with native types
 - [`TolFloat`](include/cav/numeric/TolFloat.hpp): floating point number wrapper, providing a tolerance for comparisons.
 - [`XoshiroCpp`](include/cav/numeric/XoshiroCpp.hpp): [Ryo Suzuki XoshiroCpp](https://github.com/Reputeless/Xoshiro-cpp/blob/master) C++ porting of the Xoshiro pseudo-random number generator based on David Blackman and Sebastiano Vigna's [xoshiro generator](http://prng.di.unimi.it/).
 - [`zero`](include/cav/numeric/zero.hpp): represents the zero value of any default-constructible.

### `string`
 - [`StaticStr`](include/cav/string/StaticStr.hpp): template for compile-time string manipulation and conversion.
 - [`string_utils`](include/cav/string/string_utils.hpp): collection of string manipulation functions.

### `tuplish`
 - [`dependencies`](include/cav/tuplish/dependencies.hpp): system for resolving dependencies between types, with support for both lazy and tidy resolution approaches.
 - [`tuple`](include/cav/tuplish/tuple.hpp): based on type_map, integral constant are used as keys in the map.
 - [`type_map`](include/cav/tuplish/type_map.hpp): compile-time map from types to values, with various utility methods for manipulation and access.
 - [`type_set`](include/cav/tuplish/type_set.hpp): based on type_map, compile-time set from types to their values, with various utility methods for manipulation and access.

### `vectors`
 - [`GrowArray`](include/cav/vectors/GrowArray.hpp): std::array wrapper with std::vector-like operations for known max size but partial usage.
 - [`IndexProxyIter`](include/cav/vectors/IndexProxyIter.hpp): custom iterator for indexable containers, supporting random access and arithmetic operations.
 - [`MatrixKD`](include/cav/vectors/MatrixKD.hpp): multi-dimensional matrix class with dynamic dimensions and size.
 - [`OffsetVec`](include/cav/vectors/OffsetVec.hpp): vector-like container with an offset, allowing for negative indexing and operations at both ends.
 - [`OwnSpan`](include/cav/vectors/OwnSpan.hpp): span-like container that owns its data and deallocates it on destruction.
 - [`SoAArray`](include/cav/vectors/SoAArray.hpp): simplified data structure providing easy access to either Structure of Arrays (SoA) or Array of Structures (AoS) data types, focusing on easy SoA/AoS access pattern and conversion.


## Why make it public?
There are a couple of reasons:

- I want to have it as a separate library, and if it is public, I can be free to use it even in my public repositories.
- I would be very happy if some discussion were to generate from this (though I don't expect it to happen). I like to make my (sometimes) strong opinions collide with others as a way of reciprocal contamination.
- I want to have a repo that evolves with me, showcasing my current state as a software developer. In the past few years, I have mostly coded only on "private" projects, either because there was a hard requirement or because I didn't consider my work completed or worthy to be public.

 ## Style Summary
Here's an unsorted list of principles I try to follow:

- Refactor-Oriented Programming: If an abstraction doesn't fit well its purpose, feel free to change, remove, split, merge, or do whatever it takes to fix it,  even if it means spending days on a refactor. Minor inconveniences add up, so it's better to deal with them now than later when it will be much more painful. Moreover, the pleasure to work with well thought abstractions outweight the hustle of the refactor (which is also a mean of exploring new ideas and improving as a developer). The simplest example of this approach translates into the following point.

- If you remember the name of a function wrong, rename it with the wrong name. This usually converges into a small set of names (usually one) that are equally reasonable to you. The main benefits are that the names become meaningful, and often, you don't have to remember the name but just think, "How would I name it?" and it works.

- User code: (aka: my code in other repositories that depend on this one) should be  **simple and clear&#8482;**. What this means changes over time with my personal opinion and taste, but at the time of this writing, it can be summarized as:
    - Avoid direct allocations (use library code for that).
    - Put asserts everywhere (both comptime and runtime).
    - Debug vs Release build: provide a version able to detected and fail on errors
    - Be explicit with functions (nodiscard, noexcept, const, ...).
    - Prefer objects with full public members.
    - Use default-constructible aggregate objects.
    - Defer all complexities to library code (e.g., allocations, template metaprogramming shenanigans, nifty abstractions, etc.).
    - User code shoud be understood by C programmes that never learnt C++ idioms.
    - Feel free to ignore any of the previous rules if you have a valid reason.

- Library code: This repository should contain library code. Library code is free to do whatever hideous things are necessary to expose a useful, clean, and performant abstraction.

## Comments
I'm confident that all the nifty or horrific ideas you might encounter here can easily be found in many other open-source C++ libraries. I don't expect you'll find anything original here.

This is a simple snapshot of what I enjoy writing when I'm coding in C++.
My coding style has evolved with time, influenced by commonly suggested best practices but mainly by firsthand experiences and thoughts.

Feel free to contact me for anything—whether you like or dislike something, have suggestions or critiques, or if you'd like to do something similar to a thing that you found here and want to know more about how it works.

## Todos:
- I need to start writing unit tests systematically. All the components have been used at a certain point in time in one or more projects, but this is far to consider them bug-free. In a way, the constexpr testing facilities in `test.hpp`` mitigate this point, but they are far from fix it.
- The documentation is pretty much non-existent. For many meta-functions or simple utilities it is not a problem (since they are small enough to be self-explanatory). For larger components I need to add, at the very least, a form of contract.


## External Stuff:
- [Ryo Suzuki XoshiroCpp](https://github.com/Reputeless/Xoshiro-cpp/blob/master) C++ porting of the Xoshiro pseudo-random number generator based on David Blackman and Sebastiano Vigna's [xoshiro generator](http://prng.di.unimi.it/).
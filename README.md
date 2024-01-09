# cavlib
A collection of C++ utilities I frequently reuse in my projects.

All the utilities you find here have been built and tuned around my specific needs (hence the name). In the future, some components might disappear or be completely changed, so there is no guarantee about the stability of the code. For the same reason, this library assumes that it will be compiled using the C++20 standard with the clang/gcc compiler on Linux machines.

## Why is it public then?
There are a couple of reasons:

- I want to have it as a separate library, and if it is public, I can be free to use it even in my public repositories.
- I would be very happy if some discussion were to generate from this (though I don't expect it to happen). I like to make my (sometimes) strong opinions collide with others as a way of reciprocal contamination.
- I want to have a repo that evolves with me, showcasing my current state as a software developer. In the past few years, I have mostly coded only on "private" stuff, either because there was a hard requirement or because I didn't consider my work completed or worthy to be public.

## Comments
I'm confident that all the nifty or horrific ideas you might encounter here can easily be found in many other open-source C++ libraries. I don't expect you'll find anything original here.

This is a simple snapshot of what I enjoy writing when I'm coding in C++.
My coding style has evolved with time, influenced by commonly suggested best practices but mainly by firsthand experiences and thoughts.

Feel free to contact me for anything—whether you like or dislike something, have suggestions or critiques, or if you'd like to do something similar to a thing that you found here and want to know more about how it works.

## Todos:
- I need to start writing unit tests systematically. Currently, all the components have been used at a certain point in time in one or more projects, but this is far to consider them bug-free.
- The documentation is pretty much non-existent. For many meta-functions or simple utilities it is not a problem (since they are small enough to be self-explanatory). For larger components I need to add, at the very least, a form of contract.


## External Stuff:
- [Ryo Suzuki XoshiroCpp](https://github.com/Reputeless/Xoshiro-cpp/blob/master) C++ porting of the Xoshiro pseudo-random number generator based on David Blackman and Sebastiano Vigna's [xoshiro generator](http://prng.di.unimi.it/).
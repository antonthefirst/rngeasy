# Rngeasy
A "re-editable" gamedev focused random number generator library for CPU and GPU.

## Why
Generating numbers and other mathematical objects at random without bias is often complex and subtle. Articles and papers about any given case exist, but there doesn't seem to be a great collection of these functions to use as easy reference. In games and other interactive or creative applications, we often have a need for a wide variety of these generation functions, so having them all in one place seems handy. In addition, even the topic of generating raw bits is quite deep and full of tradeoffs in speed, state size, and quality. There is no one-size-fits-all solution to this, but there is a pretty good bread and butter solution for the specific kinds of constraints in apps mentioned above (realtime, GPU-friendly, human judged). Lastly, having the same code run on CPU and GPU is very handy since these days a lot of our heavy lifting is done on GPU, and it's nice if it's the same results there.

## Usage

The library is written "C-Style" so that it can be run on both CPU and GPU with exactly the same code.  If you prefer a C++ object syntax, it should be easy to wrap it.
```
RngState rng;
u32 k = u32To(17);   // Generate an unsigned int between 0 and 17
s32 x = s32In(rng, -1, 13);     // Generate a signed int between -1 and 13
float t = floatUnit(rng);       // Generate a 0-1 inclusive float
float a = floatEUnit(rng);      // Generate a 0-1 exclusive float (useful for angles on a circle, if mul'ed by 2*PI)
vec3 n = vec3OnUnitSphere(rng); // Generate a vector on a unit sphere (useful for normals or directions or noise)
vec3 n = vec2InUnitCircle(rng); // Generate a point in a 2D circle

```
...and so on, peruse the header to find more :)

## Architecture
This library follows Knuth's advice of being "re-editable" rather than reusable. It's written to be easily dropped in and modified to fit a particular project's need, rather than be extended. There are two main reasons for this, one pragmatic and one philosophical. The pragmatic one is simply that to be compatible with GPU, there is just no real option for fancy language features that allow for even basic polymorphism (to switch out bit generators on the fly, for example). The philosophical one is simply that in my experience it's easier to pluck these functions out and put them into your codebase when they have minimal dependencies, so creating a framework to make this all extensible is counter-productive to that.

The library is basically in two parts: the bit generator and the functions that make use of it to create numbers, vectors, and so on. If you would like to use a different generator to optimize for your own specific constraints (more speed, better quality, etc) then simply replace that bit of code and the creation functions should continue working as before. Likewise you can rename or add creation functions to taste, taking advantage of the pretty good core generator and initialization.

For higher level objects like vectors and quaternions, a stub implementation is provided just to get the code to compile. The intention here is that you either replace it with your own version (by including it and shimmying the appropriate defines) or just lift the relevant functions into your own library.

## Tradeoffs
Any random number generator library has to wrestle with tradeoffs in speed, distribution quality, correctness, size, and so forth. These tradeoffs can happen in the bit generator itself or in the usage of those bits to create numbers and other objects. For this library, the tradeoff decisions are as follows:

First off, the functions creating numbers and objects from bits should be as correct as possible (so please report any bugs you find :) In a few cases, this makes them slightly slower and somewhat pedantic. But I think it's useful to see the subtle and correct versions of these functions, since the naive or approximate versions are easier to find or deduce.

Second, the functions should be readable and portable, even if that means they aren't fully optimal. Pretty much everything in the library can be done faster if optimized for a particular bespoke case.

Third, the bit generator itself should have "pretty good" quality, but still be "pretty fast". It's harder to put it more concretely than that, but I think in practice games and other related apps have a decently consistent definition of this for the common cases. The statistical distribution doesn't need to be crypto strong or even as good as possible, it just needs to be good enough to fool the human eye very well. At the same time, it should be good enough that the programmer doesn't have to worry about which bits are better than others or encountering stretches of bad quality.

Lastly, I chose to target primarily 32 bit integers and floats, because these are by far the most common. In the rare case you need a 64 bit integer, you can roll two 32 bit ones together. Besides to generate UID's of 64 bits or more, different (higher quality) approaches are better anyway. Likewise if you have an honest need for doubles, you are probably doing something scientific so please do it more carefully than this lib so nobody dies :)

To sum it up: this library's viewpoint is that CPUs and GPUs are fast enough now so that things can be done correctly enough to avoid workarounds. In cases where that's not the case, bespoke variations can be hand coded for those cases, with the correct versions serving as rough guides. At the same time, let's not waste any more cycles than we need to fool humans, so that in the best case you can just use this as is, and save yourself the optimization time :)

## Customization

The default bit generator I use is the Xoroshiro64 from https://prng.di.unimi.it/

64bits of state seems like a good balance between having quality and still running decently fast on CPU/GPU without much gymnastics.

It would be nice in this section to put some links to alternatives though, so all the various common choices are in one place, with their tradeoffs listed. So #TODO. Could also add code for them to drop into the main lib, to speed up trying different ones out.

## Demos

Shadertoy test: https://www.shadertoy.com/view/NssXzn
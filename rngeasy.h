/* CPU/GPU cross compatibility. */
#ifdef _WIN32
#pragma once

/* For copysign. */
#include <math.h>

/* An alias for an unsigned 32bit integer, used as the generic "bits" of state. */
typedef unsigned int bits32;

/* An alias for unsigned and signed 32bit integers, used as the random numbers. */
typedef unsigned int u32;
typedef int s32;

/*
   A placeholder for a quaternion structure, replace it with your own.
   Convention is Hamiltonian, component order xyzw.
*/
struct quat {
	float x, y, z, w;
	quat() { }
	quat(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) { }
};

/*
   A placeholder for a vector2 structure, replace it with your own.
*/
struct vec2 {
	float x, y;
	vec2() { }
	vec2(float _x, float _y) : x(_x), y(_y) { }
	vec2(float _s) : x(_s), y(_s) { }
	vec2 operator*(const vec2& rhs) const { return vec2(x * rhs.x, y * rhs.y); }
};

/*
   A placeholder for a vector3 structure, replace it with your own.
*/
struct vec3 {
	float x, y, z;
	vec3() { }
	vec3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) { }
	vec3(float _s) : x(_s), y(_s), z(_s) { }
	vec3 operator*(const vec3& rhs) const { return vec3(x * rhs.x, y * rhs.y, z * rhs.z); }
};

/* Math operators. */
#define floatSin sinf
#define floatCos cosf
#define floatSqrt sqrtf
inline float inversesqrt(float x) { return 1.0f / floatSqrt(x); } // GPU has a special function for this
inline float dot(const vec3& a, const vec3& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
inline float sign(float x) { // GLSL compatibility version
	if (x > 0.0f) return 1.0f;
	else if (x < 0.0f) return -1.0f;
	else return 0.0f;
}

/* Needed because GPU needs to ignore it. */
#define INLINE inline

#define RNGSTATE_REF RngState&

#pragma warning( push )
#pragma warning( disable : 4146 ) // using unary minus on unsigned values

#else

/* An alias for an unsigned 32 bit integer, used as the generic "bits" of state. */
#define bits32 uint

/* An alias for unsigned and signed 32bit integers, used as the random numbers. */
#define u32 uint
#define s32 int

/* On GPU you probably want quaternions to just be vec4's. */
#define quat vec4

/* Math operators. */
#define floatSin sin
#define floatCos cos
#define floatSqrt sqrt

/* GPU doesn't care about inline. */
#define INLINE  


#define RNGSTATE_REF inout RngState

#endif

/* Limits and specifications. */
#define FLOAT32_MANT_DIG     24                      // # of bits in mantissa

/* Mathematical constants. */
#define PI 3.14159265358979323846f
#define TAU (PI*2.0f)

/*
   Generator state and core function definitions.
   To replace the generator, any implementation that fits this api should work.
*/
struct RngState { bits32 s0; bits32 s1; }; // Can't forward declare structs on GPU...
RngState rngSeed(bits32 seed_bits);
bits32   rngAdvance(RNGSTATE_REF rng);

/*
   Integers module.
*/

u32 u32Any(RNGSTATE_REF);                  // [0, INT32_MAX)
u32 u32To(RNGSTATE_REF, u32 max);          // [0, max)
u32 u32In(RNGSTATE_REF, u32 min, u32 max); // [min, max]
s32 s32In(RNGSTATE_REF, s32 min, s32 max); // [min, max]

/*
   Distributions module.
*/
u32  dice(RNGSTATE_REF, u32 sides);                        // Simulate an N-sided die, returns [0, SIDES)
u32  diceNoRepeat(RNGSTATE_REF, u32 sides, u32 prev_roll); // Same as dice(), but ensure that the roll is different from PREV_ROLL.
bool oneIn(RNGSTATE_REF, u32 chance);                      // 1:CHANCE that it returns true. (aka. dice(CHANCE) == 0)

/*
   Floats module.
*/
float floatUnit(RNGSTATE_REF);                      // [0.0, 1.0]
float floatEUnit(RNGSTATE_REF);                     // [0.0, 1.0)
float floatIn(RNGSTATE_REF, float min, float max);  // [min, max]

/*
   Vectors module.
*/
vec2 vec2InUnitCircle(RNGSTATE_REF);
vec2 vec2OnUnitCirle(RNGSTATE_REF);
vec3 vec3InUnitSphere(RNGSTATE_REF); // #TODO
vec3 vec3OnUnitSphere(RNGSTATE_REF);
vec3 vec3OnUnitHemisphere(RNGSTATE_REF, vec3 normal); // #TODO

/*
   Quaternions module.
*/
quat quatAny(RNGSTATE_REF);

/*
   Function implementations.
*/
u32 u32Any(RNGSTATE_REF rng) {
	return rngAdvance(rng);
}
u32 u32To(RNGSTATE_REF rng, u32 max) {
	/*
	   This code is adapted from: http://www.pcg-random.org, basic generator C file (http://www.pcg-random.org/downloads/pcg-c-basic-0.9.zip)
	   The loop is guaranteed to terminate if the generator is uniform.
	   The chance of this loop re-rolling is roughly proportional to the range
	   For small ranges up to say 1024, the chance is around 0.00002%, or 1 in 4 million.
	   For medium ranges of say 2^16, the changes is around 0.002% or 1 in 66 thousand.
	   So it's safe to say for "most" cases, it will return without actually looping.
	*/
	u32 threshold = -max % max;
	for (;;) {
		u32 r = rngAdvance(rng);
		if (r >= threshold)
			return r % max;
	}
}
u32 u32In(RNGSTATE_REF rng, u32 range_min, u32 range_max) {
	return range_min + u32To(rng, range_max - range_min + u32(1)); // _to excludes the max, so +1
}
s32 s32In(RNGSTATE_REF rng, s32 range_min, s32 range_max) {
	return range_min + s32(u32To(rng, u32(range_max - range_min + 1))); // _to excludes the max, so +1
}
u32 dice(RNGSTATE_REF rng, u32 sides) {
	return u32To(rng, sides);
}
/* #TODO Go Alan go! :)
u32 diceNoRepeat(RNGSTATE_REF rng, u32 sides, u32 prev_roll) {

}
*/
bool oneIn(RNGSTATE_REF rng, u32 chance) {
	return chance != u32(0) ? dice(rng, chance) == u32(0) : false;
}

/* To have the correct distribution, we cannot simply divide by INT32_MAX, since a float cannot express that value exactly. */
/* https://prng.di.unimi.it/ section "Generating uniform doubles in the unit interval" */
INLINE float unitFromBits(u32 bits) {
	/* Divide is required here for strict correctness; "times_one_over" optimization yields slight excess of 1.0 values. (512 of ~16M instead of 256) */
	return float(bits >> (32 - FLOAT32_MANT_DIG)) / float((1 << FLOAT32_MANT_DIG) - 1);
}
INLINE float eunitFromBits(u32 bits) {
	/* Unlike the [0,1] case, the "times_one_over" optimization yields identical results, so we can apply it here. */
	return float(bits >> (32 - FLOAT32_MANT_DIG)) * (1.0f / float((1 << FLOAT32_MANT_DIG)));
}

float floatUnit(RNGSTATE_REF rng) {
	return unitFromBits(rngAdvance(rng));
}
float floatEUnit(RNGSTATE_REF rng) {
	return eunitFromBits(rngAdvance(rng));
}
/* #TODO make sure Snit and ESnit are actually unbiased down to the bit level. */
float floatSnit(RNGSTATE_REF rng) {
	u32 b = rngAdvance(rng);
	return unitFromBits(b) * sign((b & u32(0x8000000)) != u32(0) ? +1.0f : -1.0f);
}
float floatESnit(RNGSTATE_REF rng) {
	u32 b = rngAdvance(rng);
	return eunitFromBits(b) * sign((b & u32(0x8000000)) != u32(0) ? +1.0f : -1.0f);
}

float floatIn(RNGSTATE_REF rng, float range_min, float range_max) {
	return range_min + (range_max - range_min) * floatUnit(rng);
}

vec2 vec2InUnitCircle(RNGSTATE_REF rng) {
	float a = floatEUnit(rng) * TAU;
	float r = floatSqrt(floatUnit(rng));
	return vec2(sin(a), cos(a)) * vec2(r);
}
vec2 vec2OnUnitCirle(RNGSTATE_REF rng) {
	float a = floatEUnit(rng) * TAU;
	return vec2(sin(a), cos(a));
}
vec3 vec3OnUnitSphere(RNGSTATE_REF rng) {
	/* #TODO source? */
	float s = floatUnit(rng) * TAU;
	float t = floatUnit(rng) * 2.0f - 1.0f;
	return vec3(floatSin(s), floatCos(s), t) * vec3(inversesqrt(1.0f + t * t));
}
vec3 vec3OnUnitHemisphere(RNGSTATE_REF rng, vec3 normal) {
	/* #TODO source? */
	vec3 v = vec3OnUnitSphere(rng);
	return v * sign(dot(v, normal));
}

quat quatAny(RNGSTATE_REF rng) {
	// from http://planning.cs.uiuc.edu/node198.html
	float u1 = floatUnit(rng);
	float u2 = floatUnit(rng);
	float u3 = floatUnit(rng);
	return quat(floatSqrt(1.0f - u1) * floatSin(TAU * u2), floatSqrt(1.0f - u1) * floatCos(TAU * u2), floatSqrt(u1) * floatSin(TAU * u3), floatSqrt(u1) * floatCos(TAU * u3));
}


/*
   Generator implementation:
   Xoroshiro64StarStar generator, seeded with splitmix32.
   http://xoshiro.di.unimi.it/xoroshiro64starstar.c
   https://stackoverflow.com/questions/17035441/looking-for-decent-quality-prng-with-only-32-bits-of-state
*/

INLINE bits32 splitmix32(bits32 b) {
	b += bits32(0x9e3779b9);
	b ^= b >> 15;
	b *= bits32(0x85ebca6b);
	b ^= b >> 13;
	b *= bits32(0xc2b2ae3d);
	b ^= b >> 16;
	return b;
}
INLINE RngState rngSeed(bits32 seed_bits) {
	RngState rng;
	/* s0 and s1 cannot both be 0 for xoroshiro generator, so to avoid this we hash the seed twice
	   with splitmix32. Since it is bijective, it guarantees that at least one of them is non-zero
	   (because if one is zero, it would hash to nonzero). */
	rng.s1 = splitmix32(seed_bits);
	rng.s0 = splitmix32(rng.s1);
	return rng;
}
INLINE u32 rotl(const u32 x, s32 k) { return (x << k) | (x >> (32 - k)); }
INLINE bits32 rngAdvance(RNGSTATE_REF rng) {
	u32 s0 = rng.s0;
	u32 s1 = rng.s1;
	u32 result_starstar = rotl(s0 * u32(0x9E3779BB), 5) * u32(5);
	s1 ^= s0;
	rng.s0 = rotl(s0, 26) ^ s1 ^ (s1 << 9); // a, b
	rng.s1 = rotl(s1, 13); // c
	return result_starstar;
}

/*
   Adapted into a stateless versionfrom:
   https://blog.demofox.org/2013/07/06/fast-lightweight-random-shuffle-functionality-fixed/
   https://github.com/Atrix256/RandomCode/blob/master/StoragelessShuffle/Source.cpp
*/
INLINE u32 shuffle(u32 idx, u32 count, u32 seed) {
	/* Pre-calculate the masks needed for the Feistel network. */
	/* If doing this for many indecies in a row, it would make sense to cache this. */
	u32 next_pow_4 = u32(4);
	while (count > next_pow_4)
		next_pow_4 *= u32(4);
	u32 num_bits = u32(0);
	u32 mask = next_pow_4 - u32(1);
	while (mask != u32(0)) {
		mask = mask >> 1;
		num_bits++;
	}
	u32 half_num_bits = num_bits / u32(2);
	u32 right_mask = (u32(1) << half_num_bits) - u32(1);
	u32 left_mask = right_mask << half_num_bits;

	/* This will terminate as long as idx starts less than count. */
	while (true)
	{
		/* Split the index. */
		u32 left = (idx & left_mask) >> half_num_bits;
		u32 right = (idx & right_mask);
		/* Do 4 Feistel rounds. */
		for (int index = 0; index < 4; ++index)
		{
			u32 newLeft = right;
			u32 newRight = left ^ (splitmix32(right ^ seed) & right_mask);
			left = newLeft;
			right = newRight;
		}
		/* Re-assemble the bits into a shuffled index. */
		idx = (left << half_num_bits) | right;

		/* If it's in range, we are done. Otherwise keep trying (reject this result) */
		if (idx < count)
			return idx;
	}
	/* We should never get here. */
	return u32(0);
}

#ifdef _WIN32
#pragma warning( pop )
#endif
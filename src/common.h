#pragma once
#include "../include/nocta.h"
#include "fixedpoint.h"

#define MAX(A,B) ((A)>(B) ? (A) : (B))
#define MIN(A,B) ((A)<(B) ? (A) : (B))

// keep N within the range MIN..MAX
#define CLAMP(N,MIN,MAX) ((N)<(MIN)?(MIN):((N)>(MAX)?(MAX):(N)))


inline static int clip(int x) {
	return CLAMP(x, INT16_MIN, INT16_MAX);
}
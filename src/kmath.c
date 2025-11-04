#include <math.h>
#include "raylib.h"
#include "raymath.h"
#include "kmath.h"

float AngleLerp(float a, float b, float t) {
	t *= 100.0f;
	float d = b - a;

	while(d >  180) d -= 360;
	while(d < -180) d += 360;

	return a + (d * t);
}

float ILerp(float a, float b, float t, float delta_time) {
	return 1 - Lerp(a, b, pow(t, delta_time));
}

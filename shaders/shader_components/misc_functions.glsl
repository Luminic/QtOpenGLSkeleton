#ifndef MISC_FUNCTIONS_GLSL
#define MISC_FUNCTIONS_GLSL

float henyey_greenstein(float cos_theta, float G) {
	// This is the Henyey Greenstein Phase Function
	// Equation from https://www.astro.umd.edu/~jph/HG_note.pdf
	float result = 1 - G*G;
	result /= 4 * 3.1415926535f * pow(1 + G*G - 2*G*cos_theta, 1.5f);
	return result;
}

float linear_depth(float depth) {
  float near = 0.1;
  float far  = 100.0;
  float z = 2 * depth - 1;
  return (2.0 * near * far) / (far + near - z * (far - near)) / far;
}

float non_linear_depth(float depth) {
  float near = 0.1;
  float far  = 100.0;
	float z = ((2.0 * near * far)/(depth*far) - far - near)/(near-far);
	z = (z+1)/2;
	return z;
}

#endif

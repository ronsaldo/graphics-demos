/*
 * -----------------------------------------------------------------------------
 * The following code was adapted by Ronie Salgado from a Java implementation
 * of Simplex Noise. Reproduced below is the original copyright notice.
 * This adapted code is also placed in the public domain.
 * -----------------------------------------------------------------------------
 * A speed-improved simplex noise algorithm for 2D, 3D and 4D in Java.
 *
 * Based on example code by Stefan Gustavson (stegu@itn.liu.se).
 * Optimisations by Peter Eastman (peastman@drizzle.stanford.edu).
 * Better rank ordering method by Stefan Gustavson in 2012.
 *
 * This could be speeded up even further, but it's useful as it is.
 *
 * Version 2012-03-09
 *
 * This code was placed in the public domain by its original author,
 * Stefan Gustavson. You may use it as you see fit, but
 * attribution is appreciated.
 *
 */

#ifndef _T3_NOISE_HPP
#define _T3_NOISE_HPP

#include "CommonCL.hpp"

static const __constant Vector4 noise_grad3_table[] = {
    constant_vector4(1,1,0,0), constant_vector4(-1,1,0,0), constant_vector4(1,-1,0,0), constant_vector4(-1,-1,0,0),
    constant_vector4(1,0,1,0), constant_vector4(-1,0,1,0), constant_vector4(1,0,-1,0), constant_vector4(-1,0,-1,0),
    constant_vector4(0,1,1,0), constant_vector4(0,-1,1,0), constant_vector4(0,1,-1,0), constant_vector4(0,-1,-1,0)
};

static const __constant Vector4 noise_grad4_table[]= {
    constant_vector4(0,1,1,1), constant_vector4(0,1,1,-1), constant_vector4(0,1,-1,1), constant_vector4(0,1,-1,-1),
    constant_vector4(0,-1,1,1), constant_vector4(0,-1,1,-1), constant_vector4(0,-1,-1,1), constant_vector4(0,-1,-1,-1),
    constant_vector4(1,0,1,1), constant_vector4(1,0,1,-1), constant_vector4(1,0,-1,1), constant_vector4(1,0,-1,-1),
    constant_vector4(-1,0,1,1), constant_vector4(-1,0,1,-1), constant_vector4(-1,0,-1,1), constant_vector4(-1,0,-1,-1),
    constant_vector4(1,1,0,1), constant_vector4(1,1,0,-1), constant_vector4(1,-1,0,1), constant_vector4(1,-1,0,-1),
    constant_vector4(-1,1,0,1), constant_vector4(-1,1,0,-1), constant_vector4(-1,-1,0,1), constant_vector4(-1,-1,0,-1),
    constant_vector4(1,1,1,0), constant_vector4(1,1,-1,0), constant_vector4(1,-1,1,0), constant_vector4(1,-1,-1,0),
    constant_vector4(-1,1,1,0), constant_vector4(-1,1,-1,0), constant_vector4(-1,-1,1,0), constant_vector4(-1,-1,-1,0)
};

static const __constant short noise_permutations[] = {151,160,137,91,90,15,
  131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
  190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
  88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
  77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
  102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
  135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
  5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
  223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
  129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
  251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
  49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
  138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180,
  151,160,137,91,90,15,
  131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
  190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
  88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
  77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
  102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
  135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
  5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
  223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
  129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
  251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
  49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
  138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180,
};

static const __constant short noise_permutations_mod12[] = {
 7, 4, 5, 7, 6, 3, 11, 1, 9, 11, 0, 5, 2, 5, 7, 9, 8, 0, 7, 6, 9, 10, 8, 3, 1,
 0, 9, 10, 11, 10, 6, 4, 7, 0, 6, 3, 0, 2, 5, 2, 10, 0, 3, 11, 9, 11, 11, 8, 9,
 9, 9, 4, 9, 5, 8, 3, 6, 8, 5, 4, 3, 0, 8, 7, 2, 9, 11, 2, 7, 0, 3, 10, 5, 2, 2,
 3, 11, 3, 1, 2, 0, 7, 1, 2, 4, 9, 8, 5, 7, 10, 5, 4, 4, 6, 11, 6, 5, 1, 3, 5, 1,
 0, 8, 1, 5, 4, 0, 7, 4, 5, 6, 1, 8, 4, 3, 10, 8, 8, 3, 2, 8, 4, 1, 6, 5, 6, 3,
 4, 4, 1, 10, 10, 4, 3, 5, 10, 2, 3, 10, 6, 3, 10, 1, 8, 3, 2, 11, 11, 11, 4, 10,
 5, 2, 9, 4, 6, 7, 3, 2, 9, 11, 8, 8, 2, 8, 10, 7, 10, 5, 9, 5, 11, 11, 7, 4, 9,
 9, 10, 3, 1, 7, 2, 0, 2, 7, 5, 8, 4, 10, 5, 4, 8, 2, 6, 1, 0, 11, 10, 2, 1, 10,
 6, 0, 0, 11, 11, 6, 1, 9, 3, 1, 7, 9, 2, 11, 11, 1, 0, 10, 7, 1, 7, 10, 1, 4, 0,
 0, 8, 7, 1, 2, 9, 7, 4, 6, 2, 6, 8, 1, 9, 6, 6, 7, 5, 0, 0, 3, 9, 8, 3, 6, 6, 11,
 1, 0, 0, 7, 4, 5, 7, 6, 3, 11, 1, 9, 11, 0, 5, 2, 5, 7, 9, 8, 0, 7, 6, 9, 10, 8,
 3, 1, 0, 9, 10, 11, 10, 6, 4, 7, 0, 6, 3, 0, 2, 5, 2, 10, 0, 3, 11, 9, 11, 11,
 8, 9, 9, 9, 4, 9, 5, 8, 3, 6, 8, 5, 4, 3, 0, 8, 7, 2, 9, 11, 2, 7, 0, 3, 10, 5,
 2, 2, 3, 11, 3, 1, 2, 0, 7, 1, 2, 4, 9, 8, 5, 7, 10, 5, 4, 4, 6, 11, 6, 5, 1, 3,
 5, 1, 0, 8, 1, 5, 4, 0, 7, 4, 5, 6, 1, 8, 4, 3, 10, 8, 8, 3, 2, 8, 4, 1, 6, 5,
 6, 3, 4, 4, 1, 10, 10, 4, 3, 5, 10, 2, 3, 10, 6, 3, 10, 1, 8, 3, 2, 11, 11, 11,
 4, 10, 5, 2, 9, 4, 6, 7, 3, 2, 9, 11, 8, 8, 2, 8, 10, 7, 10, 5, 9, 5, 11, 11, 7,
 4, 9, 9, 10, 3, 1, 7, 2, 0, 2, 7, 5, 8, 4, 10, 5, 4, 8, 2, 6, 1, 0, 11, 10, 2, 1,
 10, 6, 0, 0, 11, 11, 6, 1, 9, 3, 1, 7, 9, 2, 11, 11, 1, 0, 10, 7, 1, 7, 10, 1, 4,
 0, 0, 8, 7, 1, 2, 9, 7, 4, 6, 2, 6, 8, 1, 9, 6, 6, 7, 5, 0, 0, 3, 9, 8, 3, 6, 6,
 11, 1, 0, 0, 
};

// Skewing and unskewing factors for 2, 3, and 4 dimensions
__constant const float Noise_F2 = 0.3660254037844386; // 0.5f*(sqrt(3.0f)-1.0f);
__constant const float Noise_G2 = 0.21132486540518713; //(3.0f-sqrt(3.0f))/6.0f;
__constant const float Noise_F3 = 1.0f/3.0;
__constant const float Noise_G3 = 1.0f/6.0;
__constant const float Noise_F4 = 0.30901699437494745; //(sqrt(5.0f)-1.0f)/4.0f;
__constant const float Noise_G4 = 0.1381966011250105;  //(5.0f-sqrt(5.0f))/20.0f;

// 2D simplex noise
inline float simplex_noise2D(float xin, float yin)
{
    float n0, n1, n2; // Noise contributions from the three corners
    // Skew the input space to determine which simplex cell we're in

    float s = (xin+yin)*Noise_F2; // Hairy factor for 2D
    int i = floor(xin+s);
    int j = floor(yin+s);
    float t = (i+j)*Noise_G2;
    float X0 = i-t; // Unskew the cell origin back to (x,y) space
    float Y0 = j-t;
    float x0 = xin-X0; // The x,y distances from the cell origin
    float y0 = yin-Y0;

    // For the 2D case, the simplex shape is an equilateral triangle.
    // Determine which simplex we are in.
    int i1, j1; // Offsets for second (middle) corner of simplex in (i,j) coords
    if(x0>y0) {i1=1; j1=0;} // lower triangle, XY order: (0,0)->(1,0)->(1,1)
    else {i1=0; j1=1;}      // upper triangle, YX order: (0,0)->(0,1)->(1,1)

    // A step of (1,0) in (i,j) means a step of (1-c,-c) in (x,y), and
    // a step of (0,1) in (i,j) means a step of (-c,1-c) in (x,y), where
    // c = (3-sqrt(3))/6
    float x1 = x0 - i1 + Noise_G2; // Offsets for middle corner in (x,y) unskewed coords
    float y1 = y0 - j1 + Noise_G2;
    float x2 = x0 - 1.0 + 2.0 * Noise_G2; // Offsets for last corner in (x,y) unskewed coords
    float y2 = y0 - 1.0 + 2.0 * Noise_G2;

    // Work out the hashed gradient indices of the three simplex corners
    int ii = i & 255;
    int jj = j & 255;
    int gi0 = noise_permutations_mod12[ii + noise_permutations[jj]];
    int gi1 = noise_permutations_mod12[ii+i1 + noise_permutations[jj+j1]];
    int gi2 = noise_permutations_mod12[ii+1 + noise_permutations[jj+1]];

    // Calculate the contribution from the three corners
    float t0 = 0.5f - x0*x0-y0*y0;
    if(t0<0) n0 = 0.0f;
    else {
      t0 *= t0;
      n0 = t0 * t0 * dot(noise_grad3_table[gi0], make_vector4(x0, y0, 0, 0));  // (x,y) of grad3 used for 2D gradient
    }
    float t1 = 0.5f - x1*x1-y1*y1;
    if(t1<0) n1 = 0.0f;
    else {
      t1 *= t1;
      n1 = t1 * t1 * dot(noise_grad3_table[gi1], make_vector4(x1, y1, 0, 0));
    }
    float t2 = 0.5f - x2*x2-y2*y2;
    if(t2<0) n2 = 0.0f;
    else {
      t2 *= t2;
      n2 = t2 * t2 * dot(noise_grad3_table[gi2], make_vector4(x2, y2, 0, 0));
    }
    // Add contributions from each corner to get the final noise value.
    // The result is scaled to return values in the interval [-1,1].
    return 70.0f * (n0 + n1 + n2);
}

// 3D simplex noise
inline float simplex_noise3D(float xin, float yin, float zin) {
    float n0, n1, n2, n3; // Noise contributions from the four corners
    // Skew the input space to determine which simplex cell we're in
    float s = (xin+yin+zin)*Noise_F3; // Very nice and simple skew factor for 3D
    int i = floor(xin+s);
    int j = floor(yin+s);
    int k = floor(zin+s);
    float t = (i+j+k)*Noise_G3;
    float X0 = i-t; // Unskew the cell origin back to (x,y,z) space
    float Y0 = j-t;
    float Z0 = k-t;
    float x0 = xin-X0; // The x,y,z distances from the cell origin
    float y0 = yin-Y0;
    float z0 = zin-Z0;
    // For the 3D case, the simplex shape is a slightly irregular tetrahedron.
    // Determine which simplex we are in.
    int i1, j1, k1; // Offsets for second corner of simplex in (i,j,k) coords
    int i2, j2, k2; // Offsets for third corner of simplex in (i,j,k) coords
    if(x0>=y0) {
      if(y0>=z0)
        { i1=1; j1=0; k1=0; i2=1; j2=1; k2=0; } // X Y Z order
        else if(x0>=z0) { i1=1; j1=0; k1=0; i2=1; j2=0; k2=1; } // X Z Y order
        else { i1=0; j1=0; k1=1; i2=1; j2=0; k2=1; } // Z X Y order
      }
    else { // x0<y0
      if(y0<z0) { i1=0; j1=0; k1=1; i2=0; j2=1; k2=1; } // Z Y X order
      else if(x0<z0) { i1=0; j1=1; k1=0; i2=0; j2=1; k2=1; } // Y Z X order
      else { i1=0; j1=1; k1=0; i2=1; j2=1; k2=0; } // Y X Z order
    }
    // A step of (1,0,0) in (i,j,k) means a step of (1-c,-c,-c) in (x,y,z),
    // a step of (0,1,0) in (i,j,k) means a step of (-c,1-c,-c) in (x,y,z), and
    // a step of (0,0,1) in (i,j,k) means a step of (-c,-c,1-c) in (x,y,z), where
    // c = 1/6.
    float x1 = x0 - i1 + Noise_G3; // Offsets for second corner in (x,y,z) coords
    float y1 = y0 - j1 + Noise_G3;
    float z1 = z0 - k1 + Noise_G3;
    float x2 = x0 - i2 + 2.0f*Noise_G3; // Offsets for third corner in (x,y,z) coords
    float y2 = y0 - j2 + 2.0f*Noise_G3;
    float z2 = z0 - k2 + 2.0f*Noise_G3;
    float x3 = x0 - 1.0f + 3.0f*Noise_G3; // Offsets for last corner in (x,y,z) coords
    float y3 = y0 - 1.0f + 3.0f*Noise_G3;
    float z3 = z0 - 1.0f + 3.0f*Noise_G3;

    // Work out the hashed gradient indices of the four simplex corners
    int ii = i & 255;
    int jj = j & 255;
    int kk = k & 255;
    int gi0 = noise_permutations_mod12[ii + noise_permutations[jj + noise_permutations[kk]]];
    int gi1 = noise_permutations_mod12[ii + i1 + noise_permutations[jj + j1 + noise_permutations[kk + k1]]];
    int gi2 = noise_permutations_mod12[ii + i2 + noise_permutations[jj + j2 + noise_permutations[kk + k2]]];
    int gi3 = noise_permutations_mod12[ii + 1 + noise_permutations[jj + 1 + noise_permutations[kk + 1]]];

    // Calculate the contribution from the four corners
    float t0 = 0.6f - x0*x0 - y0*y0 - z0*z0;
    if(t0<0) n0 = 0.0f;
    else {
      t0 *= t0;
      n0 = t0 * t0 * dot(noise_grad3_table[gi0], make_vector4(x0, y0, z0, 0));
    }
    float t1 = 0.6f - x1*x1 - y1*y1 - z1*z1;
    if(t1<0) n1 = 0.0f;
    else {
      t1 *= t1;
      n1 = t1 * t1 * dot(noise_grad3_table[gi1], make_vector4(x1, y1, z1, 0));
    }
    float t2 = 0.6f - x2*x2 - y2*y2 - z2*z2;
    if(t2<0) n2 = 0.0f;
    else {
      t2 *= t2;
      n2 = t2 * t2 * dot(noise_grad3_table[gi2], make_vector4(x2, y2, z2, 0));
    }
    float t3 = 0.6f - x3*x3 - y3*y3 - z3*z3;
    if(t3<0) n3 = 0.0f;
    else {
      t3 *= t3;
      n3 = t3 * t3 * dot(noise_grad3_table[gi3], make_vector4(x3, y3, z3, 0));
    }
    // Add contributions from each corner to get the final noise value.
    // The result is scaled to stay just inside [-1,1]
    return 32.0f*(n0 + n1 + n2 + n3);
}

inline float noise2D(Vector2 position)
{
    return simplex_noise2D(position.x, position.y);
}

inline float noise3D(Vector3 position)
{
    return simplex_noise3D(position.x, position.y, position.z);
}


#endif //_T3_NOISE_HPP


#include "vector.h"
#include <stdio.h>
#include <math.h>

/** Add v1 and v2
 */
Vector3 vector3_add(Vector3 v1, Vector3 v2)
{
  return (Vector3){v1.x+v2.x, v1.y+v2.y, v1.z+v2.z};
}

/** Subtract v2 from v1
 */
Vector3 vector3_sub(Vector3 v1, Vector3 v2)
{
  return (Vector3){v1.x-v2.x, v1.y-v2.y, v1.z-v2.z};
}

void matrix_mult(int h1, int w1, int h2, int w2, float m1m2[h1][w2], float m1[h1][w1], float m2[h2][w2])
{
  if (w1 != h2)
  {
    printf("MATRIX SIZE MISMATCH\n");
    return;
  }

  for (int i=0; i<h1; i++) {
    for (int j=0; j<w2; j++) {
      m1m2[i][j] = 0;
      for (int k=0; k<h2; k++) {
        m1m2[i][j] += m1[i][k] * m2[k][j];
      }
    }
  }
}

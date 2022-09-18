#include<math.h>

// this function gives the maximum
float maxi(float arr[],int n) {
  float m = 0;
  for (int i = 0; i < n; ++i)
    if (m < arr[i])
      m = arr[i];
  return m;
}

// this function gives the minimum
float mini(float arr[], int n) {
  float m = 1;
  for (int i = 0; i < n; ++i)
    if (m > arr[i])
      m = arr[i];
  return m;
}

void liang_barsky_clipper(float xmin, float ymin, float xmax, float ymax,
                          float x1, float y1, float x2, float y2) {
  // defining variables
  float p1 = -(x2 - x1);
  float p2 = -p1;
  float p3 = -(y2 - y1);
  float p4 = -p3;

  float q1 = x1 - xmin;
  float q2 = xmax - x1;
  float q3 = y1 - ymin;
  float q4 = ymax - y1;

  float posarr[5], negarr[5];
  int posind = 1, negind = 1;
  posarr[0] = 1;
  negarr[0] = 0;

  rectangle(xmin, ymin, xmax, ymax); // drawing the clipping window

  if ((p1 == 0 && q1 < 0) || (p2 == 0 && q2 < 0) || (p3 == 0 && q3 < 0) || (p4 == 0 && q4 < 0)) {
      outtextxy(80, 80, "Line is parallel to clipping window!");
      return;
  }
  if (p1 != 0) {
    float r1 = q1 / p1;
    float r2 = q2 / p2;
    if (p1 < 0) {
      negarr[negind++] = r1; // for negative p1, add it to negative array
      posarr[posind++] = r2; // and add p2 to positive array
    } else {
      negarr[negind++] = r2;
      posarr[posind++] = r1;
    }
  }
  if (p3 != 0) {
    float r3 = q3 / p3;
    float r4 = q4 / p4;
    if (p3 < 0) {
      negarr[negind++] = r3;
      posarr[posind++] = r4;
    } else {
      negarr[negind++] = r4;
      posarr[posind++] = r3;
    }
  }

  float xn1, yn1, xn2, yn2;
  float rn1, rn2;
  rn1 = maxi(negarr, negind); // maximum of negative array
  rn2 = mini(posarr, posind); // minimum of positive array

  if (rn1 > rn2)  { // reject
    outtextxy(80, 80, "Line is outside the clipping window!");
    return;
  }

  xn1 = x1 + p2 * rn1;
  yn1 = y1 + p4 * rn1; // computing new points

  xn2 = x1 + p2 * rn2;
  yn2 = y1 + p4 * rn2;


  line(xn1, yn1, xn2, yn2); // the drawing the new line

  setlinestyle(1, 1, 0);

  line(x1, y1, xn1, yn1);
  line(x2, y2, xn2, yn2);
}

int main() {
  float xmin, ymin, xmax, ymax;
  float x1, y1, x2, y2;

  liang_barsky_clipper(xmin, ymin, xmax, ymax, x1, y1, x2, y2);
  getch();
  closegraph();
}
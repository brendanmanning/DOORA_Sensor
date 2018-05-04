#include "SSStat.h";

SSStat::SSStat(double t) {
  double current[10];
  double historical[1440];

  threshold = t;
}

bool SSStat::isFire(double t) {

  if(c == 10)
    c = 0;

  // Only detect fire after 10 values
  if (c != 9) {
    
    current[c] = t;
    c++;
    return false;
  }

  current[c] = t;
  historical[h] = xi();

  c++;
  h++;

  return (this->t() >= threshold);;
}

/*
 *  ******************** STAT CALCULATIONS ********************
*/
double SSStat::t() {
  
  double top = xi() - mu();
  double bottom = (std_dev() / sqrt(c));

  return top / bottom;
}

double SSStat::xi() {

  double sum = 0;
  for (int i = 0; i < 10; i++) {
    sum += current[i];
  }

  return (sum / 10);
}

double SSStat::mu() {

  double sum = 0;
  for (int i = 0; i < h; i++) {
    sum += historical[i];
  }

  return (sum / (h));
}

double SSStat::std_dev() {

  // Top of SQRT
  double sum_squares = 0;
  
  for (int i = 0; i < c; i++) {
    sum_squares += pow(( xi() - current[i]),2);
  }

  // Bottom of SQRT
  double bottom_sqrt = 9.0;

  return sqrt( sum_squares / bottom_sqrt );
}

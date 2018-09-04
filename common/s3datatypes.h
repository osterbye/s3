#ifndef S3DATATYPES_H
#define S3DATATYPES_H

union floatBytes{
  float f;
  char b[sizeof(float)];
};

union doubleBytes{
  double d;
  char b[sizeof(double)];
};

#endif // S3DATATYPES_H

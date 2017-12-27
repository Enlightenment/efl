#ifndef SGINTERPOLATOR_H
#define SGINTERPOLATOR_H

#include "sgpoint.h"

class SGInterpolator
{
public:
    SGInterpolator() { /* caller must call Init later */ }

    SGInterpolator(float aX1, float aY1,
                    float aX2, float aY2)
    {
      init(aX1, aY1, aX2, aY2);
    }

    SGInterpolator(SGPointF pt1, SGPointF pt2)
    {
      init(pt1.x(), pt1.y(), pt2.x(), pt2.y());
    }

    void init(float aX1, float aY1,
              float aX2, float aY2);

    float value(float aX) const;

    void GetSplineDerivativeValues(float aX, float& aDX, float& aDY) const;
private:
    void
      CalcSampleValues();

      /**
       * Returns x(t) given t, x1, and x2, or y(t) given t, y1, and y2.
       */
      static float
      CalcBezier(float aT, float aA1, float aA2);

      /**
       * Returns dx/dt given t, x1, and x2, or dy/dt given t, y1, and y2.
       */
      static float
      GetSlope(float aT, float aA1, float aA2);

      float
      GetTForX(float aX) const;

      float
      NewtonRaphsonIterate(float aX, float aGuessT) const;

      float
      BinarySubdivide(float aX, float aA, float aB) const;

      static float
      A(float aA1, float aA2)
      {
        return 1.0 - 3.0 * aA2 + 3.0 * aA1;
      }

      static float
      B(float aA1, float aA2)
      {
        return 3.0 * aA2 - 6.0 * aA1;
      }

      static float
      C(float aA1)
      {
        return 3.0 * aA1;
      }

    float mX1;
    float mY1;
    float mX2;
    float mY2;
    enum { kSplineTableSize = 11 };
    float mSampleValues[kSplineTableSize];
    static const float kSampleStepSize;
};
#endif // SGINTERPOLATOR_H

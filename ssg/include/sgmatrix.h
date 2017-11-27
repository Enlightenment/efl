#ifndef SGMATRIX_H
#define SGMATRIX_H
#include"sgpoint.h"
#include "sgregion.h"
#include "sgglobal.h"

struct SGMatrixData;
class SG_EXPORT SGMatrix
{
public:
    enum class Axis {
        X,
        Y,
        Z
    };
    enum class MatrixType {
        None      = 0x00,
        Translate = 0x01,
        Scale     = 0x02,
        Rotate    = 0x04,
        Shear     = 0x08,
        Project   = 0x10
    };

    SGMatrix();
    ~SGMatrix();
    SGMatrix(const SGMatrix &matrix);
    SGMatrix(SGMatrix &&other);
    SGMatrix &operator=(const SGMatrix &);
    SGMatrix &operator=(SGMatrix &&other);

    bool isAffine() const;
    bool isIdentity() const;
    bool isInvertible() const;
    bool isScaling() const;
    bool isRotating() const;
    bool isTranslating() const;
    MatrixType type() const;
    inline float determinant() const;

    SGMatrix &translate(float dx, float dy);
    SGMatrix &scale(float sx, float sy);
    SGMatrix &shear(float sh, float sv);
    SGMatrix &rotate(float a, Axis axis);
    SGMatrix &rotateRadians(float a, Axis axis);

    SGPointF map(const SGPointF &p) const;
    inline SGPointF map(float x, float y) const;
    SGRect map(const SGRect &r) const;
    SGRegion map(const SGRegion &r) const;

    SG_REQUIRED_RESULT SGMatrix inverted(bool *invertible = nullptr) const;
    SG_REQUIRED_RESULT SGMatrix adjoint() const;

    SGMatrix operator*(const SGMatrix &o) const;
    SGMatrix &operator*=(const SGMatrix &);
    SGMatrix &operator*=(float mul);
    SGMatrix &operator/=(float div);
    bool operator==(const SGMatrix &) const;
    bool operator!=(const SGMatrix &) const;
    friend std::ostream& operator<<(std::ostream& os, const SGMatrix& o);
private:
    explicit SGMatrix(bool init);
    explicit SGMatrix(float m11, float m12, float m13,
                      float m21, float m22, float m23,
                      float m31, float m32, float m33);
    SGMatrix copy() const;
    void detach();
    void cleanUp(SGMatrixData *x);

    SGMatrixData *d;
};

inline SGPointF SGMatrix::map(float x, float y) const
{
    return map(SGPointF(x, y));
}

#endif // SGMATRIX_H

#ifndef EINA_F16P16_H_
#define EINA_F16P16_H_

/**
 * To be documented
 * FIXME: To be fixed
 */
typedef int32_t Eina_F16p16;

/**
 * To be documented
 * FIXME: To be fixed
 */
static inline Eina_F16p16 eina_f16p16_int_from(int32_t v)
{
	return v << 16;
}
/**
 * To be documented
 * FIXME: To be fixed
 */
static inline int32_t eina_f16p16_int_to(Eina_F16p16 v)
{
	return v >> 16;
}
/**
 * To be documented
 * FIXME: To be fixed
 */
static inline Eina_F16p16 eina_f16p16_float_from(float v)
{
	Eina_F16p16 r;
	r = (Eina_F16p16)(v * 65536.0 + (v < 0 ? -0.5 : 0.5));
	return r;
}
/**
 * To be documented
 * FIXME: To be fixed
 */
static inline float eina_f16p16_float_to(Eina_F16p16 v)
{
	float r;
	r = v / 65536.0;
	return r;
}
/**
 * To be documented
 * FIXME: To be fixed
 */
static inline Eina_F16p16 eina_f16p16_add(Eina_F16p16 a, Eina_F16p16 b)
{
	return a + b;
}
/**
 * To be documented
 * FIXME: To be fixed
 */
static inline Eina_F16p16 eina_f16p16_sub(Eina_F16p16 a, Eina_F16p16 b)
{
	return a - b;
}
/**
 * To be documented
 * FIXME: To be fixed
 */
static inline Eina_F16p16 eina_f16p16_mul(Eina_F16p16 a, Eina_F16p16 b)
{
	return (a * b) >> 16;
}
/**
 * To be documented
 * FIXME: To be fixed
 */
static inline Eina_F16p16 eina_f16p16_sqrt(Eina_F16p16 a)
{
	unsigned int root, remHi, remLo, testDiv, count;

	root = 0; /* Clear root */
	remHi = 0; /* Clear high part of partial remainder */
	remLo = a; /* Get argument into low part of partial remainder */
	count = (15 + (16 >> 1)); /* Load loop counter */
	do
	{
		remHi = (remHi << 2) | (remLo >> 30);
		remLo <<= 2; /* get 2 bits of arg */
		root <<= 1; /* Get ready for the next bit in the root */
		testDiv = (root << 1) + 1; /* Test radical */
		if (remHi >= testDiv)
		{
			remHi -= testDiv;
			root++;
		}
	} while (count-- != 0);
	return (root);
}
/**
 * To be documented
 * FIXME: To be fixed
 */
static inline unsigned int eina_f16p16_fracc_get(Eina_F16p16 v)
{
	return (v & 0xffff);
}

#endif /*EINA_F16P16_H_*/

/* Float arithmetic
 *
 * (c) Copyright 1999, Artran, Inc.
 * Written by Greg Garner (gmg@artran.com)
 * Modified in March 2001 to include user defined
 * operators for the floating point functions.
 * Modified March 2004 by Carsten Haitzler <raster@rasterman.com> to conform
 * to E coding style
 *
 * This file is provided as is (no warranties).
 */
#if defined FLOAT_INC
#endinput
#endif
#define FLOAT_INC

#pragma rational Float

/* Different methods of rounding */
enum Float_Round_Method
{
   ROUND, FLOOR, CEIL, TOZERO
};
/* different angle addressing modes (default is radians) */
enum Float_Angle_Mode
{
   RADIAN, DEGREES, GRADES
};

/* Convert a string into a floating point value */
native Float:atof(const string[]);
/* Return the fractional part of a float */
native Float:fract(Float:value);
/* Round a float into a integer value */
native       round(Float:value, Float_Round_Method:method=ROUND);
/* Return the square root of the input value, same as float_power(value, 0.5) */
native Float:sqrt(Float:value);
/* Return the value raised to the power of the exponent */
native Float:pow(Float:value, Float:exponent);
/* Return the logarithm */
native Float:log(Float:value, Float:base=10.0);
/* Return the sine, cosine or tangent. The input angle may be in radian, degrees or grades. */
native Float:sin(Float:value, Float_Angle_Mode:mode=RADIAN);
native Float:cos(Float:value, Float_Angle_Mode:mode=RADIAN);
native Float:tan(Float:value, Float_Angle_Mode:mode=RADIAN);
/* Return the absolute value */
native Float:abs(Float:value);








/**************************************************/
/* Hidden calls - all are overloaded on operators */
/**************************************************/

/* Convert an integer into a floating point value */
native Float:float(value);
/* Multiple two floats together */
native Float:float_mul(Float:oper1, Float:oper2);
/* Divide the dividend float by the divisor float */
native Float:float_div(Float:dividend, Float:divisor);
/* Add two floats together */
native Float:float_add(Float:oper1, Float:oper2);
/* Subtract oper2 float from oper1 float */
native Float:float_sub(Float:oper1, Float:oper2);
/* Compare two integers. If the two elements are equal, return 0. */
/* If the first argument is greater than the second argument, return 1, */
/* If the first argument is less than the second argument, return -1. */
native       float_cmp(Float:oper1, Float:oper2);
/* user defined operators */
native Float:operator*(Float:oper1, Float:oper2) = float_mul;
native Float:operator/(Float:oper1, Float:oper2) = float_div;
native Float:operator+(Float:oper1, Float:oper2) = float_add;
native Float:operator-(Float:oper1, Float:oper2) = float_sub;
native Float:operator=(oper) = float;
stock Float:operator++(Float:oper)
    return oper+1.0;
stock Float:operator--(Float:oper)
    return oper-1.0;
stock Float:operator-(Float:oper)
    return oper^Float:0x80000000; /* IEEE values are sign/magnitude */
stock Float:operator*(Float:oper1, oper2)
    return float_mul(oper1, float(oper2)); /* "*" is commutative */
stock Float:operator/(Float:oper1, oper2)
    return float_div(oper1, float(oper2));
stock Float:operator/(oper1, Float:oper2)
    return float_div(float(oper1), oper2);
stock Float:operator+(Float:oper1, oper2)
    return float_add(oper1, float(oper2)); /* "+" is commutative */
stock Float:operator-(Float:oper1, oper2)
    return float_sub(oper1, float(oper2));
stock Float:operator-(oper1, Float:oper2)
    return float_sub(float(oper1), oper2);
stock bool:operator==(Float:oper1, Float:oper2)
    return float_cmp(oper1, oper2) == 0;
stock bool:operator==(Float:oper1, oper2)
    return float_cmp(oper1, float(oper2)) == 0;  /* "==" is commutative */
stock bool:operator!=(Float:oper1, Float:oper2)
    return float_cmp(oper1, oper2) != 0;
stock bool:operator!=(Float:oper1, oper2)
    return float_cmp(oper1, float(oper2)) != 0;  /* "!=" is commutative */
stock bool:operator>(Float:oper1, Float:oper2)
    return float_cmp(oper1, oper2) > 0;
stock bool:operator>(Float:oper1, oper2)
    return float_cmp(oper1, float(oper2)) > 0;
stock bool:operator>(oper1, Float:oper2)
    return float_cmp(float(oper1), oper2) > 0;
stock bool:operator>=(Float:oper1, Float:oper2)
    return float_cmp(oper1, oper2) >= 0;
stock bool:operator>=(Float:oper1, oper2)
    return float_cmp(oper1, float(oper2)) >= 0;
stock bool:operator>=(oper1, Float:oper2)
    return float_cmp(float(oper1), oper2) >= 0;
stock bool:operator<(Float:oper1, Float:oper2)
    return float_cmp(oper1, oper2) < 0;
stock bool:operator<(Float:oper1, oper2)
    return float_cmp(oper1, float_(oper2)) < 0;
stock bool:operator<(oper1, Float:oper2)
    return float_cmp(float(oper1), oper2) < 0;
stock bool:operator<=(Float:oper1, Float:oper2)
    return float_cmp(oper1, oper2) <= 0;
stock bool:operator<=(Float:oper1, oper2)
    return float_cmp(oper1, float(oper2)) <= 0;
stock bool:operator<=(oper1, Float:oper2)
    return float_cmp(float(oper1), oper2) <= 0;
stock bool:operator!(Float:oper)
    return (_:oper & 0x7fffffff) == 0;
/* forbidden operations */
forward operator%(Float:oper1, Float:oper2);
forward operator%(Float:oper1, oper2);
forward operator%(oper1, Float:oper2);

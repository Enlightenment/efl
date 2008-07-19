/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifndef _EVAS_GRADIENT_PRIVATE_H
#define _EVAS_GRADIENT_PRIVATE_H


RGBA_Gradient_Type *evas_common_gradient_linear_get        (void);
RGBA_Gradient_Type *evas_common_gradient_radial_get        (void);
RGBA_Gradient_Type *evas_common_gradient_angular_get       (void);
RGBA_Gradient_Type *evas_common_gradient_rectangular_get   (void);
RGBA_Gradient_Type *evas_common_gradient_sinusoidal_get    (void);
char               *evas_common_gradient_get_key_fval      (char *in, char *key, float *val);


#endif /* _EVAS_GRADIENT_PRIVATE_H */

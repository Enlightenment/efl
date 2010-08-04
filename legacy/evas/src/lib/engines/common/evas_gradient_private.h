#ifndef _EVAS_GRADIENT_PRIVATE_H
#define _EVAS_GRADIENT_PRIVATE_H


RGBA_Gradient_Type *evas_common_gradient_linear_get        (void);
RGBA_Gradient_Type *evas_common_gradient_radial_get        (void);
RGBA_Gradient_Type *evas_common_gradient_angular_get       (void);
RGBA_Gradient_Type *evas_common_gradient_rectangular_get   (void);
RGBA_Gradient_Type *evas_common_gradient_sinusoidal_get    (void);
char               *evas_common_gradient_get_key_fval      (char *in, char *key, float *val);


#endif /* _EVAS_GRADIENT_PRIVATE_H */

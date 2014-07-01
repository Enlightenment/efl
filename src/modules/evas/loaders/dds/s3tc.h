#ifndef EFL_S3TC_H
#define EFL_S3TC_H

void s3tc_decode_dxt1_rgb(unsigned int *bgra, const unsigned char *s3tc);
void s3tc_decode_dxt1_rgba(unsigned int *bgra, const unsigned char *s3tc);
void s3tc_decode_dxt2_rgba(unsigned int *bgra, const unsigned char *s3tc);
void s3tc_decode_dxt3_rgba(unsigned int *bgra, const unsigned char *s3tc);
void s3tc_decode_dxt4_rgba(unsigned int *bgra, const unsigned char *s3tc);
void s3tc_decode_dxt5_rgba(unsigned int *bgra, const unsigned char *s3tc);

void s3tc_encode_dxt1_flip(unsigned char *dest, const unsigned char *orig, int vflip);
void s3tc_encode_dxt2_rgba_flip(unsigned char *dest, const unsigned char *orig, int vflip);
void s3tc_encode_dxt3_rgba_flip(unsigned char *dest, const unsigned char *orig, int vflip);
void s3tc_encode_dxt4_rgba_flip(unsigned char *dest, const unsigned char *orig, int vflip);
void s3tc_encode_dxt5_rgba_flip(unsigned char *dest, const unsigned char *orig, int vflip);

#endif // EFL_S3TC_H

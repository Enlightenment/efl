#ifndef EFL_S3TC_H
#define EFL_S3TC_H

void s3tc_decode_dxt1_rgb(unsigned int *bgra, const unsigned char *s3tc);
void s3tc_decode_dxt1_rgba(unsigned int *bgra, const unsigned char *s3tc);
void s3tc_decode_dxt2_rgba(unsigned int *bgra, const unsigned char *s3tc);
void s3tc_decode_dxt3_rgba(unsigned int *bgra, const unsigned char *s3tc);
void s3tc_decode_dxt4_rgba(unsigned int *bgra, const unsigned char *s3tc);
void s3tc_decode_dxt5_rgba(unsigned int *bgra, const unsigned char *s3tc);

#endif // EFL_S3TC_H

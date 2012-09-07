#ifndef INTERFACE2_H
#define INTERFACE2_H

extern EAPI Eo_Op INTERFACE2_BASE_ID;

enum {
     INTERFACE2_SUB_ID_AB_SUM_GET2,
     INTERFACE2_SUB_ID_LAST
};

#define INTERFACE2_ID(sub_id) (INTERFACE2_BASE_ID + sub_id)


/**
 * @def interface2_ab_sum_get2(sum)
 * @brief Get sum of a,b integer elements
 * @param[out] sum integer pointer to sum - value
 */
#define interface2_ab_sum_get2(sum) INTERFACE2_ID(INTERFACE2_SUB_ID_AB_SUM_GET2), EO_TYPECHECK(int *, sum)

#define INTERFACE2_CLASS interface2_class_get()
const Eo_Class *interface2_class_get(void);

#endif

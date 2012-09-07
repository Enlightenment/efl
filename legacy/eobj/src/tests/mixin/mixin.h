#ifndef MIXIN_H
#define MIXIN_H

extern EAPI Eo_Op MIXIN_BASE_ID;

enum {
     MIXIN_SUB_ID_AB_SUM_GET,
     MIXIN_SUB_ID_LAST
};

#define MIXIN_ID(sub_id) (MIXIN_BASE_ID + sub_id)


/**
 * @def mixin_ab_sum_get(sum)
 * @brief Get sum of a,b integer elements
 * @param[out] sum integer pointer to sum - value
 */
#define mixin_ab_sum_get(sum) MIXIN_ID(MIXIN_SUB_ID_AB_SUM_GET), EO_TYPECHECK(int *, sum)

#define MIXIN_CLASS mixin_class_get()
const Eo_Class *mixin_class_get(void);

#endif

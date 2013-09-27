#ifndef INTERFACE_H
#define INTERFACE_H

extern EAPI Eo_Op INTERFACE_BASE_ID;

enum {
     INTERFACE_SUB_ID_AB_SUM_GET,
     INTERFACE_SUB_ID_LAST
};

#define INTERFACE_ID(sub_id) (INTERFACE_BASE_ID + sub_id)


/**
 * @def interface_ab_sum_get(sum)
 * @brief Get sum of a,b integer elements
 * @param[out] sum integer pointer to sum - value
 */
#define interface_ab_sum_get(sum) INTERFACE_ID(INTERFACE_SUB_ID_AB_SUM_GET), EO_TYPECHECK(int *, sum)

#define INTERFACE_CLASS interface_class_get()
const Eo_Class *interface_class_get(void);

#endif

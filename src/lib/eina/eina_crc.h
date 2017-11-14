#ifndef EINA_CRC_H_
#define EINA_CRC_H_

#include <stdlib.h>

#include "eina_config.h"

#include "eina_types.h"
#include "eina_error.h"
#include "eina_iterator.h"
#include "eina_accessor.h"
#include "eina_magic.h"

/**
 * @brief Generates a checksum for the given data using the CRC-32 algorithm.
 *
 * @param[in] key The data to checksum.
 * @param[in] len The length of @p key in bytes.
 * @param[in] seed The seed used to calculate the checksum.
 * @param[in] start_stream Set as #EINA_TRUE for the initial call and
 *            #EINA_FALSE for the remaining stream data.
 * @return A 32-bit CRC checksum for the data given in @p key.
 *
 * This function generates a checksum using the CRC-32 algorithm
 * with the @c 0xEDB88320 polynomial on the data provided by @p key.
 * (https://en.wikipedia.org/wiki/Cyclic_redundancy_check#CRC-32_algorithm)
 *
 * For streaming data eina_crc() can be called repeatedly, passing the
 * returned checksum from the prior call as the @p seed to the next, and
 * setting @p start_stream to #EINA_FALSE.  For the initial call, set
 * @p seed to @c 0xffffffff and @p start_stream to #EINA_TRUE.
 *
 * For normal (non-streaming) data, seed tracking is not needed, so
 * @p seed should be set to @c 0xffffffff and @p start_stream to
 * #EINA_TRUE.
 *
 * Example usage for normal data:
 *
 * @code
 * char *tmp = "hello world";
 * unsigned int crc = eina_crc(tmp, strlen(tmp), 0xffffffff, EINA_TRUE);
 * @endcode
 *
 * Example usage for streaming data:
 *
 * @code
 * char *tmp1 = "hello "
 * char *tmp2 = "world"
 * unsigned int crc = eina_crc(tmp1, strlen(tmp1), 0xffffffff, EINA_TRUE);
 * crc = eina_crc(tmp2, strlen(tmp2), crc, EINA_FALSE);
 * @endcode
 *
 * @since 1.15
 */
static inline unsigned int eina_crc(const char *key, int len, unsigned int seed, Eina_Bool start_stream) EINA_ARG_NONNULL(2, 3);

#include "eina_inline_crc.x"

#endif

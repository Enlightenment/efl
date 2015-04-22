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
 * @brief Generate a crc checksum for the given data using crc-32 algorithm.
 *
 * @param key The data for which crc checksum has to be generated.
 * @param len The length of the data.
 * @param seed The seed used to calculate crc checksum. The value will be 0xffffffff
 * for the initial data and for the remaining stream data, it will be crc value from the
 * previous call to the eina_crc function.
 * @param start_stream EINA_TRUE for the initial call and EINA_FALSE for the remaining stream data.
 * @return 32 bit crc checksum for the given data.
 *
 * This function generates 32 bit crc checksum using crc-32 algorithm and 0xEDB88320 polynomial
 * for the data which is passed to the function. Seed has to be passed which will
 * be used in calculating checksum. For normal data 0xffffffff has to be passed as seed along with
 * EINA_TRUE for start_stream parameter. For streaming data, initial call will be similar to normal
 * data and for the rest of data, seed will be the crc returned from previous call to eina_crc
 * and start_stream parameter will be EINA_FALSE
 * API usage for normal data
 * char *tmp = "hello world";
 * unsigned int crc = eina_crc(tmp, strlen(tmp), 0xffffffff, EINA_TRUE);
 *
 * API usage for streaming data
 * char *tmp1 = "hello "
 * char *tmp2 = "world"
 * unsigned int crc = eina_crc(tmp1, strlen(tmp1), 0xffffffff, EINA_TRUE);
 * crc = eina_crc(tmp2, strlen(tmp2), crc, EINA_FALSE);
 */
EAPI unsigned int eina_crc(const char *key, int len, unsigned int seed, Eina_Bool start_stream) EINA_ARG_NONNULL(2, 3);

#endif

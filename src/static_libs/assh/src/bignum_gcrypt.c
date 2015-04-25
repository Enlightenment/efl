/*

  libassh - asynchronous ssh2 client/server library.

  Copyright (C) 2013 Alexandre Becoulet <alexandre.becoulet@free.fr>

  This library is free software; you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation; either version 2.1 of the
  License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301 USA

*/

#include <assh/assh_bignum.h>
#include <assh/assh_context.h>
#include <assh/assh_packet.h>
#include <assh/assh_prng.h>
#include <assh/assh_alloc.h>

#include <gcrypt.h>

static ASSH_WARN_UNUSED_RESULT assh_error_t
assh_gcrypt_bignum_rand(struct assh_context_s *c,
                        struct assh_bignum_s *bn,
                        const struct assh_bignum_s *min,
                        const struct assh_bignum_s *max,
                        enum assh_prng_quality_e quality)
{
  assh_error_t err;

  ASSH_CHK_RET(c->prng == NULL, ASSH_ERR_MISSING_ALGO);

  size_t bits = bn->bits;

  if (max != NULL)
    bits = ASSH_MIN(bits, gcry_mpi_get_nbits(max->n));

  size_t n = ASSH_ALIGN8(bn->bits);

  if (c->prng == &assh_prng_gcrypt)
    {
      enum gcry_random_level level;
      switch (quality)
	{
	case ASSH_PRNG_QUALITY_WEAK:
	  level = GCRY_WEAK_RANDOM;
	  break;
	case ASSH_PRNG_QUALITY_NONCE:
	case ASSH_PRNG_QUALITY_EPHEMERAL_KEY:
	  level = GCRY_STRONG_RANDOM;
	  break;
	case ASSH_PRNG_QUALITY_LONGTERM_KEY:
	default:
	  level = GCRY_VERY_STRONG_RANDOM;
	  break;
	}

      if (bn->n == NULL)
        bn->n = gcry_mpi_snew(n);

      gcry_mpi_randomize(bn->n, n, level);
      gcry_mpi_rshift(bn->n, bn->n, n - bits);

      gcry_mpi_t t = NULL;

      while ((min != NULL && gcry_mpi_cmp(bn->n, min->n) <= 0) ||
             (max != NULL && gcry_mpi_cmp(bn->n, max->n) >= 0))
        {
          if (t == NULL)
            t = gcry_mpi_snew(8);

          gcry_mpi_rshift(bn->n, bn->n, 8);
          gcry_mpi_randomize(t, 8, level);
          if (bits >= 8)
            gcry_mpi_lshift(t, t, bits - 8);
          else
            gcry_mpi_rshift(t, t, 8 - bits);
          gcry_mpi_add(bn->n, bn->n, t);
        }

      gcry_mpi_release(t);

      err = ASSH_OK;
    }
  else
    {
      ASSH_SCRATCH_ALLOC(c, uint8_t, rnd, n / 8, ASSH_ERRSV_CONTINUE, err_);

      ASSH_ERR_GTO(c->prng->f_get(c, rnd, n / 8, quality), err_sc);

      while (1)
        {
          gcry_mpi_release(bn->n);
          bn->n = NULL;
          ASSH_CHK_GTO(gcry_mpi_scan((gcry_mpi_t*)&bn->n,
                 GCRYMPI_FMT_USG, rnd, n / 8, NULL), ASSH_ERR_CRYPTO, err_sc);
          gcry_mpi_rshift(bn->n, bn->n, n - bits);

          if ((min == NULL || gcry_mpi_cmp(bn->n, min->n) > 0) &&
              (max == NULL || gcry_mpi_cmp(bn->n, max->n) < 0))
            break;

          memmove(rnd + 1, rnd, n / 8 - 1);
          ASSH_ERR_GTO(c->prng->f_get(c, rnd, 1, quality), err_sc);
        }

      err = ASSH_OK;
    err_sc:
      ASSH_SCRATCH_FREE(c, rnd);
    err_:;
    }

  return err;
}

static void assh_bignum_gcrypt_lsb(uint8_t *data, size_t size)
{
  size_t i;
  for (i = 0; i < size / 2; i++)
    ASSH_SWAP(data[i], data[size - i - 1]);
}

static ASSH_BIGNUM_CONVERT_FCN(assh_bignum_gcrypt_convert)
{
  assh_error_t err;

  const struct assh_bignum_s *srcn = src;
  struct assh_bignum_s *dstn = dst;

  if (srcfmt == ASSH_BIGNUM_NATIVE ||
      srcfmt == ASSH_BIGNUM_STEMP ||
      srcfmt == ASSH_BIGNUM_TEMP)
    {
      size_t s = ASSH_ALIGN8(srcn->bits) / 8;
      size_t z = s;

      switch (dstfmt)
        {
        case ASSH_BIGNUM_NATIVE:
        case ASSH_BIGNUM_STEMP:
        case ASSH_BIGNUM_TEMP:
          ASSH_CHK_RET(dstn->bits < gcry_mpi_get_nbits(srcn->n), ASSH_ERR_NUM_OVERFLOW);
          gcry_mpi_release(dstn->n);
          dstn->n = gcry_mpi_snew(dstn->bits);
          ASSH_CHK_RET(dstn->n == NULL, ASSH_ERR_MEM);
          dstn->n = gcry_mpi_set(dstn->n, srcn->n);
          break;
        case ASSH_BIGNUM_STRING:
          assh_store_u32(dst, s);
          dst += 4;
          ASSH_CHK_RET(gcry_mpi_print(GCRYMPI_FMT_USG, dst, s, &z, srcn->n),
                       ASSH_ERR_NUM_OVERFLOW);
          break;
        case ASSH_BIGNUM_MPINT:
          ASSH_CHK_RET(gcry_mpi_print(GCRYMPI_FMT_SSH, dst, s + 5, NULL, srcn->n),
                       ASSH_ERR_NUM_OVERFLOW);
          break;
        case ASSH_BIGNUM_LSB_RAW:
        case ASSH_BIGNUM_MSB_RAW:
          ASSH_CHK_RET(gcry_mpi_print(GCRYMPI_FMT_USG, dst, s, &z, srcn->n),
                       ASSH_ERR_NUM_OVERFLOW);
          break;
        default:
          ASSH_ERR_RET(ASSH_ERR_NOTSUP);
        }

      /* shift and zero pad */
      if (z < s)
        {
          size_t d = s - z;
          memmove(dst + d, dst, z);
          memset(dst, 0, d);
        }

      /* reverse byte order */
      if (dstfmt == ASSH_BIGNUM_LSB_RAW)
        assh_bignum_gcrypt_lsb(dst, s);
    }
  else
    {
      assert(dstfmt == ASSH_BIGNUM_NATIVE ||
             dstfmt == ASSH_BIGNUM_STEMP ||
             dstfmt == ASSH_BIGNUM_TEMP);
      size_t s, n, b;

      if (srcfmt == ASSH_BIGNUM_MSB_RAW ||
          srcfmt == ASSH_BIGNUM_LSB_RAW)
        {
          b = dstn->bits;
          n = s = ASSH_ALIGN8(b) / 8;
        }
      else
        {
          ASSH_ERR_RET(assh_bignum_size_of_data(srcfmt, src, &s, &n, &b));
          ASSH_CHK_RET(dstn->bits < b, ASSH_ERR_NUM_OVERFLOW);
        }

      gcry_mpi_release(dstn->n);
      dstn->n = NULL;

      switch (srcfmt)
        {
        case ASSH_BIGNUM_STRING:
        case ASSH_BIGNUM_MPINT: {
          const uint8_t *mpint = src;
          ASSH_CHK_RET(gcry_mpi_scan((gcry_mpi_t*)&dstn->n, GCRYMPI_FMT_USG,
                                     mpint + 4, s - 4, NULL),
                       ASSH_ERR_NUM_OVERFLOW);
          break;
        }

        case ASSH_BIGNUM_ASN1: {
          const uint8_t *asn1 = src;
          ASSH_CHK_RET(gcry_mpi_scan((gcry_mpi_t*)&dstn->n, GCRYMPI_FMT_USG,
                                     asn1 + s - n, n, NULL),
                       ASSH_ERR_NUM_OVERFLOW);
          break;
        }

        case ASSH_BIGNUM_LSB_RAW: {
          ASSH_SCRATCH_ALLOC(c, uint8_t, lsb, s,
                             ASSH_ERRSV_CONTINUE, err_lsb);
          memcpy(lsb, src, s);
          assh_bignum_gcrypt_lsb(lsb, s);
          ASSH_CHK_GTO(gcry_mpi_scan((gcry_mpi_t*)&dstn->n, GCRYMPI_FMT_USG,
                                     lsb, s, NULL),
                       ASSH_ERR_NUM_OVERFLOW, err_lsb_scan);          
          gcry_mpi_clear_highbit((gcry_mpi_t)dstn->n, b);

          ASSH_SCRATCH_FREE(c, lsb);
          break;

         err_lsb_scan:
          ASSH_SCRATCH_FREE(c, lsb);
         err_lsb:
          return err;
        }

        case ASSH_BIGNUM_MSB_RAW: {
          ASSH_CHK_RET(gcry_mpi_scan((gcry_mpi_t*)&dstn->n, GCRYMPI_FMT_USG,
                                     src, s, NULL),
                       ASSH_ERR_NUM_OVERFLOW);
          gcry_mpi_clear_highbit((gcry_mpi_t)dstn->n, b);
          break;
        }

        case ASSH_BIGNUM_HEX: {
          ASSH_CHK_RET(gcry_mpi_scan((gcry_mpi_t*)&dstn->n, GCRYMPI_FMT_HEX,
                                     src, 0, NULL),
                       ASSH_ERR_NUM_OVERFLOW);
          break;
        }

        case ASSH_BIGNUM_INT: {
          ASSH_CHK_RET(dstn->bits < sizeof(intptr_t) * 8, ASSH_ERR_NUM_OVERFLOW);
          dstn->n = gcry_mpi_set_ui(dstn->n, (uintptr_t)src);
          break;
        }

        case ASSH_BIGNUM_SIZE: {
          if (b > 0)
            dstn->n = gcry_mpi_snew(b);
          dstn->bits = b;
          break;
        }

        default:
          ASSH_ERR_RET(ASSH_ERR_NOTSUP);
        }

      ASSH_CHK_RET(dstn->n == NULL, ASSH_ERR_MEM);
      ASSH_CHK_RET(gcry_mpi_is_neg(dstn->n), ASSH_ERR_NUM_OVERFLOW);
      ASSH_CHK_RET(gcry_mpi_get_nbits(dstn->n) > dstn->bits, ASSH_ERR_NUM_OVERFLOW);
    }

  return ASSH_OK;
}

struct assh_gcrypt_prime_s
{
  gcry_mpi_t min;
  gcry_mpi_t max;
};

static int assh_gcrypt_prime_chk(void *arg, int mode,
                                 gcry_mpi_t candidate)
{
  struct assh_gcrypt_prime_s *p = arg;

  if (p->min != NULL && gcry_mpi_cmp(candidate, p->min) <= 0)
    return 0;
  if (p->max != NULL && gcry_mpi_cmp(candidate, p->max) >= 0)
    return 0;
  return 1;
}

static ASSH_BIGNUM_BYTECODE_FCN(assh_bignum_gcrypt_bytecode)
{
  uint_fast8_t flen, tlen = 0;
  assh_error_t err;
  uint_fast8_t i, j, pc = 0;

  /* find number of arguments and temporaries */
  for (tlen = flen = 0; format[flen]; flen++)
    if (format[flen] == 'T' ||
        format[flen] == 'X' ||
        format[flen] == 'm')
      tlen++;

  void *args[flen];
  struct assh_bignum_s tmp[tlen];
  memset(tmp, 0, sizeof(tmp));

  for (j = i = 0; i < flen; i++)
    switch (format[i])
      {
      case ASSH_BIGNUM_STEMP:
        tmp[j].secret = 1;
      case ASSH_BIGNUM_TEMP:
      case ASSH_BIGNUM_MT:
        args[i] = &tmp[j];
        j++;
        break;
      case ASSH_BIGNUM_SIZE:
        args[i] = (void*)va_arg(ap, size_t);
        break;
      default:
        args[i] = va_arg(ap, void *);
      }

  while (1)
    {
      uint32_t opc = ops[pc];
      enum assh_bignum_opcode_e op = opc >> 26;
      uint_fast8_t oa = (opc >> 20) & 0x3f;
      uint_fast8_t ob = (opc >> 14) & 0x3f;
      uint_fast8_t oc = (opc >> 6) & 0xff;
      uint_fast8_t od = opc & 0x3f;
      uint_fast32_t value = (opc >> 6) & 0xfffff;

#if defined(CONFIG_ASSH_DEBUG) && 1
      const char *opnames[] = ASSH_BIGNUM_OP_NAMES;
      ASSH_DEBUG("exec=%p, pc=%u, op=%s, a=%u, b=%u, c=%u, d=%u, value=%u\n",
                 ops, pc, opnames[op], oa, ob, oc, od, value);
#if 1
      {
        size_t i;
        for (i = 0; i < flen; i++)
          if (format[i] == ASSH_BIGNUM_NATIVE ||
              format[i] == ASSH_BIGNUM_TEMP ||
              format[i] == ASSH_BIGNUM_STEMP)
            {
              struct assh_bignum_s *src = args[i];
              ASSH_DEBUG("%u: 0x", i);
              if (src->n)
                gcry_mpi_dump(src->n);
              fprintf(stderr, "\n");
            }
      }
#endif
#endif

      pc++;
      switch (op)
        {
        case ASSH_BIGNUM_OP_END:
          goto end;

        case ASSH_BIGNUM_OP_MOVE:
          ASSH_ERR_GTO(assh_bignum_gcrypt_convert(c, format[od], format[oc],
                                             args[od], args[oc]), err_sc);
          break;

        case ASSH_BIGNUM_OP_SIZER:
        case ASSH_BIGNUM_OP_SIZE: {
          size_t b, i;
          ASSH_ERR_GTO(assh_bignum_size_of_data(format[ob], args[ob],
                                                NULL, NULL, &b), err_sc);
          if (op == ASSH_BIGNUM_OP_SIZE)
            {
              struct assh_bignum_s *dst = args[oa];
              dst->bits = ((od >= 32) ? (b << (od - 32))
                           : (b >> (32 - od))) + (intptr_t)(int8_t)oc;
            }
          else
            for (i = oa; i <= oc; i++) 
              {
                struct assh_bignum_s *dst = args[i];
                dst->bits = b;
              }
          break;
        }

        case ASSH_BIGNUM_OP_MTINIT: {
          struct assh_bignum_s *src = args[od];
          struct assh_bignum_s *dst = args[oc];
          gcry_mpi_release(dst->n);
          dst->n = gcry_mpi_copy(src->n);
          dst->bits = src->bits;
          break;
        }

        case ASSH_BIGNUM_OP_MTFROM:
        case ASSH_BIGNUM_OP_MTTO: {
          uint_fast8_t i;
          for (i = 0; i < oa; i++)
            {
              if (oc == ob)
                continue;
              struct assh_bignum_s *src = args[oc + i];
              struct assh_bignum_s *dst = args[ob + i];
              gcry_mpi_release(dst->n);
              dst->n = gcry_mpi_copy(src->n);
            }
          break;
        }

        case ASSH_BIGNUM_OP_EXPM:
          assert(format[od] == ASSH_BIGNUM_MT);
        case ASSH_BIGNUM_OP_ADD:
        case ASSH_BIGNUM_OP_SUB:
        case ASSH_BIGNUM_OP_MUL: {
          struct assh_bignum_s *dst = args[oa];
          if (dst->n == NULL)
            {
              dst->n = gcry_mpi_snew(dst->bits);
              ASSH_CHK_GTO(dst->n == NULL, ASSH_ERR_MEM, err_sc);
            }

          struct assh_bignum_s *src1 = args[ob];
          struct assh_bignum_s *src2 = args[oc];
          if (od == ASSH_BOP_NOREG)
            {
              assert(dst->bits >= src1->bits &&
                     dst->bits >= src2->bits);
              switch (op)
                {
                case ASSH_BIGNUM_OP_ADD:
                  gcry_mpi_add(dst->n, src1->n, src2->n);
                  ASSH_CHK_GTO(gcry_mpi_get_nbits(dst->n) > dst->bits,
                               ASSH_ERR_NUM_OVERFLOW, err_sc);
                  break;
                case ASSH_BIGNUM_OP_SUB:
                  gcry_mpi_sub(dst->n, src1->n, src2->n);
                  ASSH_CHK_GTO(gcry_mpi_get_nbits(dst->n) > dst->bits,
                               ASSH_ERR_NUM_OVERFLOW, err_sc);
                  ASSH_CHK_GTO(gcry_mpi_is_neg(dst->n),
                               ASSH_ERR_NUM_OVERFLOW, err_sc);
                  break;
                case ASSH_BIGNUM_OP_MUL:
                  assert(dst->bits >= src1->bits + src2->bits);
                  gcry_mpi_mul(dst->n, src1->n, src2->n);
                  break;
                default:
                  abort();
                }
            }
          else
            {
              struct assh_bignum_s *mod = args[od];
              switch (op)
                {
                case ASSH_BIGNUM_OP_ADD:
                  gcry_mpi_addm(dst->n, src1->n, src2->n, mod->n);
                  break;
                case ASSH_BIGNUM_OP_SUB:
                  gcry_mpi_subm(dst->n, src1->n, src2->n, mod->n);
                  ASSH_CHK_GTO(gcry_mpi_is_neg(dst->n),
                               ASSH_ERR_NUM_OVERFLOW, err_sc);
                  break;
                case ASSH_BIGNUM_OP_MUL:
                  gcry_mpi_mulm(dst->n, src1->n, src2->n, mod->n);
                  break;
                case ASSH_BIGNUM_OP_EXPM:
                  gcry_mpi_powm(dst->n, src1->n, src2->n, mod->n);
                  break;
                default:
                  abort();
                }
            }
          break;
        }

        case ASSH_BIGNUM_OP_DIV: {
          gcry_mpi_t q = NULL, r = NULL;
          if (oa != ASSH_BOP_NOREG)
            {
              struct assh_bignum_s *dst = args[oa];
              if (dst->n == NULL)
                {
                  dst->n = gcry_mpi_snew(dst->bits);
                  ASSH_CHK_GTO(dst->n == NULL, ASSH_ERR_MEM, err_sc);
                }
              q = dst->n;
            }
          if (ob != ASSH_BOP_NOREG)
            {
              struct assh_bignum_s *dst = args[ob];
              if (dst->n == NULL)
                {
                  dst->n = gcry_mpi_snew(dst->bits);
                  ASSH_CHK_GTO(dst->n == NULL, ASSH_ERR_MEM, err_sc);
                }
              r = dst->n;
            }
          struct assh_bignum_s *src1 = args[oc];
          struct assh_bignum_s *src2 = args[od];
          ASSH_CHK_RET(!gcry_mpi_cmp_ui(src2->n, 0), ASSH_ERR_NUM_OVERFLOW);
          gcry_mpi_div(q, r, src1->n, src2->n, 0);
          break;
        }

        case ASSH_BIGNUM_OP_INV:
        case ASSH_BIGNUM_OP_GCD: {
          struct assh_bignum_s *dst = args[ob];
          struct assh_bignum_s *src1 = args[oc];
          struct assh_bignum_s *src2 = args[od];
          if (dst->n == NULL)
            {
              dst->n = gcry_mpi_snew(dst->bits);
              ASSH_CHK_GTO(dst->n == NULL, ASSH_ERR_MEM, err_sc);
            }
          switch (op)
            {
            case ASSH_BIGNUM_OP_GCD:
              gcry_mpi_gcd(dst->n, src1->n, src2->n);
              break;
            case ASSH_BIGNUM_OP_INV:
              gcry_mpi_invm(dst->n, src1->n, src2->n);
              break;
            default:
              abort();
            }
          break;
        }

        case ASSH_BIGNUM_OP_SHR:
        case ASSH_BIGNUM_OP_SHL: {
          struct assh_bignum_s *dst = args[oa];
          struct assh_bignum_s *src = args[ob];
          size_t b = 0;
          ASSH_CHK_RET(dst->bits != src->bits, ASSH_ERR_OUTPUT_OVERFLOW);
          if (od != ASSH_BOP_NOREG)
            {
              ASSH_ERR_GTO(assh_bignum_size_of_data(format[od], args[od],
                                                    NULL, NULL, &b), err_sc);
            }
          if (dst->n == NULL)
            {
              dst->n = gcry_mpi_snew(dst->bits);
              ASSH_CHK_GTO(dst->n == NULL, ASSH_ERR_MEM, err_sc);
            }
          switch (op)
            {
            case ASSH_BIGNUM_OP_SHR:
              gcry_mpi_rshift(dst->n, src->n, b + oc - 128);
              break;
            case ASSH_BIGNUM_OP_SHL:
              gcry_mpi_lshift(dst->n, src->n, b + oc - 128);
	      gcry_mpi_clear_highbit(dst->n, dst->bits);
              break;
            default:
              abort();
            }
          break;
        }

        case ASSH_BIGNUM_OP_RAND: {
          ASSH_ERR_GTO(assh_gcrypt_bignum_rand(c, args[oa],
                         ob == ASSH_BOP_NOREG ? NULL : args[ob],
                         oc == ASSH_BOP_NOREG ? NULL : args[oc],
                         od), err_sc);
          break;
        }

        case ASSH_BIGNUM_OP_CMP: {
          int r = 0;
          struct assh_bignum_s *src1 = args[oa];
          struct assh_bignum_s *src2 = args[ob];
          if (ob == ASSH_BOP_NOREG)
            r = src1->n != NULL;
          else if (ob != oa)
            r = gcry_mpi_cmp(src1->n, src2->n);
          switch (od)
            {
            case 0:             /* cmpeq */
              r = r != 0;
              break;
            case 1:             /* cmpne */
              r = r == 0;
              break;
            case 2:             /* cmplt */
              r = r >= 0;
              break;
            case 3:             /* cmplteq */
              r = r > 0;
              break;
            }
          if (r)
            ASSH_CHK_GTO(oc == 128, ASSH_ERR_NUM_COMPARE_FAILED, err_sc);
          else
            pc += oc - 128;
          break;
        }

        case ASSH_BIGNUM_OP_TESTS:
        case ASSH_BIGNUM_OP_TESTC: {
          struct assh_bignum_s *src1 = args[oa];
          size_t b = ob;
          if (od != ASSH_BOP_NOREG)
            {
              ASSH_ERR_GTO(assh_bignum_size_of_data(format[od], args[od],
                                                    NULL, NULL, &b), err_sc);
              b -= ob;
            }
          if (!gcry_mpi_test_bit(src1->n, b) ^ (op == ASSH_BIGNUM_OP_TESTC))
            ASSH_CHK_GTO(oc == 128, ASSH_ERR_NUM_COMPARE_FAILED, err_sc);
          else
            pc += oc - 128;
          break;
        }

        case ASSH_BIGNUM_OP_UINT: {
          struct assh_bignum_s *dst = args[od];
          dst->n = gcry_mpi_set_ui(dst->n, value);
          ASSH_CHK_GTO(dst->n == NULL, ASSH_ERR_MEM, err_sc);
          break;
        }

        case ASSH_BIGNUM_OP_LADJMP: {
          if (assh_bignum_lad(args[od]))
            pc += oc - 128;
          break;
        }

        case ASSH_BIGNUM_OP_LADSWAP: {
          struct assh_bignum_s *src1 = args[ob];
          struct assh_bignum_s *src2 = args[oc];

          if (assh_bignum_lad(args[od]))
            gcry_mpi_swap(src1->n, src2->n);
          break;
        }

        case ASSH_BIGNUM_OP_LADLOOP: {
          struct assh_bignum_lad_s *lad = args[od];
          uint16_t bit = --lad->count;
          if (bit)
            {
              if (lad->msbyte_1st && (bit & 7) == 0)
                lad->data++;
              pc -= oc;
            }
          break;
        }

        case ASSH_BIGNUM_OP_PRIME: {
          struct assh_bignum_s *dst = args[ob];
          struct assh_gcrypt_prime_s pchk = { NULL, NULL };
          size_t bits = dst->bits;
          if (oc != ASSH_BOP_NOREG)
            {
              struct assh_bignum_s *min = args[oc];
              pchk.min = min->n;
              assert(min->bits == dst->bits);
            }
          if (od != ASSH_BOP_NOREG)
            {
              struct assh_bignum_s *max = args[od];
              pchk.max = max->n;
              assert(max->bits == dst->bits);
              bits = gcry_mpi_get_nbits(pchk.max);
            }
          if (dst->n)
            gcry_mpi_release(dst->n);
 
         /* FIXME call gcry_random_add_bytes here */
          ASSH_CHK_GTO(gcry_prime_generate((struct gcry_mpi **)&dst->n,
                         bits, 0, NULL, assh_gcrypt_prime_chk,
                         &pchk, GCRY_STRONG_RANDOM, 0),
                       ASSH_ERR_CRYPTO, err_sc);
          break;
        }

        case ASSH_BIGNUM_OP_ISPRIM: {
          struct assh_bignum_s *src = args[od];
          if (ob ^ (gcry_mpi_cmp_ui(src->n, 2) <= 0 ||
                    gcry_prime_check(src->n, 0)))
            pc += oc - 128;
          else
            ASSH_CHK_GTO(oc == 128, ASSH_ERR_NUM_OVERFLOW, err_sc);
          break;
        }

        case ASSH_BIGNUM_OP_PRIVACY: {
          struct assh_bignum_s *src = args[od];
          src->secret = oc;
          break;
        }

        case ASSH_BIGNUM_OP_PRINT: {
#ifdef CONFIG_ASSH_DEBUG
          struct assh_bignum_s *src = args[od];
          char id[5];
          id[4] = 0;
          assh_store_u32le((uint8_t*)id, oc);
          fprintf(stderr, "[pc=%u, id=%s, type=%c] ", pc, id, format[od]);
          switch (format[od])
            {
            case ASSH_BIGNUM_NATIVE:
            case ASSH_BIGNUM_STEMP:
            case ASSH_BIGNUM_TEMP:
              fprintf(stderr, "[bits=%zu] ", src->bits);
              if (src->n != NULL)
                gcry_mpi_dump(src->n);
              else
                fprintf(stderr, "NULL");
              break;
            case ASSH_BIGNUM_SIZE:
              fprintf(stderr, "%u", (unsigned)(uintptr_t)args[od]);
              break;
            }
          fprintf(stderr, "\n");
#endif
          break;
        }

        }
    }

 end:
  err = ASSH_OK;
 err_sc:;
  for (i = 0; i < tlen; i++)
    if (tmp[i].n != NULL)
      gcry_mpi_release(tmp[i].n);
  return err;
}

static ASSH_BIGNUM_RELEASE_FCN(assh_bignum_gcrypt_release)
{
  gcry_mpi_release(bn->n);
  bn->n = NULL;
}

const struct assh_bignum_algo_s assh_bignum_gcrypt =
{
  .name = "gcrypt",
  .f_bytecode = assh_bignum_gcrypt_bytecode,
  .f_convert = assh_bignum_gcrypt_convert,
  .f_release = assh_bignum_gcrypt_release,
};


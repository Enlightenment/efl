int
_eet_hash_gen(const char *key, int hash_size)
{
   int			hash_num = 0;
   int			value, i;
   unsigned char	*ptr;
 
   const int masks[9] =
     {
	0x00,
	0x01,
	0x03,
	0x07,
	0x0f,
	0x1f,
	0x3f,
	0x7f,
	0xff
     };
 
   /* no string - index 0 */
   if (!key) return 0;
 
   /* calc hash num */
   for (i = 0, ptr = (unsigned char *)key, value = (int)(*ptr);
	value;
	ptr++, i++, value = (int)(*ptr))
     hash_num ^= (value | (value << 8)) >> (i & 0x7);

   /* mask it */
   hash_num &= masks[hash_size];
   /* return it */
   return hash_num;
}


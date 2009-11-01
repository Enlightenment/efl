/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#ifdef SMOOTH
{
   while (ww > 0)
     {
# ifdef COLBLACK
        *d = 0xff000000; // col
# else        
        FPc u1, v1, u2, v2;
        FPc rv, ru;
        DATA32 val1, val2, val3, val4;
        
        u1 = u;
        if (u1 < 0) u1 = 0;
        else if (u1 >= swp) u1 = swp - 1;
        
        v1 = v;
        if (v1 < 0) v1 = 0;
        else if (v1 >= shp) v1 = shp - 1;
        
        u2 = u1 + FPFPI1;
        if (u2 >= swp) u2 = swp - 1;
        
        v2 = v1 + FPFPI1;
        if (v2 >= shp) v2 = shp - 1;
        
        ru = (u >> (FP + FPI - 8)) & 0xff;
        rv = (v >> (FP + FPI - 8)) & 0xff;
        
        s = sp + ((v1 >> (FP + FPI)) * sw) + 
          (u1 >> (FP + FPI));
        val1 = *s;
        s = sp + ((v1 >> (FP + FPI)) * sw) + 
          (u2 >> (FP + FPI));
        val2 = *s;
        
        s = sp + ((v2 >> (FP + FPI)) * sw) + 
          (u1 >> (FP + FPI));
        val3 = *s;
        s = sp + ((v2 >> (FP + FPI)) * sw) + 
          (u2 >> (FP + FPI));
        val4 = *s;
#  ifdef SCALE_USING_MMX
        MOV_A2R(rv, mm4);
        MOV_A2R(ru, mm6);
        MOV_P2R(val1, mm1, mm0);
        if (val1 | val2)
          {
             MOV_P2R(val2, mm2, mm0);
             INTERP_256_R2R(mm6, mm2, mm1, mm5);
          }
        MOV_P2R(val3, mm2, mm0);
        if (val3 | val4)
          {
             MOV_P2R(val4, mm3, mm0);
             INTERP_256_R2R(mm6, mm3, mm2, mm5);
          }
        INTERP_256_R2R(mm4, mm2, mm1, mm5);
#   ifdef COLMUL
        cc = cv >> 16; // col
        MOV_A2R(cc, mm2); // col
        MOV_A2R(c1, mm3); // col
        MOV_A2R(c2, mm4); // col
        INTERP_256_R2R(mm2, mm4, mm3, mm5); // col
        MUL4_256_R2R(mm3, mm1);
#   endif                            
        MOV_R2P(mm1, *d, mm0);
#  else
        val1 = INTERP_256(ru, val2, val1);
        val3 = INTERP_256(ru, val4, val3);
        val1 = INTERP_256(rv, val3, val1); // col
#   ifdef COLMUL                            
        val2 = INTERP_256((cv >> 16), c2, c1); // col
        *d   = MUL4_SYM(val2, val1); // col
        cv += cd; // col
#   else                            
        *d   = INTERP_256(rv, val3, val1);
#   endif
#  endif
        u += ud;
        v += vd;
# endif        
        d++;
        ww--;
     }
}
#else
{
   while (ww > 0)
     {
# ifdef COLMUL
        DATA32 val1, cval; // col
# endif        
# ifdef COLBLACK
        *d = 0xff000000; // col
# else        
        s = sp + ((v >> (FP + FPI)) * sw) + 
          (u >> (FP + FPI));
#  ifdef COLMUL
        val1 = *s; // col
        cval = INTERP_256((cv >> 16), c2, c1); // col
        *d = MUL4_SYM(cval, val1);
        cv += cd; // col              
#  else        
        *d = *s;
#  endif        
        u += ud;
        v += vd;
# endif        
        d++;
        ww--;
     }
}
#endif

/* Simple bump map algorithms for the software engine */

#include "evas_common_private.h"
#include "evas_private.h"
#include "evas_filter.h"
#include "evas_filter_private.h"
#include "evas_blend_private.h"

#include <math.h>

#undef ENFN
#undef ENDT
#define ENFN cmd->ctx->evas->engine.func
#define ENDT cmd->ctx->evas->engine.data.output

#ifdef CLAMP
# undef CLAMP
#endif
#define CLAMP(a,b,c) MIN(MAX((b),(a)),(c))

#define DEFAULT_ZANGLE 45.f

static Eina_Bool _bump_map_cpu_alpha_alpha(Evas_Filter_Command *cmd);
static Eina_Bool _bump_map_cpu_alpha_rgba(Evas_Filter_Command *cmd);
static Eina_Bool _bump_map_cpu_rgba_rgba(Evas_Filter_Command *cmd);

Evas_Filter_Apply_Func
evas_filter_bump_map_cpu_func_get(Evas_Filter_Command *cmd)
{
   int w, h;

   EINA_SAFETY_ON_NULL_RETURN_VAL(cmd, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(cmd->input, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(cmd->mask, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(cmd->output, NULL);

   EINA_SAFETY_ON_FALSE_RETURN_VAL(cmd->input != cmd->output, NULL);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(cmd->mask->alpha_only, NULL);
   EINA_SAFETY_ON_FALSE_RETURN_VAL((!cmd->output->alpha_only)
                                   || cmd->input->alpha_only, NULL);

   w = cmd->input->w;
   h = cmd->input->h;
   EINA_SAFETY_ON_FALSE_RETURN_VAL(cmd->output->w == w, NULL);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(cmd->output->h == h, NULL);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(cmd->mask->w == w, NULL);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(cmd->mask->h == h, NULL);

   if (cmd->input->alpha_only)
     {
        if (cmd->output->alpha_only)
          return _bump_map_cpu_alpha_alpha;
        else
          return _bump_map_cpu_alpha_rgba;
     }
   else
     return _bump_map_cpu_rgba_rgba;
}

static void
_phong_alpha_generate(DATA8 *phong, DATA8 dark, DATA8 color, DATA8 white,
                      float sf)
{
   int x, y;

   // FIXME: Flat surfaces should be of color COLOR when compensate is set
   // FIXME: Include white in the computation for specular light
   (void) white;
   (void) sf;

   /*
   float3 lightDir = light.position - pos3D; //3D position in space of the surface
   float distance = length( lightDir );
   lightDir = lightDir / distance; // = normalize( lightDir );
   distance = distance * distance; //This line may be optimised using Inverse square root

   //Intensity of the diffuse light. Saturate to keep within the 0-1 range.
   float NdotL = dot( normal, lightDir );
   float intensity = saturate( NdotL );

   // Calculate the diffuse light factoring in light color, power and the attenuation
   OUT.Diffuse = intensity * light.diffuseColor * light.diffusePower / distance;

   //Calculate the half vector between the light vector and the view vector.
   //This is faster than calculating the actual reflective vector.
   float3 H = normalize( lightDir + viewDir );

   //Intensity of the specular light
   float NdotH = dot( normal, H );
   intensity = pow( saturate( NdotH ), specularHardness );

   //Sum up the specular light factoring
   OUT.Specular = intensity * light.specularColor * light.specularPower / distance;
   */

   for (y = 0; y < 256; y++)
     for (x = 0; x < 256; x++)
       {
          float dx = (127.5 - x);
          float dy = (127.5 - y);
          float dist = sqrt(dx*dx + dy*dy) * 2.;
          int diff = dark + MAX(255 - dist, 0) * (color - dark) / 255;
          int spec = 0; // TODO
          phong[x + (y << 8)] = MIN(MAX(diff + spec, 0), 255);
       }
}

static Eina_Bool
_bump_map_cpu_alpha_alpha(Evas_Filter_Command *cmd)
{
   DATA8 *src, *map, *dst, *map_y1, *map_y2;
   DATA8 dark, color, white;
   DATA8 *phong;
   int x, y, w, h, lx, ly;
   float xyangle, zangle, sf, lxy;

   w = cmd->input->w;
   h = cmd->input->h;
   EINA_SAFETY_ON_FALSE_RETURN_VAL(w > 2 && h > 2, EINA_FALSE);

   src = ((RGBA_Image *) cmd->input->backing)->image.data8;
   map = ((RGBA_Image *) cmd->mask->backing)->image.data8;
   dst = ((RGBA_Image *) cmd->output->backing)->image.data8;
   EINA_SAFETY_ON_NULL_RETURN_VAL(src, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(map, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(dst, EINA_FALSE);

   xyangle = cmd->bump.xyangle;
   zangle = cmd->bump.zangle;
   sf = cmd->bump.specular_factor;

   dark = cmd->bump.dark >> 24;
   white = cmd->bump.white >> 24;
   color = cmd->bump.color >> 24;

   // Convenience for alpha output only
   if ((!dark && !white && !color) ||
       (dark == 0xff && white == 0xff && color == 0xff))
     {
        INF("Bump colors are all 0 or 255. Using low byte instead of alpha.");
        dark = cmd->bump.dark & 0xff;
        white = cmd->bump.white & 0xff;
        color = cmd->bump.color & 0xff;
     }

   // Compute appropriate lx, ly
   if (abs(zangle) >= 90)
     {
        WRN("Z angle was defined as %.0f, out of range. Defaults to %.0f.",
            zangle, DEFAULT_ZANGLE);
        zangle = DEFAULT_ZANGLE;
     }

   lxy = sin(abs(zangle * M_PI / 180.));
   lx = (int) (40.f * (lxy + 1.0) * cos(xyangle * M_PI / 180.));
   ly = (int) (40.f * (lxy + 1.0) * sin(xyangle * M_PI / 180.));
   INF("Using light vector (%d,%d)", lx, ly);

   // Generate light table
   phong = malloc(256 * 256 * sizeof(*phong));
   EINA_SAFETY_ON_NULL_RETURN_VAL(phong, EINA_FALSE);
   _phong_alpha_generate(phong, dark, color, white, sf);

   for (y = 0; y < h; y++)
     {
        int gx, gy, vx, vy;

        if (!y)
          {
             map_y1 = map;
             map_y2 = map + w;
          }
        else if (y == (h - 1))
          {
             map_y1 = map - w;
             map_y2 = map;
          }
        else
          {
             map_y1 = map - w;
             map_y2 = map + w;
          }

        // x = 0
        gx = (map[1] - map[0]) / 2;
        gy = (*map_y2 - *map_y1) / 2;
        vx = gx + lx + 127;
        vy = (-gy) + ly + 127;

        //printf("dx,dy: %d,%d, lx,ly: %d,%d, vx,vy: %d,%d\n", gx, gy, lx, ly, vx, vy);

        if ((vx & 0xFF00) || (vy & 0xFF00))
          *dst = *src * dark;
        else
          *dst = (*src * phong[(vy << 8) + vx]) >> 8;

        dst++, src++, map_y1++, map_y2++;
        for (x = 1; x < (w - 1); x++, map++, map_y1++, map_y2++, src++, dst++)
          {
             // note: map is one pixel left of (x,y)

             if (!*src)
               {
                  *dst = 0;
                  continue;
               }

             // compute gradient (gx, gy). this gives us the normal vector
             gx = (map[2] - map[0]) / 2;
             gy = (*map_y2 - *map_y1) / 2;

             // compute halfway vector between light and gradient vectors
             vx = gx + lx + 127;
             vy = (-gy) + ly + 127;

             // take light from the phong table
             if ((vx & 0xFF00) || (vy & 0xFF00))
               *dst = *src * dark;
             else
               *dst = (*src * phong[(vy << 8) + vx]) >> 8;
          }

        // x = (w - 1)
        gx = (map[1] - map[0]) / 2;
        gy = (*map_y2 - *map_y1) / 2;
        vx = gx + lx + 127;
        vy = (-gy) + ly + 127;

        if ((vx & 0xFF00) || (vy & 0xFF00))
          *dst = *src * dark;
        else
          *dst = (*src * phong[(vy << 8) + vx]) >> 8;

        map += 2;
        dst++;
        src++;
     }

   free(phong);
   return EINA_TRUE;
}

static Eina_Bool
_bump_map_cpu_alpha_rgba(Evas_Filter_Command *cmd)
{
   DATA8 *src, *map, *map_y1, *map_y2;
   DATA32 *dst;
   DATA32 dark, color, white, col;
   //DATA32 *phong;
   Eina_Bool compensate;
   int x, y, w, h, lx, ly, lz, gz, NL, diffusion, gzlz, gz2;
   double xyangle, zangle, sf, lxy, elevation;

   w = cmd->input->w;
   h = cmd->input->h;
   EINA_SAFETY_ON_FALSE_RETURN_VAL(w > 2 && h > 2, EINA_FALSE);

   src = ((RGBA_Image *) cmd->input->backing)->image.data8;
   map = ((RGBA_Image *) cmd->mask->backing)->image.data8;
   dst = ((RGBA_Image *) cmd->output->backing)->image.data;
   EINA_SAFETY_ON_NULL_RETURN_VAL(src, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(map, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(dst, EINA_FALSE);

   xyangle = cmd->bump.xyangle;
   zangle = cmd->bump.zangle;
   sf = cmd->bump.specular_factor;
   compensate = cmd->bump.compensate;
   elevation = cmd->bump.elevation;

   dark = cmd->bump.dark;
   white = cmd->bump.white;
   color = cmd->bump.color;

   // Compute appropriate lx, ly
   if (abs(zangle) >= 90)
     {
        WRN("Z angle was defined as %.0f, out of range. Defaults to %.0f.",
            zangle, DEFAULT_ZANGLE);
        zangle = DEFAULT_ZANGLE;
     }

   lxy = 255. * cos(zangle * M_PI / 180.);
   lx = (int) (lxy * cos(xyangle * M_PI / 180.));
   ly = (int) (lxy * sin(xyangle * M_PI / 180.));
   lz = (int) (255. * sin(zangle));
   INF("Using light vector (%d,%d,%d)", lx, ly, lz);

   if (elevation <= 0)
     {
        WRN("Invalid elevation value of %.0f, using 10 instead.", elevation);
        elevation = 10.0;
     }

   gz = (6*255) / elevation;
   gzlz = gz * lz;
   gz2 = gz * gz;

   // Generate light table
   // FIXME: phong LUT not used (we need two)
   //phong = malloc(256 * 256 * sizeof(*phong));
   //EINA_SAFETY_ON_NULL_RETURN_VAL(phong, EINA_FALSE);
   //_phong_rgba_generate(phong, 1.5, sf, 20, dark, color, white);

   // FIXME: x=0 and x=w-1 are NOT implemented.

   for (y = 0; y < h; y++)
     {
        int gx, gy;

        if (!y)
          {
             map_y1 = map;
             map_y2 = map + w;
          }
        else if (y == (h - 1))
          {
             map_y1 = map - w;
             map_y2 = map;
          }
        else
          {
             map_y1 = map - w;
             map_y2 = map + w;
          }

        for (x = 0; x < w; x++, dst++, src++, map++, map_y1++, map_y2++)
          {
             if (!*src) continue;

             /* Color intensity is defined as:
              * I = Kd*N.L*Cd + Ks*N.H*Cs
              * Where Ks and Kd are 1 in this implementation
              * And Cs is white, Cd is color
              */

             /* Compute N.L
              * N = (gx,gy,gz)
              * L = (lx,ly,lz)   |L| = 255
              */

             if (EINA_LIKELY(x && (x < (w - 1))))
               {
                  gx = map[-1] + map_y1[-1] + map_y2[-1] - map[1] - map_y1[1] - map_y2[1];
                  gy = map_y2[-1] + map_y2[0] + map_y2[1] - map_y1[-1] - map_y1[0] - map_y1[1];
               }
             else if (!x)
               {
                  gx = map[0] + map_y1[0] + map_y2[0] - map[1] - map_y1[1] - map_y2[1];
                  gy = map_y2[0] + map_y2[1] + map_y2[1] - map_y1[0] - map_y1[1] - map_y1[1];
               }
             else
               {
                  gx = map[-1] + map_y1[-1] + map_y2[-1] - map[0] - map_y1[0] - map_y2[0];
                  gy = map_y2[-1] + map_y2[0] + map_y2[0] - map_y1[-1] - map_y1[0] - map_y1[0];
               }

             NL = gx*lx + gy*ly + gzlz;

             if (0 && NL < 0)
               {
                  // TODO: Check this
                  diffusion = lz;
               }
             else
               {
                  int g2 = gx*gx + gy*gy + gz2;
                  diffusion = NL / sqrt(MAX(g2, 1));
                  //diffusion += MAX(0, lz - diffusion);
               }

             if (compensate)
               diffusion = diffusion * 255 / lz;

             diffusion = CLAMP(1, diffusion + 1, 256);
             col = INTERP_256(diffusion, color, dark);

             if (sf > 0)
               {
                  /* Compute N.H
                   * H = (L+V) / |L+V|
                   * V = (0,0,255)
                   * L = (lx,ly,lz)   |L| = 255
                   */

                  // FIXME: All these doubles :)

                  int shine;
                  const double hnorm = sqrt(lx*lx + ly*ly + (lz+255)*(lz+255));
                  const double hx = (double) lx / hnorm;
                  const double hy = (double) ly / hnorm;
                  const double hz = (double) (lz+255) / hnorm;
                  double NHx = hx*gx / 255.0;
                  double NHy = hy*gy / 255.0;
                  double nz = sqrt(255.0*255.0 - gx*gx - gy*gy);
                  double NHz = (hz*nz) / 255.0;
                  double NH = NHx + NHy + NHz;
                  double specular = NH > 0 ? pow(NH, sf) : 0;

                  if (compensate)
                    {
                       const double basespecular = pow(hz, sf);
                       shine = (specular - basespecular) * 255.0 / (1.0 - basespecular);
                    }
                  else shine = specular * 255.0;

                  col = INTERP_256(CLAMP(1, shine + 1, 256), white, col);
               }

             *dst = INTERP_256(*src + 1, col, *dst);
          }
   }

   return EINA_TRUE;
}

static Eina_Bool
_bump_map_cpu_rgba_rgba(Evas_Filter_Command *cmd)
{
   (void) cmd;

   CRI("Not implemented yet.");
   return EINA_FALSE;
}

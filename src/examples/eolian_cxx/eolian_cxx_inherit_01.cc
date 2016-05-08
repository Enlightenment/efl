
#include <iostream>
#include <cassert>

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ns_colourable.eo.hh"
#include "ns_colourablesquare.eo.hh"

#include <eo_inherit.hh>

using namespace efl;

struct ColourableCircle
  : efl::eo::inherit<ColourableCircle, ::ns::Colourable>
{
   ColourableCircle(int rgb)
     : inherit_base(::ns::Colourable::rgb_24bits_constructor(rgb))
   {}

   int colour_get()
   {
      int rgb = 0;
      rgb = ::ns_colourable_colour_get(eo_super(_eo_ptr(), _eo_class()));
      std::cout << "ColourableCircle::colour_get(" << this << ") ==> "
                << std::hex << rgb << std::endl;
      return rgb;
   }
};

/*
struct ColourableFoo
  : efl::eo::inherit<ColourableFoo,
                     ::colourable,
                     ::colourablesquare>
{
   ColourableFoo(int size, int rgb)
     : inherit_base(efl::eo::args<::colourable>(size)
                  , efl::eo::args<::colourablesquare>(rgb))
   {}
};*/

struct ColourableBar
  : efl::eo::inherit<ColourableBar, ::ns::ColourableSquare>
{
   ColourableBar()
     : inherit_base(::ns::Colourable::rgb_24bits_constructor(0))
   {}

   int colour_get()
   {
      int rgb = 0;
      rgb = ::ns_colourable_colour_get(eo_super(_eo_ptr(), _eo_class()));
      std::cout << "ColourableBar::colour_get(" << this << ") ==> "
                << std::hex << rgb << std::endl;
      return rgb;
   }

};

int
main()
{
   efl::eo::eo_init init;
   eina_log_domain_level_set("colourable", EINA_LOG_LEVEL_DBG);

   ColourableCircle obj1(0x0);
   obj1.composite_colour_set(0xc0, 0xff, 0xee);

   ColourableCircle obj2(0xc0ffee);
   int r, g, b;
   obj2.composite_colour_get(&r, &g, &b);


   ColourableBar obj3;
   obj3.composite_colour_get(&r, &g, &b);

   assert(r == 0xc0);
   assert(g == 0xff);
   assert(b == 0xee);

   assert(obj1.colour_get() == obj2.colour_get());

   return 0;
}

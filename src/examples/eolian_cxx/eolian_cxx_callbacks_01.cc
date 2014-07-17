#include <iostream>
#include <thread>
#include <cassert>
#include <sstream>

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eo.h>
#include <Ecore.h>
#include <Evas.h>
#include <Ecore_Evas.h>

#include <Eina.hh>
#include <Eo.hh>
#include <Ecore.hh>
#include <Evas.hh>

#include <ecore_poll.eo.hh>

namespace efl { namespace ecore {
using ::ecore::poller;
} }

void
example_callbacks()
{
   int count = 0;
   efl::ecore::poller poller
      (ECORE_POLLER_CORE, 1,
       [&count, &poller]
      {
         if (++count == 5)
           {
              std::cout << std::endl;
              ecore_main_loop_quit();
              return false;
           }
         poller.interval_set(2*poller.interval_get());
         std::cout << "."<< std::flush;
         return true;
      });
   ::ecore_main_loop_begin();
}

int main()
{
   efl::eina::eina_init eina_;
   efl::eo::eo_init eo_;
   efl::ecore::ecore_init ecore_;

   std::cerr << "[+] Running EFL++ example: Callbacks" << std::endl;
   example_callbacks();

   return 0;
}


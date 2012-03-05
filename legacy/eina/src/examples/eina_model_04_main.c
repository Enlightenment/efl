/*
 * main_animal.c
 * compile with: gcc eina_model_04_*.c -o eina_model_04 `pkg-config --cflags --libs eina`
 */

/*
 * This example demonstrates the extended usage of Eina Model.
 * Class inheritance and interface implementation
 *
 *  Animal Class is inherited from BaseClass and implements
 *  "_breathe_animal()" and "_eat_animal()" methods.
 *
 *  Human Class is inherited from Animal class.
 *  Parrot Class is inherited from Animal class.
 *
 *  Child Class is inherited from Human class.
 *
 *  Human Class and Parrot Class implement Whistler Interface.
 *  Human Class implements Diver Interface. Diver Interface inherited from Swimmer Interface
 *
 *
 *                Animal Class (inherited from Base Class)
 *                   + _breathe_animal()
 *                   + _eat_animal()
 *             / -------/      \-------------\
 *            /                               \
 *     Human Class                          Parrot Class
 *     inherits                             inherits
 *     + animal_breathe()                   + animal_breathe()
 *     overrides                            overrides
 *     + animal_eat();                      + animal_eat();
 *     implements                           implements
 *     + human_walk();                      + parrot_fly();
 *
 *     implements Whistler, Swimmer,        implements Whistler,
 *     Diver Interfaces:                    + whistler_whistle()
 *     + whistler_whistle()
 *     + swimmer_swim()
 *     + diver_dive()
 *
 *             ----------------------------------------------------------
 *             |     Swim_Interface                                     |
 *             |       + swim()                                         |
 *             |          |                                             |
 *             |          |                                             |
 *             |      Dive Intarface (inherited from Swim Interface)    |
 *             |        + dive()                                        |
 *              ---------------------------------------------------------
 *          |
 *          |
 *          Child Class
 *     + inherits all parent's methods
 *     + implements cry_child()
 *     + overrides dive() interface method
 */

#include <Eina.h>
#include "eina_model_04_human.h"
#include "eina_model_04_parrot.h"
#include "eina_model_04_child.h"
#include "eina_model_04_whistler.h"

int
main()
{
   Eina_Model *h, *p, *c;

   eina_init();

   human_init();
   parrot_init();
   child_init();

   h = eina_model_new(HUMAN_TYPE);
   p = eina_model_new(PARROT_TYPE);
   c = eina_model_new(CHILD_TYPE);

   animal_breathe(p);
   animal_eat(p);
   parrot_fly(p);
   whistler_whistle(p);

   printf("\n");
   animal_breathe(h);
   animal_eat(h);
   human_walk(h);
   whistler_whistle(h);
   swimmer_swim(h);
   diver_dive(h);

   printf("\n");
   animal_breathe(c);
   animal_eat(c);
   human_walk(c);
   whistler_whistle(c);
   swimmer_swim(c);
   diver_dive(c);
   child_cry(c);

   eina_model_unref(c);
   eina_model_unref(h);
   eina_model_unref(p);

   eina_shutdown();

   return 0;
}




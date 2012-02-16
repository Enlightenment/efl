/*
 * whistler.h
 */

#ifndef WHISTLER_H_
#define WHISTLER_H_

#include <Eina.h>
#include <eina_safety_checks.h>

#define WHISTLER_INTERFACE_NAME "Whistler_Interface"
#define SWIMMER_INTERFACE_NAME "Swimmer_Interface"
#define DIVER_INTERFACE_NAME "Diver_Interface"

#define WHISTLER_INTERFACE(x) ((Whistler_Interface *) x)
#define SWIMMER_INTERFACE(x) ((Swimmer_Interface *) x)
#define DIVER_INTERFACE(x) ((Diver_Interface *) x)

typedef struct _Whistler_Interface
{
   Eina_Model_Interface base_interface;
   void (*whistle)(Eina_Model *);

} Whistler_Interface;

typedef struct _Swimmer_Interface
{
   Eina_Model_Interface base_interface;
   void (*swim)(Eina_Model *);

} Swimmer_Interface;

//Diver Interface will use Swimmer Interface as a parent
typedef struct _Diver_Interface
{
   Eina_Model_Interface base_interface;
   void (*dive)(Eina_Model *);

} Diver_Interface;

void whistler_whistle(Eina_Model *m);
void swimmer_swim(Eina_Model *m);
void diver_dive(Eina_Model *m);

#endif /* WHISTLER_H_ */

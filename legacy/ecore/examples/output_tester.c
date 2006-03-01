/* output and error tester.
 */

#include <Ecore.h>

#include <stdlib.h>
#include <stdio.h>

Ecore_Timer        *timer1 = NULL;

char               *data1 = "data1";

int
timer1_tick(void *data)
{
   printf("Once only timer called at %3.2f seconds, data %p\n",
	  ecore_time_get(), data);
   fprintf(stderr, "This is an error message from the timer callback.\n");
   fprintf(stdout, "This is an output message from the timer callback.\n");
   fprintf(stderr,
	   "Some text for testing the various problems that may occur.  It should all be to stderr.\n");
   fprintf(stderr, "!@#$%%^&*(){}|[]\\/\\\\=?+',.\"<>`~-_;:\n");
   fprintf(stderr, "1234567890\n");
   fprintf(stderr, "abcdefghijklmnopqrstuvwxyz\n");
   fprintf(stderr, "ABCDEFGHIJKLMNOPQRSTUVWXYZ\n");
   fprintf(stderr,
	   "This is a really long line that is used to test the systems response to really long lines.  Having said that, if I hadn't added this extra sentence it would not have been as long as the introduction line that is five lines above this one.  If you got this far, then you may actually have no life, as this is just a really long test line that says nothing important, and the only parts that have any actual relevance are the beginning and the end.\n");
   fprintf(stderr,
	   "This line is almost identical to the one above, only \\n's ae added to test multiline.\nThis is a really long line that is used to test the systems response to really long lines.\nHaving said that, if I hadn't added this extra sentence it would not have been as long as the introduction line that is five lines above this one.\nIf you got this far, then you may actually have no life, as this is just a really long test line that says nothing important, and the only parts that have any actual relevance are the beginning and the\nend.\n");
   fprintf(stderr, "&gt;&amp;&lt; == <&>\n");
   fprintf(stderr, "This - <br> - is a HTML line break.\n");
   fprintf(stderr, "&gt;br&lt; == <br>\n");
   fprintf(stderr,
	   "The rest is just some lines cut and paste from http://matrix-rad.net/ coz I need a big lump of text, and it was handy.  Used with the permission of the author, coz I (David 'onefang' Seikel) am the author, apply the standard E17 license to this part when it is part of E17 cvs.\n");
   fprintf(stderr, "\n");
   fprintf(stderr,
	   "NAWS stands for Not A Widget Set, it will not be a widget set, it will be a\n");
   fprintf(stderr,
	   "single, generalised widget and a single generalized widget container.\n");
   fprintf(stderr, "  </P>\n");
   fprintf(stderr, "  <P>\n");
   fprintf(stderr, "The idea is roughly this -<BR><BR>\n");
   fprintf(stderr, "\n");
   fprintf(stderr,
	   "NAWS should allow pixel perfect layout on any OS/JVM combination. This is very\n");
   fprintf(stderr,
	   "important, it gives the GUI designers the freedom from differences that has been\n");
   fprintf(stderr, "sorely lacking on the web.<BR><BR>\n");
   fprintf(stderr, "\n");
   fprintf(stderr,
	   "All widgets should be shaped, so that you can have two roundish widgets\n");
   fprintf(stderr,
	   "snuggling up to each other. All widgets should be font sensitive, this could\n");
   fprintf(stderr,
	   "break pixel perfection though. The shape should be definable by a mask graphic,\n");
   fprintf(stderr, "or a combination of graphics primitives.<BR><BR>\n");
   fprintf(stderr, "\n");
   fprintf(stderr,
	   "Labels should be able to display multiline flowed text or a graphic, with the\n");
   fprintf(stderr,
	   "graphic being a .png, .jpg, .gif (in that order of preference), and with\n");
   fprintf(stderr,
	   "animation support. Labels should be able to be attached to all other widgets as\n");
   fprintf(stderr,
	   "prompts. Labels should have automatic disabled graphics (ghosting) created and\n");
   fprintf(stderr,
	   "the option of supplying one. So that a widgets prompt gets ghosted when the\n");
   fprintf(stderr,
	   "widget gets ghosted, and to provide ghosting support for the other widget types.\n");
   fprintf(stderr,
	   "Labels can have a different text / graphic for the mouse hover state. Label\n");
   fprintf(stderr, "graphics should naturally be paintable.<BR><BR>\n");
   fprintf(stderr, "\n");
   fprintf(stderr,
	   "Borders are shaped outlines that can be put around anything. Not really a\n");
   fprintf(stderr,
	   "widget, just a decoration. When using graphics, the border could be part of the\n");
   fprintf(stderr,
	   "graphic, so no actual border is needed. Might be able to make Labels as\n");
   fprintf(stderr, "containers do the job of Borders.<BR><BR>\n");
   fprintf(stderr, "\n");
   fprintf(stderr,
	   "Buttons are Labels that can be clicked. Alternate text / graphics for the\n");
   fprintf(stderr,
	   "clicked state should be catered for. Radio buttons can be handled by widget\n");
   fprintf(stderr,
	   "notification. Checkboxes are buttons that are sticky. The difference between\n");
   fprintf(stderr,
	   "Checkboxes and toggled Buttons is purely cosmetic.<BR><BR>\n");
   fprintf(stderr, "\n");
   fprintf(stderr, "Icons are movable Buttons.<BR><BR>\n");
   fprintf(stderr, "\n");
   fprintf(stderr,
	   "Text editing fields are Labels that allow the text to be edited. Masks and\n");
   fprintf(stderr, "validation should be built in.<BR><BR>\n");
   fprintf(stderr, "\n");
   fprintf(stderr,
	   "Progress indicators are Labels with two graphics where the first graphic is\n");
   fprintf(stderr,
	   "slowly revealed by chipping away at the second overlayed graphic. Default\n");
   fprintf(stderr, "graphics should be supplied.<BR><BR>\n");
   fprintf(stderr, "\n");
   fprintf(stderr,
	   "Containers should be automatically scrollable if needed. Actually, Containers\n");
   fprintf(stderr,
	   "could be just Labels that allow sub widgets, but I don't think I will go that\n");
   fprintf(stderr, "far B-). On second thoughts, I might.<BR><BR>\n");
   fprintf(stderr, "\n");
   fprintf(stderr,
	   "All else is really just a Container with several of the basic widgets in them. A\n");
   fprintf(stderr,
	   "List is just a collection of Buttons. A Menu is not very different from a List\n");
   fprintf(stderr,
	   "with sub Lists. Choices are Lists where the selected item is usually all that is\n");
   fprintf(stderr, "\n");
   ecore_main_loop_quit();
   return 0;
}

int
main(int argc, char **argv)
{
   ecore_init();
   timer1 = ecore_timer_add(5.0, timer1_tick, data1);
   fprintf(stdout, "This is an output message from the main function.\n");
   fprintf(stderr, "This is an error message from the main function.\n");
   ecore_main_loop_begin();
   ecore_shutdown();
   return 123;
}

/*Type of events and callbacks for object Evas_Canvas3D_Node*/

#define GET_CALLBACK_TYPE(check, type)    \
   if (!(strcmp(type, "clicked")))        \
     check = PRIVATE_CALLBACK_CLICKED;    \
   else if (!(strcmp(type, "collision"))) \
     check = PRIVATE_CALLBACK_COLLISION;  \
   else                                   \
     check = PRIVATE_CALLBACK_NONE;

typedef enum _Evas_Canvas3D_Node_Private_Callback_Type
{
   PRIVATE_CALLBACK_CLICKED = 0,
   PRIVATE_CALLBACK_COLLISION,
   /*Insert here new type of callback*/
   PRIVATE_CALLBACK_NONE
} Evas_Canvas3D_Node_Private_Callback_Type;

const Eo_Event_Description evas_canvas3d_node_private_event_desc[] =
{
   EO_EVENT_DESCRIPTION("clicked,private"),
   EO_EVENT_DESCRIPTION("collision,private")
};

/*Private callbacks */
static void
_evas_canvas3d_node_private_callback_clicked(void *data, const Eo_Event *event);
static void
_evas_canvas3d_node_private_callback_collision(void *data, const Eo_Event *event);

Eo_Event_Cb evas_canvas3d_node_private_callback_functions[] =
{
   _evas_canvas3d_node_private_callback_clicked,
   _evas_canvas3d_node_private_callback_collision,
};


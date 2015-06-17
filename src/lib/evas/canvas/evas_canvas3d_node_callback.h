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
   {"clicked,private", "private event clicked", EINA_FALSE},
   {"collision,private", "private event collision", EINA_FALSE}
};

/*Private callbacks */
static Eina_Bool
_evas_canvas3d_node_private_callback_clicked(void *data, Eo *obj, const Eo_Event_Description *desc,
									   void *event_info);
static Eina_Bool
_evas_canvas3d_node_private_callback_collision(void *data, Eo *obj, const Eo_Event_Description *desc,
										 void *event_info);

Eo_Event_Cb evas_canvas3d_node_private_callback_functions[] =
{
   _evas_canvas3d_node_private_callback_clicked,
   _evas_canvas3d_node_private_callback_collision,
};


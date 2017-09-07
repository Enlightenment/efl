
typedef Eo*(*Efl_Model_Accessor_View_Constructor_Cb)(void* data, Eo* child);

Eina_Accessor* efl_model_accessor_view_new(Eina_Accessor* accessor,
                                           Efl_Model_Accessor_View_Constructor_Cb constructor, void* data);

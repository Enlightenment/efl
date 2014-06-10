
typedef enum _Elm_Atspi_Text_Granulatity Elm_Atspi_Text_Granularity;

enum _Elm_Atspi_Text_Granulatity
{
   ELM_ATSPI_TEXT_GRANULARITY_CHAR,
   ELM_ATSPI_TEXT_GRANULARITY_WORD,
   ELM_ATSPI_TEXT_GRANULARITY_SENTENCE,
   ELM_ATSPI_TEXT_GRANULARITY_LINE,
   ELM_ATSPI_TEXT_GRANULARITY_PARAGRAPH
};

typedef struct _Elm_Atspi_Text_Attribute Elm_Atspi_Text_Attribute;

struct _Elm_Atspi_Text_Attribute
{
   const char *name;
   const char *value;
};

typedef struct _Elm_Atspi_Text_Range Elm_Atspi_Text_Range;

struct _Elm_Atspi_Text_Range
{
   int start_offset;
   int end_offset;
   char *content;
};

typedef enum _Elm_Atspi_Text_Clip_Type Elm_Atspi_Text_Clip_Type;

enum _Elm_Atspi_Text_Clip_Type
{
    ELM_ATSPI_TEXT_CLIP_NONE,
    ELM_ATSPI_TEXT_CLIP_MIN,
    ELM_ATSPI_TEXT_CLIP_MAX,
    ELM_ATSPI_TEXT_CLIP_BOTH
};

/**
 * @brief Free Elm_Atspi_Text_Attribute structure
 */
void elm_atspi_text_text_attribute_free(Elm_Atspi_Text_Attribute *attr);

typedef struct _Elm_Atspi_Text_Change_Info Elm_Atspi_Text_Change_Info;

struct _Elm_Atspi_Text_Change_Info
{
   const char *content;
   Eina_Bool inserted;
   size_t pos;
   size_t len;
};

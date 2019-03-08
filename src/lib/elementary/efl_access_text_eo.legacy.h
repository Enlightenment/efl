#ifndef _EFL_ACCESS_TEXT_EO_LEGACY_H_
#define _EFL_ACCESS_TEXT_EO_LEGACY_H_

#ifndef _EFL_ACCESS_TEXT_EO_CLASS_TYPE
#define _EFL_ACCESS_TEXT_EO_CLASS_TYPE

typedef Eo Efl_Access_Text;

#endif

#ifndef _EFL_ACCESS_TEXT_EO_TYPES
#define _EFL_ACCESS_TEXT_EO_TYPES

/** Text accessibility granularity
 *
 * @ingroup Efl_Access
 */
typedef enum
{
  EFL_ACCESS_TEXT_GRANULARITY_CHAR = 0, /**< Character granularity */
  EFL_ACCESS_TEXT_GRANULARITY_WORD, /**< Word granularity */
  EFL_ACCESS_TEXT_GRANULARITY_SENTENCE, /**< Sentence granularity */
  EFL_ACCESS_TEXT_GRANULARITY_LINE, /**< Line granularity */
  EFL_ACCESS_TEXT_GRANULARITY_PARAGRAPH /**< Paragraph granularity */
} Efl_Access_Text_Granularity;

/** Text clip type
 *
 * @ingroup Efl_Access
 */
typedef enum
{
  EFL_ACCESS_TEXT_CLIP_TYPE_NONE = 0, /**< No clip type */
  EFL_ACCESS_TEXT_CLIP_TYPE_MIN, /**< Minimum clip type */
  EFL_ACCESS_TEXT_CLIP_TYPE_MAX, /**< Maximum clip type */
  EFL_ACCESS_TEXT_CLIP_TYPE_BOTH /**< Both clip types */
} Efl_Access_Text_Clip_Type;

/** Text attribute
 *
 * @ingroup Efl_Access
 */
typedef struct _Efl_Access_Text_Attribute
{
  const char *name; /**< Text attribute name */
  const char *value; /**< Text attribute value */
} Efl_Access_Text_Attribute;

/** Text range
 *
 * @ingroup Efl_Access
 */
typedef struct _Efl_Access_Text_Range
{
  int start_offset; /**< Range start offset */
  int end_offset; /**< Range end offset */
  char *content; /**< Range content */
} Efl_Access_Text_Range;

/** Text change information
 *
 * @ingroup Efl_Access
 */
typedef struct _Efl_Access_Text_Change_Info
{
  const char *content; /**< Change content */
  Eina_Bool inserted; /**< @c true if text got inserted */
  size_t pos; /**< Change position */
  size_t len; /**< Change length */
} Efl_Access_Text_Change_Info;


#endif
#endif

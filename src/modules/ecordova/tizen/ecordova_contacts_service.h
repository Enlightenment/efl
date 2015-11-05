#ifndef _ECORDOVA_CONTACTS_SERVICE_H
#define _ECORDOVA_CONTACTS_SERVICE_H

#include <stdbool.h>
#include <errno.h>
#include <stdlib.h>

#define CONTACTS_API

struct __contacts_record_h;
typedef struct __contacts_record_h* contacts_record_h;
struct __contacts_filter_h;
typedef struct __contacts_record_h* contacts_filter_h;
struct __contacts_list_h;
typedef struct __contacts_list_h* contacts_list_h;
struct __contacts_query_h;
typedef struct __contacts_query_h* contacts_query_h;

#define _CONTACTS_BEGIN_VIEW() \
    typedef struct{ \
        const char* const _uri;
#define _CONTACTS_BEGIN_READ_ONLY_VIEW() _CONTACTS_BEGIN_VIEW()
#define _CONTACTS_PROPERTY_INT(property_id_name)    unsigned int property_id_name;
#define _CONTACTS_PROPERTY_STR(property_id_name)    unsigned int property_id_name;
#define _CONTACTS_PROPERTY_BOOL(property_id_name)   unsigned int property_id_name;
#define _CONTACTS_PROPERTY_DOUBLE(property_id_name) unsigned int property_id_name;
#define _CONTACTS_PROPERTY_LLI(property_id_name)    unsigned int property_id_name;

#define _CONTACTS_PROPERTY_CHILD_SINGLE(property_id_name)   unsigned int property_id_name;
#define _CONTACTS_PROPERTY_CHILD_MULTIPLE(property_id_name) unsigned int property_id_name;

#define _CONTACTS_PROPERTY_FILTER_INT(property_id_name)     unsigned int property_id_name;
#define _CONTACTS_PROPERTY_FILTER_STR(property_id_name)     unsigned int property_id_name;
#define _CONTACTS_PROPERTY_FILTER_BOOL(property_id_name)    unsigned int property_id_name;
#define _CONTACTS_PROPERTY_FILTER_DOUBLE(property_id_name)  unsigned int property_id_name;
#define _CONTACTS_PROPERTY_FILTER_LLI(property_id_name)     unsigned int property_id_name;

#define _CONTACTS_PROPERTY_PROJECTION_INT(property_id_name)     unsigned int property_id_name;
#define _CONTACTS_PROPERTY_PROJECTION_STR(property_id_name)     unsigned int property_id_name;
#define _CONTACTS_PROPERTY_PROJECTION_BOOL(property_id_name)    unsigned int property_id_name;
#define _CONTACTS_PROPERTY_PROJECTION_DOUBLE(property_id_name)  unsigned int property_id_name;
#define _CONTACTS_PROPERTY_PROJECTION_LLI(property_id_name)     unsigned int property_id_name;
#define _CONTACTS_END_VIEW(name) \
    } name##_property_ids; \
    extern CONTACTS_API const name##_property_ids* name;
#define _CONTACTS_END_READ_ONLY_VIEW(name) _CONTACTS_END_VIEW(name)

_CONTACTS_BEGIN_VIEW()
    _CONTACTS_PROPERTY_INT(id)                /* read only */
    _CONTACTS_PROPERTY_INT(contact_id)        /* read, write once */
    _CONTACTS_PROPERTY_INT(type)              /* read, write */
    _CONTACTS_PROPERTY_STR(label)             /* read, write */
    _CONTACTS_PROPERTY_STR(postbox)           /* read, write */
    _CONTACTS_PROPERTY_STR(extended)          /* read, write */
    _CONTACTS_PROPERTY_STR(street)            /* read, write */
    _CONTACTS_PROPERTY_STR(locality)          /* read, write */
    _CONTACTS_PROPERTY_STR(region)            /* read, write */
    _CONTACTS_PROPERTY_STR(postal_code)       /* read, write */
    _CONTACTS_PROPERTY_STR(country)           /* read, write */
    _CONTACTS_PROPERTY_BOOL(is_default)       /* read, write */
_CONTACTS_END_VIEW(_contacts_address)

_CONTACTS_BEGIN_VIEW()
    _CONTACTS_PROPERTY_INT(id)                        /* read only */
    _CONTACTS_PROPERTY_STR(display_name)              /* read only */
    _CONTACTS_PROPERTY_INT(display_source_type)       /* read only */
    _CONTACTS_PROPERTY_INT(address_book_id)           /* read, write once */
    _CONTACTS_PROPERTY_STR(ringtone_path)             /* read, write */
    _CONTACTS_PROPERTY_STR(image_thumbnail_path)      /* read only */
    _CONTACTS_PROPERTY_BOOL(is_favorite)              /* read, write */
    _CONTACTS_PROPERTY_BOOL(has_phonenumber)          /* read only */
    _CONTACTS_PROPERTY_BOOL(has_email)                /* read only */
    _CONTACTS_PROPERTY_INT(person_id)                 /* read, write once */
    _CONTACTS_PROPERTY_STR(uid)                       /* read, write */
    _CONTACTS_PROPERTY_STR(vibration)                 /* read, write */
    _CONTACTS_PROPERTY_INT(changed_time)              /* read only */
    _CONTACTS_PROPERTY_INT(link_mode)                 /* read, write */
    _CONTACTS_PROPERTY_CHILD_SINGLE(name)             /* read, write */
    _CONTACTS_PROPERTY_CHILD_MULTIPLE(image)          /* read, write */
    _CONTACTS_PROPERTY_CHILD_MULTIPLE(company)        /* read, write */
    _CONTACTS_PROPERTY_CHILD_MULTIPLE(note)           /* read, write */
    _CONTACTS_PROPERTY_CHILD_MULTIPLE(number)         /* read, write */
    _CONTACTS_PROPERTY_CHILD_MULTIPLE(email)          /* read, write */
    _CONTACTS_PROPERTY_CHILD_MULTIPLE(event)          /* read, write */
    _CONTACTS_PROPERTY_CHILD_MULTIPLE(messenger)      /* read, write */
    _CONTACTS_PROPERTY_CHILD_MULTIPLE(address)        /* read, write */
    _CONTACTS_PROPERTY_CHILD_MULTIPLE(url)            /* read, write */
    _CONTACTS_PROPERTY_CHILD_MULTIPLE(nickname)       /* read, write */
    _CONTACTS_PROPERTY_CHILD_MULTIPLE(profile)        /* read, write */
    _CONTACTS_PROPERTY_CHILD_MULTIPLE(relationship)   /* read, write */
    _CONTACTS_PROPERTY_CHILD_MULTIPLE(group_relation) /* read, write */
    _CONTACTS_PROPERTY_CHILD_MULTIPLE(extension)      /* read, write */
    _CONTACTS_PROPERTY_STR(message_alert)             /* read, write */
_CONTACTS_END_VIEW(_contacts_contact)

_CONTACTS_BEGIN_VIEW()
    _CONTACTS_PROPERTY_INT(id)                    /* read only */
    _CONTACTS_PROPERTY_INT(contact_id)            /* read, write once */
    _CONTACTS_PROPERTY_STR(name)                  /* read, write */
_CONTACTS_END_VIEW(_contacts_nickname)

_CONTACTS_BEGIN_VIEW()
    _CONTACTS_PROPERTY_INT(id)            /* read only */
    _CONTACTS_PROPERTY_INT(contact_id)    /* read, write once */
    _CONTACTS_PROPERTY_STR(note)          /* read, write */
_CONTACTS_END_VIEW(_contacts_note)

_CONTACTS_BEGIN_VIEW()
    _CONTACTS_PROPERTY_INT(id)                /* read only */
    _CONTACTS_PROPERTY_INT(contact_id)        /* read, write once */
    _CONTACTS_PROPERTY_STR(first)             /* read, write */
    _CONTACTS_PROPERTY_STR(last)              /* read, write */
    _CONTACTS_PROPERTY_STR(addition)          /* read, write */
    _CONTACTS_PROPERTY_STR(suffix)            /* read, write */
    _CONTACTS_PROPERTY_STR(prefix)            /* read, write */
    _CONTACTS_PROPERTY_STR(phonetic_first)    /* read, write */
    _CONTACTS_PROPERTY_STR(phonetic_middle)   /* read, write */
    _CONTACTS_PROPERTY_STR(phonetic_last)     /* read, write */
_CONTACTS_END_VIEW(_contacts_name)

_CONTACTS_BEGIN_VIEW()
    _CONTACTS_PROPERTY_INT(id)                /* read only */
    _CONTACTS_PROPERTY_INT(contact_id)        /* read, write once */
    _CONTACTS_PROPERTY_INT(type)              /* read, write */
    _CONTACTS_PROPERTY_STR(label)             /* read, write */
    _CONTACTS_PROPERTY_BOOL(is_default)       /* read, write */
    _CONTACTS_PROPERTY_STR(number)            /* read, write */
    _CONTACTS_PROPERTY_STR(normalized_number) /* filter only */
    _CONTACTS_PROPERTY_STR(cleaned_number)    /* filter only */
    _CONTACTS_PROPERTY_STR(number_filter)     /* filter only */
_CONTACTS_END_VIEW(_contacts_number)

_CONTACTS_BEGIN_VIEW()
    _CONTACTS_PROPERTY_INT(id)                    /* read only */
    _CONTACTS_PROPERTY_INT(contact_id)            /* read, write once */
    _CONTACTS_PROPERTY_INT(type)                  /* read, write */
    _CONTACTS_PROPERTY_STR(label)                 /* read, write */
    _CONTACTS_PROPERTY_STR(name)                  /* read, write */
    _CONTACTS_PROPERTY_STR(department)            /* read, write */
    _CONTACTS_PROPERTY_STR(job_title)             /* read, write */
    _CONTACTS_PROPERTY_STR(assistant_name)        /* read, write */
    _CONTACTS_PROPERTY_STR(role)                  /* read, write */
    _CONTACTS_PROPERTY_STR(logo)                  /* read, write */
    _CONTACTS_PROPERTY_STR(location)              /* read, write */
    _CONTACTS_PROPERTY_STR(description)           /* read, write */
    _CONTACTS_PROPERTY_STR(phonetic_name)         /* read, write */
_CONTACTS_END_VIEW(_contacts_company)

_CONTACTS_BEGIN_VIEW()
    _CONTACTS_PROPERTY_INT(id)                /* read only */
    _CONTACTS_PROPERTY_INT(contact_id)        /* read, write once */
    _CONTACTS_PROPERTY_INT(type)              /* read, write */
    _CONTACTS_PROPERTY_STR(label)             /* read, write */
    _CONTACTS_PROPERTY_BOOL(is_default)       /* read, write */
    _CONTACTS_PROPERTY_STR(email)             /* read, write */
_CONTACTS_END_VIEW(_contacts_email)

_CONTACTS_BEGIN_VIEW()
    _CONTACTS_PROPERTY_INT(id)                    /* read only */
    _CONTACTS_PROPERTY_INT(contact_id)            /* read, write once */
    _CONTACTS_PROPERTY_INT(type)                  /* read, write */
    _CONTACTS_PROPERTY_STR(label)                 /* read, write */
    _CONTACTS_PROPERTY_STR(im_id)                 /* read, write */
_CONTACTS_END_VIEW(_contacts_messenger)

_CONTACTS_BEGIN_VIEW()
    _CONTACTS_PROPERTY_INT(id)                /* read only */
    _CONTACTS_PROPERTY_INT(contact_id)        /* read, write once */
    _CONTACTS_PROPERTY_INT(type)              /* read, write */
    _CONTACTS_PROPERTY_STR(label)             /* read, write */
    _CONTACTS_PROPERTY_STR(path)              /* read, write */
    _CONTACTS_PROPERTY_BOOL(is_default)
_CONTACTS_END_VIEW(_contacts_image)

_CONTACTS_BEGIN_VIEW()
    _CONTACTS_PROPERTY_INT(id)                /* read only */
    _CONTACTS_PROPERTY_INT(contact_id)        /* read, write once */
    _CONTACTS_PROPERTY_INT(type)              /* read, write */
    _CONTACTS_PROPERTY_STR(label)             /* read, write */
    _CONTACTS_PROPERTY_STR(url)               /* read, write */
_CONTACTS_END_VIEW(_contacts_url)

_CONTACTS_BEGIN_VIEW()
    _CONTACTS_PROPERTY_INT(id)                /* read only */
    _CONTACTS_PROPERTY_INT(contact_id)        /* read, write once */
    _CONTACTS_PROPERTY_INT(type)              /* read, write */
    _CONTACTS_PROPERTY_STR(label)             /* read, write */
    _CONTACTS_PROPERTY_INT(date)              /* read, write */
    _CONTACTS_PROPERTY_INT(calendar_type)     /* read, write */
    _CONTACTS_PROPERTY_BOOL(is_leap_month)    /* read, write (Deprecated since 2.4) */
_CONTACTS_END_VIEW(_contacts_event)

typedef enum {
    CONTACTS_FILTER_OPERATOR_AND,   /**< AND */
    CONTACTS_FILTER_OPERATOR_OR     /**< OR */
} contacts_filter_operator_e;

typedef enum {
    CONTACTS_ADDRESS_TYPE_OTHER,            /**< Other address type */
    CONTACTS_ADDRESS_TYPE_CUSTOM = 1<<0,    /**< A delivery address for a residence */
    CONTACTS_ADDRESS_TYPE_HOME = 1<<1,      /**< A delivery address for a residence */
    CONTACTS_ADDRESS_TYPE_WORK = 1<<2,      /**< A delivery address for a place of work */
    CONTACTS_ADDRESS_TYPE_DOM = 1<<3,       /**< A domestic delivery address */
    CONTACTS_ADDRESS_TYPE_INTL = 1<<4,      /**< An international delivery address */
    CONTACTS_ADDRESS_TYPE_POSTAL = 1<<5,    /**< A postal delivery address */
    CONTACTS_ADDRESS_TYPE_PARCEL = 1<<6,    /**< A parcel delivery address */
}contacts_address_type_e;

typedef enum {
    CONTACTS_NUMBER_TYPE_OTHER,                 /**< Other number type */
    CONTACTS_NUMBER_TYPE_CUSTOM = 1<<0,         /**< Custom number type */
    CONTACTS_NUMBER_TYPE_HOME = 1<<1,           /**< A telephone number associated with a residence */
    CONTACTS_NUMBER_TYPE_WORK = 1<<2,           /**< A telephone number associated with a place of work */
    CONTACTS_NUMBER_TYPE_VOICE = 1<<3,          /**< A voice telephone number */
    CONTACTS_NUMBER_TYPE_FAX = 1<<4,            /**< A facsimile telephone number */
    CONTACTS_NUMBER_TYPE_MSG = 1<<5,            /**< The telephone number has voice messaging support */
    CONTACTS_NUMBER_TYPE_CELL = 1<<6,           /**< A cellular telephone number */
    CONTACTS_NUMBER_TYPE_PAGER = 1<<7,          /**< A paging device telephone number */
    CONTACTS_NUMBER_TYPE_BBS = 1<<8,            /**< A bulletin board system telephone number */
    CONTACTS_NUMBER_TYPE_MODEM = 1<<9,          /**< A MODEM connected telephone number */
    CONTACTS_NUMBER_TYPE_CAR = 1<<10,           /**< A car-phone telephone number */
    CONTACTS_NUMBER_TYPE_ISDN = 1<<11,          /**< An ISDN service telephone number */
    CONTACTS_NUMBER_TYPE_VIDEO = 1<<12,         /**< A video conferencing telephone number */
    CONTACTS_NUMBER_TYPE_PCS = 1<<13,           /**< A personal communication services telephone number */
    CONTACTS_NUMBER_TYPE_COMPANY_MAIN = 1<<14,  /**< A company main number */
    CONTACTS_NUMBER_TYPE_RADIO = 1<<15,         /**< A radio phone number */
    CONTACTS_NUMBER_TYPE_MAIN = 1<<29,          /**< An additional type for main */
    CONTACTS_NUMBER_TYPE_ASSISTANT = 1<<30,     /**< An additional type for assistant */
}contacts_number_type_e;

typedef enum {
    CONTACTS_EMAIL_TYPE_OTHER,          /**< Other email type */
    CONTACTS_EMAIL_TYPE_CUSTOM = 1<<0,  /**< Custom email type */
    CONTACTS_EMAIL_TYPE_HOME = 1<<1,    /**< An email address associated with a residence */
    CONTACTS_EMAIL_TYPE_WORK = 1<<2,    /**< An email address associated with a place of work */
    CONTACTS_EMAIL_TYPE_MOBILE = 1<<3,  /**< A mobile email address */
}contacts_email_type_e;

typedef enum{
    CONTACTS_MESSENGER_TYPE_OTHER,      /**< Other messenger type */
    CONTACTS_MESSENGER_TYPE_CUSTOM,     /**< Custom messenger type */
    CONTACTS_MESSENGER_TYPE_GOOGLE,     /**< Google messenger type */
    CONTACTS_MESSENGER_TYPE_WLM,        /**< Windows live messenger type */
    CONTACTS_MESSENGER_TYPE_YAHOO,      /**< Yahoo messenger type */
    CONTACTS_MESSENGER_TYPE_FACEBOOK,   /**< Facebook messenger type */
    CONTACTS_MESSENGER_TYPE_ICQ,        /**< ICQ type */
    CONTACTS_MESSENGER_TYPE_AIM,        /**< AOL instance messenger type */
    CONTACTS_MESSENGER_TYPE_QQ,         /**< QQ type */
    CONTACTS_MESSENGER_TYPE_JABBER,     /**< Jabber type */
    CONTACTS_MESSENGER_TYPE_SKYPE,      /**< Skype type */
    CONTACTS_MESSENGER_TYPE_IRC,        /**< IRC type */
}contacts_messenger_type_e;

typedef enum {
    CONTACTS_URL_TYPE_OTHER,    /**< Other URL type*/
    CONTACTS_URL_TYPE_CUSTOM,   /**< Custom URL type */
    CONTACTS_URL_TYPE_HOME,     /**< Home URL type */
    CONTACTS_URL_TYPE_WORK,     /**< Work URL type */
}contacts_url_type_e;

typedef enum {
    CONTACTS_COMPANY_TYPE_OTHER,            /**< Other company type */
    CONTACTS_COMPANY_TYPE_CUSTOM = 1<<0,    /**< Custom company type */
    CONTACTS_COMPANY_TYPE_WORK = 1<<1,      /**< Work company type */
}contacts_company_type_e;

typedef enum
{
    CONTACTS_MATCH_EQUAL,                   /**< '=' */
    CONTACTS_MATCH_GREATER_THAN,            /**< '>' */
    CONTACTS_MATCH_GREATER_THAN_OR_EQUAL,   /**< '>=' */
    CONTACTS_MATCH_LESS_THAN,               /**< '<' */
    CONTACTS_MATCH_LESS_THAN_OR_EQUAL,      /**< '<=' */
    CONTACTS_MATCH_NOT_EQUAL,               /**< '<>', this flag can yield poor performance */
    CONTACTS_MATCH_NONE,                    /**< IS NULL */
} contacts_match_int_flag_e;

typedef enum
{
    CONTACTS_MATCH_EXACTLY,         /**< Full string, case-sensitive */
    CONTACTS_MATCH_FULLSTRING,      /**< Full string, case-insensitive */
    CONTACTS_MATCH_CONTAINS,        /**< Sub string, case-insensitive */
    CONTACTS_MATCH_STARTSWITH,      /**< Start with, case-insensitive */
    CONTACTS_MATCH_ENDSWITH,        /**< End with, case-insensitive */
    CONTACTS_MATCH_EXISTS           /**< IS NOT NULL */
} contacts_match_str_flag_e;

typedef enum {
    CONTACTS_EVENT_TYPE_OTHER,          /**< Other event type */
    CONTACTS_EVENT_TYPE_CUSTOM,         /**< Custom event type */
    CONTACTS_EVENT_TYPE_BIRTH,          /**< Birthday event type */
    CONTACTS_EVENT_TYPE_ANNIVERSARY     /**< Anniversary event type */
}contacts_event_type_e;

// Record
extern int (*contacts_record_create)(const char *view_uri, contacts_record_h *record);
extern int (*contacts_record_destroy )(contacts_record_h record, bool delete_child);
extern int (*contacts_record_clone)(contacts_record_h record, contacts_record_h* cloned_record);
extern int (*contacts_record_get_child_record_count)(contacts_record_h record, unsigned int property_id, int *count);
extern int (*contacts_record_get_child_record_at_p)(contacts_record_h record, unsigned int property_id, int index, contacts_record_h* child_record);
extern int (*contacts_record_get_str)(contacts_record_h record, unsigned int property_id, char** value);
extern int (*contacts_record_get_str_p)(contacts_record_h record, unsigned int property_id, char** value);
extern int (*contacts_record_get_int)(contacts_record_h record, unsigned int property_id, int* value);
extern int (*contacts_record_set_str)(contacts_record_h record, unsigned int property_id, const char* value);
extern int (*contacts_record_set_int)(contacts_record_h record, unsigned int property_id, int value);
extern int (*contacts_record_get_bool)(contacts_record_h record, unsigned int property_id, bool *value);
extern int (*contacts_record_set_bool)(contacts_record_h record, unsigned int property_id, bool value);
extern int (*contacts_record_remove_child_record)(contacts_record_h record, unsigned int property_id, contacts_record_h child_record);
extern int (*contacts_record_add_child_record)(contacts_record_h record, unsigned int property_id, contacts_record_h child_record);

// DB
extern int (*contacts_db_delete_record)(const char* view_uri, int record_id);
extern int (*contacts_db_update_record)(contacts_record_h record);
extern int (*contacts_db_insert_record)(contacts_record_h record, int *id);
extern int (*contacts_db_get_record)(const char* view_uri, int record_id, contacts_record_h* record);
extern int (*contacts_db_get_all_records)(const char* view_uri, int offset, int limit, contacts_list_h* record_list);
extern int (*contacts_db_get_records_with_query)(contacts_query_h query, int offset, int limit, contacts_list_h* record_list);

// List
extern int (*contacts_list_get_current_record_p)(contacts_list_h contacts_list, contacts_record_h* record);
extern int (*contacts_list_next)(contacts_list_h contacts_list);
extern int (*contacts_list_destroy)(contacts_list_h contacts_list, bool delete_child);
extern int (*contacts_list_first)(contacts_list_h contacts_list);
extern int (*contacts_list_create)(contacts_list_h* contacts_list);
extern int (*contacts_list_get_count)(contacts_list_h contacts_list, int *count);
extern int (*contacts_list_add)(contacts_list_h contacts_list, contacts_record_h record);

// Filter
extern int (*contacts_filter_create)(const char* view_uri, contacts_filter_h* filter);
extern int (*contacts_filter_add_operator)(contacts_filter_h filter, contacts_filter_operator_e operator_type);
extern int (*contacts_filter_add_int)(contacts_filter_h filter, unsigned int property_id, contacts_match_int_flag_e match, int match_value);
extern int (*contacts_filter_destroy)(contacts_filter_h filter);
extern int (*contacts_filter_add_str)(contacts_filter_h filter, unsigned int property_id, contacts_match_str_flag_e match, const char* match_value);

// Query
extern int (*contacts_query_create)(const char* view_uri, contacts_query_h* query);
extern int (*contacts_query_set_filter)(contacts_query_h query, contacts_filter_h filter);
extern int (*contacts_query_destroy)(contacts_query_h query);

typedef struct _Ecordova_Contact_Data Ecordova_Contact_Data;

/* Check if slp error or not */
#define TIZEN_ERROR_MIN_PLATFORM_ERROR (-1073741824LL) /* = -2147483648 / 2 */
/* Tizen Contacts Error */
#define TIZEN_ERROR_CONTACTS		-0x02010000

/**
 * @brief Enumeration for tizen errors.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 *
*/
typedef enum
{
    TIZEN_ERROR_NONE = 0, /**< Successful */
    TIZEN_ERROR_OUT_OF_MEMORY = -ENOMEM, /**< Out of memory */
    TIZEN_ERROR_INVALID_PARAMETER = -EINVAL, /**< Invalid function parameter */
    TIZEN_ERROR_FILE_NO_SPACE_ON_DEVICE = -ENOSPC, /**< No space left on device */
    TIZEN_ERROR_PERMISSION_DENIED = -EACCES, /**< Permission denied */
    TIZEN_ERROR_NO_DATA = -ENODATA, /**< No data available */

    TIZEN_ERROR_UNKNOWN = TIZEN_ERROR_MIN_PLATFORM_ERROR, /**< Unknown error */

    /* This is a place to add new errors here.
     * Do not assign integer values explicitly. Values are auto-assigned.
     */
    TIZEN_ERROR_TIMED_OUT, /**< Time out */
    TIZEN_ERROR_NOT_SUPPORTED, /**< Not supported */
    TIZEN_ERROR_USER_NOT_CONSENTED, /**< Not Consented */
    TIZEN_ERROR_END_OF_COLLECTION,
} tizen_error_e;

typedef enum
{
    /* GENERAL */
    CONTACTS_ERROR_NONE                 = TIZEN_ERROR_NONE,                      /**< Successful */
    CONTACTS_ERROR_OUT_OF_MEMORY        = TIZEN_ERROR_OUT_OF_MEMORY,             /**< Out of memory */
    CONTACTS_ERROR_INVALID_PARAMETER    = TIZEN_ERROR_INVALID_PARAMETER,         /**< Invalid parameter */
    CONTACTS_ERROR_FILE_NO_SPACE        = TIZEN_ERROR_FILE_NO_SPACE_ON_DEVICE,   /**< FS Full */
    CONTACTS_ERROR_PERMISSION_DENIED    = TIZEN_ERROR_PERMISSION_DENIED,         /**< Permission denied */
    CONTACTS_ERROR_NOT_SUPPORTED        = TIZEN_ERROR_NOT_SUPPORTED,             /**< Not supported */

    /* LOGIC & DATA */
    CONTACTS_ERROR_NO_DATA	              = TIZEN_ERROR_NO_DATA,                   /**< Requested data does not exist */

    /* DB */
    CONTACTS_ERROR_DB_LOCKED            = TIZEN_ERROR_CONTACTS | 0x81,           /**< Database table locked or file locked */
    CONTACTS_ERROR_DB                   = TIZEN_ERROR_CONTACTS | 0x9F,           /**< Unknown DB error */

    /* IPC */
    CONTACTS_ERROR_IPC_NOT_AVALIABLE    = TIZEN_ERROR_CONTACTS | 0xB1,           /**< IPC server is not available */
    CONTACTS_ERROR_IPC                  = TIZEN_ERROR_CONTACTS | 0xBF,           /**< Unknown IPC error */

    /* ENVIRONMENT & OTHER MODULE */
    /* Socket, inotify, vconf, icu, tapi, security/smack, account and so on */
    CONTACTS_ERROR_SYSTEM               = TIZEN_ERROR_CONTACTS | 0xEF,           /**< Internal system module error */

    /* UNHANDLED EXCEPTION */
    CONTACTS_ERROR_INTERNAL				= TIZEN_ERROR_CONTACTS | 0xFF,            /**< Implementation Error, Temporary Use */
} contacts_error_e;

typedef enum _Ecordova_ContactField_Property
{
   ECORDOVA_CONTACTFIELD_PARENT_PROPERTY_ID,
   ECORDOVA_CONTACTFIELD_PROPERTY_ID,
   ECORDOVA_CONTACTFIELD_PROPERTY_TYPE,
   ECORDOVA_CONTACTFIELD_PROPERTY_LABEL,
   ECORDOVA_CONTACTFIELD_PROPERTY_VALUE,
   ECORDOVA_CONTACTFIELD_PROPERTY_PREF,
   ECORDOVA_CONTACTFIELD_PROPERTY_COUNT
} Ecordova_ContactField_Property;


typedef struct _Ecordova_ContactField_Metadata
{
   const char * const *uri;
   const unsigned int *ids[ECORDOVA_CONTACTFIELD_PROPERTY_COUNT];
   char *(*type2label)(int type, const char *custom);
   int(*label2type)(const char *label);
} Ecordova_ContactField_Metadata;

typedef struct _Ecordova_ContactField_Data Ecordova_ContactField_Data;


#endif


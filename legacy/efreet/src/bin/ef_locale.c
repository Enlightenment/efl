#include "Efreet.h"
#include "efreet_private.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int
ef_cb_locale(void)
{
    int ret = 1, i;
    struct
    {
        char *lc_message;
        char *lang;
        char *country;
        char *modifier;
    } langs[] = {
        /* these are ordered such that when we move from LANG to LC_MESSAGES
         * the LANG env will still be effect. Same with moving from
         * LC_MESSAGES to LANG */
        {"LANG=", NULL, NULL, NULL},
        {"LANG=en", "en", NULL, NULL},
        {"LANG=en@Latn", "en", NULL, "Latn"},
        {"LANG=en_US", "en", "US", NULL},
        {"LANG=en_US@Latn", "en", "US", "Latn"},
        {"LANG=en_US.blah@Latn", "en", "US", "Latn"},
        {"LC_MESSAGES=", "en", "US", "Latn"}, /* This will fallback to LANG */
        {"LC_MESSAGES=fr", "fr", NULL, NULL},
        {"LC_MESSAGES=fr@Blah", "fr", NULL, "Blah"},
        {"LC_MESSAGES=fr_FR", "fr", "FR", NULL},
        {"LC_MESSAGES=fr_FR@Blah", "fr", "FR", "Blah"},
        {"LC_MESSAGES=fr_FR.Foo@Blah", "fr", "FR", "Blah"},
        {"LC_ALL=", "fr", "FR", "Blah"}, /* this will fallback to LC_MESSAGES */
        {"LC_ALL=sr", "sr", NULL, NULL},
        {"LC_ALL=sr@Ret", "sr", NULL, "Ret"},
        {"LC_ALL=sr_YU", "sr", "YU", NULL},
        {"LC_ALL=sr_YU@Ret", "sr", "YU", "Ret"},
        {"LC_ALL=sr_YU.ssh@Ret", "sr", "YU", "Ret"},
        {NULL, NULL, NULL, NULL}
    };

    /* reset everything to blank */
    putenv("LC_ALL=");
    putenv("LC_MESSAGES=");
    putenv("LANG=");

    for (i = 0; langs[i].lc_message != NULL; i++)
    {
        const char *tmp;

        putenv(langs[i].lc_message);

        tmp = efreet_lang_get();
        if ((langs[i].lang && (!tmp || strcmp(tmp, langs[i].lang)))
                || (!langs[i].lang && tmp))
        {
            printf("efreet_lang_get() is wrong (%s) with %s\n",
                                            tmp, langs[i].lang);
            ret = 0;
        }

        tmp = efreet_lang_country_get();
        if ((langs[i].country && (!tmp || strcmp(tmp, langs[i].country)))
                || (!langs[i].country && tmp))
        {
            printf("efreet_lang_country_get() is wrong (%s) with %s\n",
                                                tmp, langs[i].lang);
            ret = 0;
        }

        tmp = efreet_lang_modifier_get();
        if ((langs[i].modifier && (!tmp || strcmp(tmp, langs[i].modifier)))
                || (!langs[i].modifier && tmp))
        {
            printf("efreet_lang_modifier_get() is wrong with %s with %s\n",
                                                tmp, langs[i].lang);
            ret = 0;
        }

        efreet_shutdown();
        efreet_init();
    }

    return ret;
}

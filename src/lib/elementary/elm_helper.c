#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include <regex.h>

struct _Elm_Validator_Regexp
{
   Eina_Stringshare *signal;
   int status;
   regex_t regex;
};

EAPI Elm_Validator_Regexp *
elm_validator_regexp_new(const char *pattern, const char *sig)
{
   Elm_Validator_Regexp *validator;

   validator = calloc(1, sizeof(Elm_Validator_Regexp));
   validator->signal = eina_stringshare_add(sig ? sig : "default");
   validator->status = regcomp(&validator->regex, pattern, REG_EXTENDED | REG_NOSUB) ? ELM_REG_BADPAT : ELM_REG_NOERROR;

   return validator;
}

EAPI void
elm_validator_regexp_free(Elm_Validator_Regexp *validator)
{
   eina_stringshare_del(validator->signal);
   regfree(&validator->regex);
   free(validator);
}

EAPI Elm_Regexp_Status
elm_validator_regexp_status_get(Elm_Validator_Regexp *validator)
{
   return validator->status;
}

EAPI void
elm_validator_regexp_helper(void *data, const Efl_Event *event)
{
   Elm_Validate_Content *vc = event->info;
   Elm_Validator_Regexp *validator = (Elm_Validator_Regexp *)data;

   validator->status = regexec(&validator->regex, vc->text, (size_t)0, NULL, 0) ? ELM_REG_NOMATCH : ELM_REG_NOERROR;
   vc->signal = validator->signal;
   if (validator->status)
      efl_event_callback_stop(event->object);
}

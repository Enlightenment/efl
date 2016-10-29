struct _Efl_Net_Ssl_Ctx {
};

static void *
efl_net_ssl_ctx_connection_new(Efl_Net_Ssl_Ctx *ctx EINA_UNUSED)
{
   return NULL;
}

static Eina_Error
efl_net_ssl_ctx_setup(Efl_Net_Ssl_Ctx *ctx EINA_UNUSED, Efl_Net_Ssl_Ctx_Config cfg EINA_UNUSED)
{
   ERR("EFL compiled with --with-crypto=none");
   return ENOSYS;
}

static void
efl_net_ssl_ctx_teardown(Efl_Net_Ssl_Ctx *ctx EINA_UNUSED)
{
}

static Eina_Error
efl_net_ssl_ctx_verify_mode_set(Efl_Net_Ssl_Ctx *ctx EINA_UNUSED, Efl_Net_Ssl_Verify_Mode verify_mode EINA_UNUSED)
{
   return ENOSYS;
}

static Eina_Error
efl_net_ssl_ctx_hostname_verify_set(Efl_Net_Ssl_Ctx *ctx EINA_UNUSED, Eina_Bool hostname_verify EINA_UNUSED)
{
   return ENOSYS;
}

static Eina_Error
efl_net_ssl_ctx_hostname_set(Efl_Net_Ssl_Ctx *ctx EINA_UNUSED, const char *hostname EINA_UNUSED)
{
   return ENOSYS;
}

struct _Efl_Net_Ssl_Conn {
};

static Eina_Error
efl_net_ssl_conn_setup(Efl_Net_Ssl_Conn *conn EINA_UNUSED, Eina_Bool is_dialer EINA_UNUSED, Efl_Net_Socket *sock EINA_UNUSED, Efl_Net_Ssl_Context *context EINA_UNUSED)
{
   ERR("EFL compiled with --with-crypto=none");
   return ENOSYS;
}

static void
efl_net_ssl_conn_teardown(Efl_Net_Ssl_Conn *conn EINA_UNUSED)
{
}

static Eina_Error
efl_net_ssl_conn_write(Efl_Net_Ssl_Conn *conn EINA_UNUSED, Eina_Slice *slice EINA_UNUSED)
{
   return ENOSYS;
}

static Eina_Error
efl_net_ssl_conn_read(Efl_Net_Ssl_Conn *conn EINA_UNUSED, Eina_Rw_Slice *slice EINA_UNUSED)
{
   return ENOSYS;
}

static Eina_Error
efl_net_ssl_conn_handshake(Efl_Net_Ssl_Conn *conn EINA_UNUSED, Eina_Bool *done EINA_UNUSED)
{
   return ENOSYS;
}

static Eina_Error
efl_net_ssl_conn_verify_mode_set(Efl_Net_Ssl_Conn *conn EINA_UNUSED, Efl_Net_Ssl_Verify_Mode verify_mode EINA_UNUSED)
{
   return ENOSYS;
}

static Eina_Error
efl_net_ssl_conn_hostname_verify_set(Efl_Net_Ssl_Conn *conn EINA_UNUSED, Eina_Bool hostname_verify EINA_UNUSED)
{
   return ENOSYS;
}

static Eina_Error
efl_net_ssl_conn_hostname_override_set(Efl_Net_Ssl_Conn *conn EINA_UNUSED, const char *hostname EINA_UNUSED)
{
   return ENOSYS;
}

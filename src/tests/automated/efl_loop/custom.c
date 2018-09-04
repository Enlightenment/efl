EAPI_MAIN void
efl_main(void *data EINA_UNUSED, const Efl_Event *ev)
{
    efl_loop_quit(ev->object, EINA_VALUE_EMPTY);
}

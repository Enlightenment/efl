master_timeout = 240
# The failsafe clock has to expire before meson stops waiting, or it never
# gets to say which test case was still running. See src/tests/timeout.c.
executable('timeout', 'timeout.c',
           c_args: '-DEFL_TEST_MASTER_TIMEOUT=@0@'.format(master_timeout))

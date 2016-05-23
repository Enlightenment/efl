#ifndef EFL_CXX_THROW

#ifdef EFL_CXX_NO_EXCEPTIONS
# define EFL_CXX_THROW(x)    std::abort()
#else
# define EFL_CXX_THROW(x)    throw x
#endif

#endif

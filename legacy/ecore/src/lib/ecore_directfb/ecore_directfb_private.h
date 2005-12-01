/* macro for a safe call to DirectFB functions */
#define DFBCHECK(x...) \
     {                                                                 \
          _err = x;                                                    \
          if (_err != DFB_OK) {                                        \
               fprintf( stderr, "%s <%d>:\n\t", __FILE__, __LINE__ );  \
               DirectFBErrorFatal( #x, _err );                         \
          }                                                            \
     }

struct keymap
{
	char *name;
	char *string;
};

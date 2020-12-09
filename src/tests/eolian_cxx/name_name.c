
#define EFL_BETA_API_SUPPORT 1
#include <Eo.h>

struct Name_Name_Data {};
typedef struct Name_Name_Data Name_Name_Data;

struct Ns_Name_Data {};
typedef struct Ns_Name_Data Ns_Name_Data;

struct Ns_Name_Other_Data {};
typedef struct Ns_Name_Other_Data Ns_Name_Other_Data;

#define EOLIANCXXTEST_API
#define EOLIANCXXTEST_API_WEAK

#include "name_name.eo.h"
#include "name_name.eo.c"
#include "ns_name.eo.h"
#include "ns_name.eo.c"
#include "ns_name_other.eo.h"
#include "ns_name_other.eo.c"

#include "evas_font_private.h"

#define PROPS_CHANGE(Props) Props->changed = EINA_TRUE;


/* X11 colors
 * https://gitlab.freedesktop.org/xorg/xserver/blob/master/os/oscolor.c
*/

typedef struct _Color_Name_Value Color_Name_Value;

struct _Color_Name_Value
{
   char *name;
   /*color name will not exceed 255 byte*/
   unsigned char name_len;
   unsigned char r;
   unsigned char g;
   unsigned char b;
};

#define COLOR_NAME(name,r,g,b) {name,(unsigned char)sizeof(name)-1,r,g,b}

/**
 * @internal
 * @var color_name_value_sorted[]
 * This array consists of Color Name Value structure sorted by name string
 * And new added value must be placed sorted position
 */

static const Color_Name_Value color_name_value_sorted[] = {
   COLOR_NAME("aliceblue",240,248,255),
   COLOR_NAME("antiquewhite",250,235,215),
   COLOR_NAME("antiquewhite1",255,239,219),
   COLOR_NAME("antiquewhite2",238,223,204),
   COLOR_NAME("antiquewhite3",205,192,176),
   COLOR_NAME("antiquewhite4",139,131,120),
   COLOR_NAME("aqua",0,255,255),
   COLOR_NAME("aquamarine",127,255,212),
   COLOR_NAME("aquamarine1",127,255,212),
   COLOR_NAME("aquamarine2",118,238,198),
   COLOR_NAME("aquamarine3",102,205,170),
   COLOR_NAME("aquamarine4",69,139,116),
   COLOR_NAME("azure",240,255,255),
   COLOR_NAME("azure1",240,255,255),
   COLOR_NAME("azure2",224,238,238),
   COLOR_NAME("azure3",193,205,205),
   COLOR_NAME("azure4",131,139,139),
   COLOR_NAME("beige",245,245,220),
   COLOR_NAME("bisque",255,228,196),
   COLOR_NAME("bisque1",255,228,196),
   COLOR_NAME("bisque2",238,213,183),
   COLOR_NAME("bisque3",205,183,158),
   COLOR_NAME("bisque4",139,125,107),
   COLOR_NAME("black",0,0,0),
   COLOR_NAME("blanchedalmond",255,235,205),
   COLOR_NAME("blue",0,0,255),
   COLOR_NAME("blueviolet",138,43,226),
   COLOR_NAME("blue1",0,0,255),
   COLOR_NAME("blue2",0,0,238),
   COLOR_NAME("blue3",0,0,205),
   COLOR_NAME("blue4",0,0,139),
   COLOR_NAME("brown",165,42,42),
   COLOR_NAME("brown1",255,64,64),
   COLOR_NAME("brown2",238,59,59),
   COLOR_NAME("brown3",205,51,51),
   COLOR_NAME("brown4",139,35,35),
   COLOR_NAME("burlywood",222,184,135),
   COLOR_NAME("burlywood1",255,211,155),
   COLOR_NAME("burlywood2",238,197,145),
   COLOR_NAME("burlywood3",205,170,125),
   COLOR_NAME("burlywood4",139,115,85),
   COLOR_NAME("cadetblue",95,158,160),
   COLOR_NAME("cadetblue1",152,245,255),
   COLOR_NAME("cadetblue2",142,229,238),
   COLOR_NAME("cadetblue3",122,197,205),
   COLOR_NAME("cadetblue4",83,134,139),
   COLOR_NAME("chartreuse",127,255,0),
   COLOR_NAME("chartreuse1",127,255,0),
   COLOR_NAME("chartreuse2",118,238,0),
   COLOR_NAME("chartreuse3",102,205,0),
   COLOR_NAME("chartreuse4",69,139,0),
   COLOR_NAME("chocolate",210,105,30),
   COLOR_NAME("chocolate1",255,127,36),
   COLOR_NAME("chocolate2",238,118,33),
   COLOR_NAME("chocolate3",205,102,29),
   COLOR_NAME("chocolate4",139,69,19),
   COLOR_NAME("coral",255,127,80),
   COLOR_NAME("coral1",255,114,86),
   COLOR_NAME("coral2",238,106,80),
   COLOR_NAME("coral3",205,91,69),
   COLOR_NAME("coral4",139,62,47),
   COLOR_NAME("cornflowerblue",100,149,237),
   COLOR_NAME("cornsilk",255,248,220),
   COLOR_NAME("cornsilk1",255,248,220),
   COLOR_NAME("cornsilk2",238,232,205),
   COLOR_NAME("cornsilk3",205,200,177),
   COLOR_NAME("cornsilk4",139,136,120),
   COLOR_NAME("crimson",220,20,60),
   COLOR_NAME("cyan",0,255,255),
   COLOR_NAME("cyan1",0,255,255),
   COLOR_NAME("cyan2",0,238,238),
   COLOR_NAME("cyan3",0,205,205),
   COLOR_NAME("cyan4",0,139,139),
   COLOR_NAME("darkblue",0,0,139),
   COLOR_NAME("darkcyan",0,139,139),
   COLOR_NAME("darkgoldenrod",184,134,11),
   COLOR_NAME("darkgray",169,169,169),
   COLOR_NAME("darkgreen",0,100,0),
   COLOR_NAME("darkgrey",169,169,169),
   COLOR_NAME("darkkhaki",189,183,107),
   COLOR_NAME("darkmagenta",139,0,139),
   COLOR_NAME("darkolivegreen",85,107,47),
   COLOR_NAME("darkorange",255,140,0),
   COLOR_NAME("darkorchid",153,50,204),
   COLOR_NAME("darkred",139,0,0),
   COLOR_NAME("darksalmon",233,150,122),
   COLOR_NAME("darkseagreen",143,188,143),
   COLOR_NAME("darkslateblue",72,61,139),
   COLOR_NAME("darkslategray",47,79,79),
   COLOR_NAME("darkslategrey",47,79,79),
   COLOR_NAME("darkturquoise",0,206,209),
   COLOR_NAME("darkviolet",148,0,211),
   COLOR_NAME("darkgoldenrod1",255,185,15),
   COLOR_NAME("darkgoldenrod2",238,173,14),
   COLOR_NAME("darkgoldenrod3",205,149,12),
   COLOR_NAME("darkgoldenrod4",139,101,8),
   COLOR_NAME("darkolivegreen1",202,255,112),
   COLOR_NAME("darkolivegreen2",188,238,104),
   COLOR_NAME("darkolivegreen3",162,205,90),
   COLOR_NAME("darkolivegreen4",110,139,61),
   COLOR_NAME("darkorange1",255,127,0),
   COLOR_NAME("darkorange2",238,118,0),
   COLOR_NAME("darkorange3",205,102,0),
   COLOR_NAME("darkorange4",139,69,0),
   COLOR_NAME("darkorchid1",191,62,255),
   COLOR_NAME("darkorchid2",178,58,238),
   COLOR_NAME("darkorchid3",154,50,205),
   COLOR_NAME("darkorchid4",104,34,139),
   COLOR_NAME("darkseagreen1",193,255,193),
   COLOR_NAME("darkseagreen2",180,238,180),
   COLOR_NAME("darkseagreen3",155,205,155),
   COLOR_NAME("darkseagreen4",105,139,105),
   COLOR_NAME("darkslategray1",151,255,255),
   COLOR_NAME("darkslategray2",141,238,238),
   COLOR_NAME("darkslategray3",121,205,205),
   COLOR_NAME("darkslategray4",82,139,139),
   COLOR_NAME("deeppink",255,20,147),
   COLOR_NAME("deepskyblue",0,191,255),
   COLOR_NAME("deeppink1",255,20,147),
   COLOR_NAME("deeppink2",238,18,137),
   COLOR_NAME("deeppink3",205,16,118),
   COLOR_NAME("deeppink4",139,10,80),
   COLOR_NAME("deepskyblue1",0,191,255),
   COLOR_NAME("deepskyblue2",0,178,238),
   COLOR_NAME("deepskyblue3",0,154,205),
   COLOR_NAME("deepskyblue4",0,104,139),
   COLOR_NAME("dimgray",105,105,105),
   COLOR_NAME("dimgrey",105,105,105),
   COLOR_NAME("dodgerblue",30,144,255),
   COLOR_NAME("dodgerblue1",30,144,255),
   COLOR_NAME("dodgerblue2",28,134,238),
   COLOR_NAME("dodgerblue3",24,116,205),
   COLOR_NAME("dodgerblue4",16,78,139),
   COLOR_NAME("firebrick",178,34,34),
   COLOR_NAME("firebrick1",255,48,48),
   COLOR_NAME("firebrick2",238,44,44),
   COLOR_NAME("firebrick3",205,38,38),
   COLOR_NAME("firebrick4",139,26,26),
   COLOR_NAME("floralwhite",255,250,240),
   COLOR_NAME("forestgreen",34,139,34),
   COLOR_NAME("fuchsia",255,0,255),
   COLOR_NAME("gainsboro",220,220,220),
   COLOR_NAME("ghostwhite",248,248,255),
   COLOR_NAME("gold",255,215,0),
   COLOR_NAME("gold1",255,215,0),
   COLOR_NAME("gold2",238,201,0),
   COLOR_NAME("gold3",205,173,0),
   COLOR_NAME("gold4",139,117,0),
   COLOR_NAME("goldenrod",218,165,32),
   COLOR_NAME("goldenrod1",255,193,37),
   COLOR_NAME("goldenrod2",238,180,34),
   COLOR_NAME("goldenrod3",205,155,29),
   COLOR_NAME("goldenrod4",139,105,20),
   COLOR_NAME("gray",190,190,190),
   COLOR_NAME("gray0",0,0,0),
   COLOR_NAME("gray1",3,3,3),
   COLOR_NAME("gray10",26,26,26),
   COLOR_NAME("gray100",255,255,255),
   COLOR_NAME("gray11",28,28,28),
   COLOR_NAME("gray12",31,31,31),
   COLOR_NAME("gray13",33,33,33),
   COLOR_NAME("gray14",36,36,36),
   COLOR_NAME("gray15",38,38,38),
   COLOR_NAME("gray16",41,41,41),
   COLOR_NAME("gray17",43,43,43),
   COLOR_NAME("gray18",46,46,46),
   COLOR_NAME("gray19",48,48,48),
   COLOR_NAME("gray2",5,5,5),
   COLOR_NAME("gray20",51,51,51),
   COLOR_NAME("gray21",54,54,54),
   COLOR_NAME("gray22",56,56,56),
   COLOR_NAME("gray23",59,59,59),
   COLOR_NAME("gray24",61,61,61),
   COLOR_NAME("gray25",64,64,64),
   COLOR_NAME("gray26",66,66,66),
   COLOR_NAME("gray27",69,69,69),
   COLOR_NAME("gray28",71,71,71),
   COLOR_NAME("gray29",74,74,74),
   COLOR_NAME("gray3",8,8,8),
   COLOR_NAME("gray30",77,77,77),
   COLOR_NAME("gray31",79,79,79),
   COLOR_NAME("gray32",82,82,82),
   COLOR_NAME("gray33",84,84,84),
   COLOR_NAME("gray34",87,87,87),
   COLOR_NAME("gray35",89,89,89),
   COLOR_NAME("gray36",92,92,92),
   COLOR_NAME("gray37",94,94,94),
   COLOR_NAME("gray38",97,97,97),
   COLOR_NAME("gray39",99,99,99),
   COLOR_NAME("gray4",10,10,10),
   COLOR_NAME("gray40",102,102,102),
   COLOR_NAME("gray41",105,105,105),
   COLOR_NAME("gray42",107,107,107),
   COLOR_NAME("gray43",110,110,110),
   COLOR_NAME("gray44",112,112,112),
   COLOR_NAME("gray45",115,115,115),
   COLOR_NAME("gray46",117,117,117),
   COLOR_NAME("gray47",120,120,120),
   COLOR_NAME("gray48",122,122,122),
   COLOR_NAME("gray49",125,125,125),
   COLOR_NAME("gray5",13,13,13),
   COLOR_NAME("gray50",127,127,127),
   COLOR_NAME("gray51",130,130,130),
   COLOR_NAME("gray52",133,133,133),
   COLOR_NAME("gray53",135,135,135),
   COLOR_NAME("gray54",138,138,138),
   COLOR_NAME("gray55",140,140,140),
   COLOR_NAME("gray56",143,143,143),
   COLOR_NAME("gray57",145,145,145),
   COLOR_NAME("gray58",148,148,148),
   COLOR_NAME("gray59",150,150,150),
   COLOR_NAME("gray6",15,15,15),
   COLOR_NAME("gray60",153,153,153),
   COLOR_NAME("gray61",156,156,156),
   COLOR_NAME("gray62",158,158,158),
   COLOR_NAME("gray63",161,161,161),
   COLOR_NAME("gray64",163,163,163),
   COLOR_NAME("gray65",166,166,166),
   COLOR_NAME("gray66",168,168,168),
   COLOR_NAME("gray67",171,171,171),
   COLOR_NAME("gray68",173,173,173),
   COLOR_NAME("gray69",176,176,176),
   COLOR_NAME("gray7",18,18,18),
   COLOR_NAME("gray70",179,179,179),
   COLOR_NAME("gray71",181,181,181),
   COLOR_NAME("gray72",184,184,184),
   COLOR_NAME("gray73",186,186,186),
   COLOR_NAME("gray74",189,189,189),
   COLOR_NAME("gray75",191,191,191),
   COLOR_NAME("gray76",194,194,194),
   COLOR_NAME("gray77",196,196,196),
   COLOR_NAME("gray78",199,199,199),
   COLOR_NAME("gray79",201,201,201),
   COLOR_NAME("gray8",20,20,20),
   COLOR_NAME("gray80",204,204,204),
   COLOR_NAME("gray81",207,207,207),
   COLOR_NAME("gray82",209,209,209),
   COLOR_NAME("gray83",212,212,212),
   COLOR_NAME("gray84",214,214,214),
   COLOR_NAME("gray85",217,217,217),
   COLOR_NAME("gray86",219,219,219),
   COLOR_NAME("gray87",222,222,222),
   COLOR_NAME("gray88",224,224,224),
   COLOR_NAME("gray89",227,227,227),
   COLOR_NAME("gray9",23,23,23),
   COLOR_NAME("gray90",229,229,229),
   COLOR_NAME("gray91",232,232,232),
   COLOR_NAME("gray92",235,235,235),
   COLOR_NAME("gray93",237,237,237),
   COLOR_NAME("gray94",240,240,240),
   COLOR_NAME("gray95",242,242,242),
   COLOR_NAME("gray96",245,245,245),
   COLOR_NAME("gray97",247,247,247),
   COLOR_NAME("gray98",250,250,250),
   COLOR_NAME("gray99",252,252,252),
   COLOR_NAME("green",0,255,0),
   COLOR_NAME("greenyellow",173,255,47),
   COLOR_NAME("green1",0,255,0),
   COLOR_NAME("green2",0,238,0),
   COLOR_NAME("green3",0,205,0),
   COLOR_NAME("green4",0,139,0),
   COLOR_NAME("grey",190,190,190),
   COLOR_NAME("grey0",0,0,0),
   COLOR_NAME("grey1",3,3,3),
   COLOR_NAME("grey10",26,26,26),
   COLOR_NAME("grey100",255,255,255),
   COLOR_NAME("grey11",28,28,28),
   COLOR_NAME("grey12",31,31,31),
   COLOR_NAME("grey13",33,33,33),
   COLOR_NAME("grey14",36,36,36),
   COLOR_NAME("grey15",38,38,38),
   COLOR_NAME("grey16",41,41,41),
   COLOR_NAME("grey17",43,43,43),
   COLOR_NAME("grey18",46,46,46),
   COLOR_NAME("grey19",48,48,48),
   COLOR_NAME("grey2",5,5,5),
   COLOR_NAME("grey20",51,51,51),
   COLOR_NAME("grey21",54,54,54),
   COLOR_NAME("grey22",56,56,56),
   COLOR_NAME("grey23",59,59,59),
   COLOR_NAME("grey24",61,61,61),
   COLOR_NAME("grey25",64,64,64),
   COLOR_NAME("grey26",66,66,66),
   COLOR_NAME("grey27",69,69,69),
   COLOR_NAME("grey28",71,71,71),
   COLOR_NAME("grey29",74,74,74),
   COLOR_NAME("grey3",8,8,8),
   COLOR_NAME("grey30",77,77,77),
   COLOR_NAME("grey31",79,79,79),
   COLOR_NAME("grey32",82,82,82),
   COLOR_NAME("grey33",84,84,84),
   COLOR_NAME("grey34",87,87,87),
   COLOR_NAME("grey35",89,89,89),
   COLOR_NAME("grey36",92,92,92),
   COLOR_NAME("grey37",94,94,94),
   COLOR_NAME("grey38",97,97,97),
   COLOR_NAME("grey39",99,99,99),
   COLOR_NAME("grey4",10,10,10),
   COLOR_NAME("grey40",102,102,102),
   COLOR_NAME("grey41",105,105,105),
   COLOR_NAME("grey42",107,107,107),
   COLOR_NAME("grey43",110,110,110),
   COLOR_NAME("grey44",112,112,112),
   COLOR_NAME("grey45",115,115,115),
   COLOR_NAME("grey46",117,117,117),
   COLOR_NAME("grey47",120,120,120),
   COLOR_NAME("grey48",122,122,122),
   COLOR_NAME("grey49",125,125,125),
   COLOR_NAME("grey5",13,13,13),
   COLOR_NAME("grey50",127,127,127),
   COLOR_NAME("grey51",130,130,130),
   COLOR_NAME("grey52",133,133,133),
   COLOR_NAME("grey53",135,135,135),
   COLOR_NAME("grey54",138,138,138),
   COLOR_NAME("grey55",140,140,140),
   COLOR_NAME("grey56",143,143,143),
   COLOR_NAME("grey57",145,145,145),
   COLOR_NAME("grey58",148,148,148),
   COLOR_NAME("grey59",150,150,150),
   COLOR_NAME("grey6",15,15,15),
   COLOR_NAME("grey60",153,153,153),
   COLOR_NAME("grey61",156,156,156),
   COLOR_NAME("grey62",158,158,158),
   COLOR_NAME("grey63",161,161,161),
   COLOR_NAME("grey64",163,163,163),
   COLOR_NAME("grey65",166,166,166),
   COLOR_NAME("grey66",168,168,168),
   COLOR_NAME("grey67",171,171,171),
   COLOR_NAME("grey68",173,173,173),
   COLOR_NAME("grey69",176,176,176),
   COLOR_NAME("grey7",18,18,18),
   COLOR_NAME("grey70",179,179,179),
   COLOR_NAME("grey71",181,181,181),
   COLOR_NAME("grey72",184,184,184),
   COLOR_NAME("grey73",186,186,186),
   COLOR_NAME("grey74",189,189,189),
   COLOR_NAME("grey75",191,191,191),
   COLOR_NAME("grey76",194,194,194),
   COLOR_NAME("grey77",196,196,196),
   COLOR_NAME("grey78",199,199,199),
   COLOR_NAME("grey79",201,201,201),
   COLOR_NAME("grey8",20,20,20),
   COLOR_NAME("grey80",204,204,204),
   COLOR_NAME("grey81",207,207,207),
   COLOR_NAME("grey82",209,209,209),
   COLOR_NAME("grey83",212,212,212),
   COLOR_NAME("grey84",214,214,214),
   COLOR_NAME("grey85",217,217,217),
   COLOR_NAME("grey86",219,219,219),
   COLOR_NAME("grey87",222,222,222),
   COLOR_NAME("grey88",224,224,224),
   COLOR_NAME("grey89",227,227,227),
   COLOR_NAME("grey9",23,23,23),
   COLOR_NAME("grey90",229,229,229),
   COLOR_NAME("grey91",232,232,232),
   COLOR_NAME("grey92",235,235,235),
   COLOR_NAME("grey93",237,237,237),
   COLOR_NAME("grey94",240,240,240),
   COLOR_NAME("grey95",242,242,242),
   COLOR_NAME("grey96",245,245,245),
   COLOR_NAME("grey97",247,247,247),
   COLOR_NAME("grey98",250,250,250),
   COLOR_NAME("grey99",252,252,252),
   COLOR_NAME("honeydew",240,255,240),
   COLOR_NAME("honeydew1",240,255,240),
   COLOR_NAME("honeydew2",224,238,224),
   COLOR_NAME("honeydew3",193,205,193),
   COLOR_NAME("honeydew4",131,139,131),
   COLOR_NAME("hotpink",255,105,180),
   COLOR_NAME("hotpink1",255,110,180),
   COLOR_NAME("hotpink2",238,106,167),
   COLOR_NAME("hotpink3",205,96,144),
   COLOR_NAME("hotpink4",139,58,98),
   COLOR_NAME("indianred",205,92,92),
   COLOR_NAME("indianred1",255,106,106),
   COLOR_NAME("indianred2",238,99,99),
   COLOR_NAME("indianred3",205,85,85),
   COLOR_NAME("indianred4",139,58,58),
   COLOR_NAME("indigo",75,0,130),
   COLOR_NAME("ivory",255,255,240),
   COLOR_NAME("ivory1",255,255,240),
   COLOR_NAME("ivory2",238,238,224),
   COLOR_NAME("ivory3",205,205,193),
   COLOR_NAME("ivory4",139,139,131),
   COLOR_NAME("khaki",240,230,140),
   COLOR_NAME("khaki1",255,246,143),
   COLOR_NAME("khaki2",238,230,133),
   COLOR_NAME("khaki3",205,198,115),
   COLOR_NAME("khaki4",139,134,78),
   COLOR_NAME("lavender",230,230,250),
   COLOR_NAME("lavenderblush",255,240,245),
   COLOR_NAME("lavenderblush1",255,240,245),
   COLOR_NAME("lavenderblush2",238,224,229),
   COLOR_NAME("lavenderblush3",205,193,197),
   COLOR_NAME("lavenderblush4",139,131,134),
   COLOR_NAME("lawngreen",124,252,0),
   COLOR_NAME("lemonchiffon",255,250,205),
   COLOR_NAME("lemonchiffon1",255,250,205),
   COLOR_NAME("lemonchiffon2",238,233,191),
   COLOR_NAME("lemonchiffon3",205,201,165),
   COLOR_NAME("lemonchiffon4",139,137,112),
   COLOR_NAME("lightblue",173,216,230),
   COLOR_NAME("lightcoral",240,128,128),
   COLOR_NAME("lightcyan",224,255,255),
   COLOR_NAME("lightgoldenrod",238,221,130),
   COLOR_NAME("lightgoldenrodyellow",250,250,210),
   COLOR_NAME("lightgray",211,211,211),
   COLOR_NAME("lightgreen",144,238,144),
   COLOR_NAME("lightgrey",211,211,211),
   COLOR_NAME("lightpink",255,182,193),
   COLOR_NAME("lightsalmon",255,160,122),
   COLOR_NAME("lightseagreen",32,178,170),
   COLOR_NAME("lightskyblue",135,206,250),
   COLOR_NAME("lightslateblue",132,112,255),
   COLOR_NAME("lightslategray",119,136,153),
   COLOR_NAME("lightslategrey",119,136,153),
   COLOR_NAME("lightsteelblue",176,196,222),
   COLOR_NAME("lightyellow",255,255,224),
   COLOR_NAME("lightblue1",191,239,255),
   COLOR_NAME("lightblue2",178,223,238),
   COLOR_NAME("lightblue3",154,192,205),
   COLOR_NAME("lightblue4",104,131,139),
   COLOR_NAME("lightcyan1",224,255,255),
   COLOR_NAME("lightcyan2",209,238,238),
   COLOR_NAME("lightcyan3",180,205,205),
   COLOR_NAME("lightcyan4",122,139,139),
   COLOR_NAME("lightgoldenrod1",255,236,139),
   COLOR_NAME("lightgoldenrod2",238,220,130),
   COLOR_NAME("lightgoldenrod3",205,190,112),
   COLOR_NAME("lightgoldenrod4",139,129,76),
   COLOR_NAME("lightpink1",255,174,185),
   COLOR_NAME("lightpink2",238,162,173),
   COLOR_NAME("lightpink3",205,140,149),
   COLOR_NAME("lightpink4",139,95,101),
   COLOR_NAME("lightsalmon1",255,160,122),
   COLOR_NAME("lightsalmon2",238,149,114),
   COLOR_NAME("lightsalmon3",205,129,98),
   COLOR_NAME("lightsalmon4",139,87,66),
   COLOR_NAME("lightskyblue1",176,226,255),
   COLOR_NAME("lightskyblue2",164,211,238),
   COLOR_NAME("lightskyblue3",141,182,205),
   COLOR_NAME("lightskyblue4",96,123,139),
   COLOR_NAME("lightsteelblue1",202,225,255),
   COLOR_NAME("lightsteelblue2",188,210,238),
   COLOR_NAME("lightsteelblue3",162,181,205),
   COLOR_NAME("lightsteelblue4",110,123,139),
   COLOR_NAME("lightyellow1",255,255,224),
   COLOR_NAME("lightyellow2",238,238,209),
   COLOR_NAME("lightyellow3",205,205,180),
   COLOR_NAME("lightyellow4",139,139,122),
   COLOR_NAME("lime",0,255,0),
   COLOR_NAME("limegreen",50,205,50),
   COLOR_NAME("linen",250,240,230),
   COLOR_NAME("magenta",255,0,255),
   COLOR_NAME("magenta1",255,0,255),
   COLOR_NAME("magenta2",238,0,238),
   COLOR_NAME("magenta3",205,0,205),
   COLOR_NAME("magenta4",139,0,139),
   COLOR_NAME("maroon",176,48,96),
   COLOR_NAME("maroon1",255,52,179),
   COLOR_NAME("maroon2",238,48,167),
   COLOR_NAME("maroon3",205,41,144),
   COLOR_NAME("maroon4",139,28,98),
   COLOR_NAME("mediumaquamarine",102,205,170),
   COLOR_NAME("mediumblue",0,0,205),
   COLOR_NAME("mediumorchid",186,85,211),
   COLOR_NAME("mediumpurple",147,112,219),
   COLOR_NAME("mediumseagreen",60,179,113),
   COLOR_NAME("mediumslateblue",123,104,238),
   COLOR_NAME("mediumspringgreen",0,250,154),
   COLOR_NAME("mediumturquoise",72,209,204),
   COLOR_NAME("mediumvioletred",199,21,133),
   COLOR_NAME("mediumorchid1",224,102,255),
   COLOR_NAME("mediumorchid2",209,95,238),
   COLOR_NAME("mediumorchid3",180,82,205),
   COLOR_NAME("mediumorchid4",122,55,139),
   COLOR_NAME("mediumpurple1",171,130,255),
   COLOR_NAME("mediumpurple2",159,121,238),
   COLOR_NAME("mediumpurple3",137,104,205),
   COLOR_NAME("mediumpurple4",93,71,139),
   COLOR_NAME("midnightblue",25,25,112),
   COLOR_NAME("mintcream",245,255,250),
   COLOR_NAME("mistyrose",255,228,225),
   COLOR_NAME("mistyrose1",255,228,225),
   COLOR_NAME("mistyrose2",238,213,210),
   COLOR_NAME("mistyrose3",205,183,181),
   COLOR_NAME("mistyrose4",139,125,123),
   COLOR_NAME("moccasin",255,228,181),
   COLOR_NAME("navajowhite",255,222,173),
   COLOR_NAME("navajowhite1",255,222,173),
   COLOR_NAME("navajowhite2",238,207,161),
   COLOR_NAME("navajowhite3",205,179,139),
   COLOR_NAME("navajowhite4",139,121,94),
   COLOR_NAME("navy",0,0,128),
   COLOR_NAME("navyblue",0,0,128),
   COLOR_NAME("oldlace",253,245,230),
   COLOR_NAME("olive",128,128,0),
   COLOR_NAME("olivedrab",107,142,35),
   COLOR_NAME("olivedrab1",192,255,62),
   COLOR_NAME("olivedrab2",179,238,58),
   COLOR_NAME("olivedrab3",154,205,50),
   COLOR_NAME("olivedrab4",105,139,34),
   COLOR_NAME("orange",255,165,0),
   COLOR_NAME("orangered",255,69,0),
   COLOR_NAME("orange1",255,165,0),
   COLOR_NAME("orange2",238,154,0),
   COLOR_NAME("orange3",205,133,0),
   COLOR_NAME("orange4",139,90,0),
   COLOR_NAME("orangered1",255,69,0),
   COLOR_NAME("orangered2",238,64,0),
   COLOR_NAME("orangered3",205,55,0),
   COLOR_NAME("orangered4",139,37,0),
   COLOR_NAME("orchid",218,112,214),
   COLOR_NAME("orchid1",255,131,250),
   COLOR_NAME("orchid2",238,122,233),
   COLOR_NAME("orchid3",205,105,201),
   COLOR_NAME("orchid4",139,71,137),
   COLOR_NAME("palegoldenrod",238,232,170),
   COLOR_NAME("palegreen",152,251,152),
   COLOR_NAME("paleturquoise",175,238,238),
   COLOR_NAME("palevioletred",219,112,147),
   COLOR_NAME("palegreen1",154,255,154),
   COLOR_NAME("palegreen2",144,238,144),
   COLOR_NAME("palegreen3",124,205,124),
   COLOR_NAME("palegreen4",84,139,84),
   COLOR_NAME("paleturquoise1",187,255,255),
   COLOR_NAME("paleturquoise2",174,238,238),
   COLOR_NAME("paleturquoise3",150,205,205),
   COLOR_NAME("paleturquoise4",102,139,139),
   COLOR_NAME("palevioletred1",255,130,171),
   COLOR_NAME("palevioletred2",238,121,159),
   COLOR_NAME("palevioletred3",205,104,137),
   COLOR_NAME("palevioletred4",139,71,93),
   COLOR_NAME("papayawhip",255,239,213),
   COLOR_NAME("peachpuff",255,218,185),
   COLOR_NAME("peachpuff1",255,218,185),
   COLOR_NAME("peachpuff2",238,203,173),
   COLOR_NAME("peachpuff3",205,175,149),
   COLOR_NAME("peachpuff4",139,119,101),
   COLOR_NAME("peru",205,133,63),
   COLOR_NAME("pink",255,192,203),
   COLOR_NAME("pink1",255,181,197),
   COLOR_NAME("pink2",238,169,184),
   COLOR_NAME("pink3",205,145,158),
   COLOR_NAME("pink4",139,99,108),
   COLOR_NAME("plum",221,160,221),
   COLOR_NAME("plum1",255,187,255),
   COLOR_NAME("plum2",238,174,238),
   COLOR_NAME("plum3",205,150,205),
   COLOR_NAME("plum4",139,102,139),
   COLOR_NAME("powderblue",176,224,230),
   COLOR_NAME("purple",160,32,240),
   COLOR_NAME("purple1",155,48,255),
   COLOR_NAME("purple2",145,44,238),
   COLOR_NAME("purple3",125,38,205),
   COLOR_NAME("purple4",85,26,139),
   COLOR_NAME("rebeccapurple",102,51,153),
   COLOR_NAME("red",255,0,0),
   COLOR_NAME("red1",255,0,0),
   COLOR_NAME("red2",238,0,0),
   COLOR_NAME("red3",205,0,0),
   COLOR_NAME("red4",139,0,0),
   COLOR_NAME("rosybrown",188,143,143),
   COLOR_NAME("rosybrown1",255,193,193),
   COLOR_NAME("rosybrown2",238,180,180),
   COLOR_NAME("rosybrown3",205,155,155),
   COLOR_NAME("rosybrown4",139,105,105),
   COLOR_NAME("royalblue",65,105,225),
   COLOR_NAME("royalblue1",72,118,255),
   COLOR_NAME("royalblue2",67,110,238),
   COLOR_NAME("royalblue3",58,95,205),
   COLOR_NAME("royalblue4",39,64,139),
   COLOR_NAME("saddlebrown",139,69,19),
   COLOR_NAME("salmon",250,128,114),
   COLOR_NAME("salmon1",255,140,105),
   COLOR_NAME("salmon2",238,130,98),
   COLOR_NAME("salmon3",205,112,84),
   COLOR_NAME("salmon4",139,76,57),
   COLOR_NAME("sandybrown",244,164,96),
   COLOR_NAME("seagreen",46,139,87),
   COLOR_NAME("seagreen1",84,255,159),
   COLOR_NAME("seagreen2",78,238,148),
   COLOR_NAME("seagreen3",67,205,128),
   COLOR_NAME("seagreen4",46,139,87),
   COLOR_NAME("seashell",255,245,238),
   COLOR_NAME("seashell1",255,245,238),
   COLOR_NAME("seashell2",238,229,222),
   COLOR_NAME("seashell3",205,197,191),
   COLOR_NAME("seashell4",139,134,130),
   COLOR_NAME("sienna",160,82,45),
   COLOR_NAME("sienna1",255,130,71),
   COLOR_NAME("sienna2",238,121,66),
   COLOR_NAME("sienna3",205,104,57),
   COLOR_NAME("sienna4",139,71,38),
   COLOR_NAME("silver",192,192,192),
   COLOR_NAME("skyblue",135,206,235),
   COLOR_NAME("skyblue1",135,206,255),
   COLOR_NAME("skyblue2",126,192,238),
   COLOR_NAME("skyblue3",108,166,205),
   COLOR_NAME("skyblue4",74,112,139),
   COLOR_NAME("slateblue",106,90,205),
   COLOR_NAME("slategray",112,128,144),
   COLOR_NAME("slategrey",112,128,144),
   COLOR_NAME("slateblue1",131,111,255),
   COLOR_NAME("slateblue2",122,103,238),
   COLOR_NAME("slateblue3",105,89,205),
   COLOR_NAME("slateblue4",71,60,139),
   COLOR_NAME("slategray1",198,226,255),
   COLOR_NAME("slategray2",185,211,238),
   COLOR_NAME("slategray3",159,182,205),
   COLOR_NAME("slategray4",108,123,139),
   COLOR_NAME("snow",255,250,250),
   COLOR_NAME("snow1",255,250,250),
   COLOR_NAME("snow2",238,233,233),
   COLOR_NAME("snow3",205,201,201),
   COLOR_NAME("snow4",139,137,137),
   COLOR_NAME("springgreen",0,255,127),
   COLOR_NAME("springgreen1",0,255,127),
   COLOR_NAME("springgreen2",0,238,118),
   COLOR_NAME("springgreen3",0,205,102),
   COLOR_NAME("springgreen4",0,139,69),
   COLOR_NAME("steelblue",70,130,180),
   COLOR_NAME("steelblue1",99,184,255),
   COLOR_NAME("steelblue2",92,172,238),
   COLOR_NAME("steelblue3",79,148,205),
   COLOR_NAME("steelblue4",54,100,139),
   COLOR_NAME("tan",210,180,140),
   COLOR_NAME("tan1",255,165,79),
   COLOR_NAME("tan2",238,154,73),
   COLOR_NAME("tan3",205,133,63),
   COLOR_NAME("tan4",139,90,43),
   COLOR_NAME("teal",0,128,128),
   COLOR_NAME("thistle",216,191,216),
   COLOR_NAME("thistle1",255,225,255),
   COLOR_NAME("thistle2",238,210,238),
   COLOR_NAME("thistle3",205,181,205),
   COLOR_NAME("thistle4",139,123,139),
   COLOR_NAME("tomato",255,99,71),
   COLOR_NAME("tomato1",255,99,71),
   COLOR_NAME("tomato2",238,92,66),
   COLOR_NAME("tomato3",205,79,57),
   COLOR_NAME("tomato4",139,54,38),
   COLOR_NAME("turquoise",64,224,208),
   COLOR_NAME("turquoise1",0,245,255),
   COLOR_NAME("turquoise2",0,229,238),
   COLOR_NAME("turquoise3",0,197,205),
   COLOR_NAME("turquoise4",0,134,139),
   COLOR_NAME("violet",238,130,238),
   COLOR_NAME("violetred",208,32,144),
   COLOR_NAME("violetred1",255,62,150),
   COLOR_NAME("violetred2",238,58,140),
   COLOR_NAME("violetred3",205,50,120),
   COLOR_NAME("violetred4",139,34,82),
   COLOR_NAME("webgray",128,128,128),
   COLOR_NAME("webgreen",0,128,0),
   COLOR_NAME("webgrey",128,128,128),
   COLOR_NAME("webmaroon",128,0,0),
   COLOR_NAME("webpurple",128,0,128),
   COLOR_NAME("wheat",245,222,179),
   COLOR_NAME("wheat1",255,231,186),
   COLOR_NAME("wheat2",238,216,174),
   COLOR_NAME("wheat3",205,186,150),
   COLOR_NAME("wheat4",139,126,102),
   COLOR_NAME("white",255,255,255),
   COLOR_NAME("whitesmoke",245,245,245),
   COLOR_NAME("x11gray",190,190,190),
   COLOR_NAME("x11green",0,255,0),
   COLOR_NAME("x11grey",190,190,190),
   COLOR_NAME("x11maroon",176,48,96),
   COLOR_NAME("x11purple",160,32,240),
   COLOR_NAME("yellow",255,255,0),
   COLOR_NAME("yellowgreen",154,205,50),
   COLOR_NAME("yellow1",255,255,0),
   COLOR_NAME("yellow2",238,238,0),
   COLOR_NAME("yellow3",205,205,0),
   COLOR_NAME("yellow4",139,139,0),
};

int _color_name_search(const void * value, const void * element)
{
   return strcmp((char *) value, ((const Color_Name_Value *) element)->name);
}

void
evas_common_text_props_bidi_set(Evas_Text_Props *props,
      Evas_BiDi_Paragraph_Props *bidi_par_props, size_t start)
{
#ifdef BIDI_SUPPORT
   props->bidi_dir = (evas_bidi_is_rtl_char(
            bidi_par_props,
            0,
            start)) ? EVAS_BIDI_DIRECTION_RTL : EVAS_BIDI_DIRECTION_LTR;
#else
   (void) start;
   (void) bidi_par_props;
   props->bidi_dir = EVAS_BIDI_DIRECTION_LTR;
#endif
   PROPS_CHANGE(props);
}

void
evas_common_text_props_script_set(Evas_Text_Props *props, Evas_Script_Type scr)
{
   props->script = scr;
   PROPS_CHANGE(props);
}

void
evas_common_text_props_content_copy_and_ref(Evas_Text_Props *dst,
      const Evas_Text_Props *src)
{
   memcpy(dst, src, sizeof(Evas_Text_Props));
   dst->glyphs = NULL;
   evas_common_text_props_content_ref(dst);
}

void
evas_common_text_props_content_ref(Evas_Text_Props *props)
{
   /* No content in this case */
   if (!props->info)
      return;

   props->info->refcount++;
   if (props->font_instance)
     ((RGBA_Font_Int *)props->font_instance)->references++;
}

void
evas_common_text_props_content_nofree_unref(Evas_Text_Props *props)
{
   /* No content in this case */
   if (!props->info)
      return;

   if (--(props->info->refcount) == 0)
     {
        if (props->font_instance)
          {
             evas_common_font_int_unref(props->font_instance);
             props->font_instance = NULL;
          }

        evas_common_font_glyphs_unref(props->glyphs);
        props->glyphs = NULL;

        if (props->info->glyph)
          free(props->info->glyph);
#ifdef OT_SUPPORT
        if (props->info->ot)
          free(props->info->ot);
#endif
        free(props->info);
        props->info = NULL;
     }
}

void
evas_common_text_props_content_unref(Evas_Text_Props *props)
{
   /* No content in this case */
   if (!props->info)
      return;

   if (props->glyphs) evas_common_font_glyphs_unref(props->glyphs);
   props->glyphs = NULL;

   if (--(props->info->refcount) == 0)
     {
        if (props->font_instance)
          {
             evas_common_font_int_unref(props->font_instance);
             props->font_instance = NULL;
          }

        if (props->info->glyph)
          free(props->info->glyph);
#ifdef OT_SUPPORT
        if (props->info->ot)
          free(props->info->ot);
#endif
        free(props->info);
        props->info = NULL;
     }
}

static int
_evas_common_text_props_cluster_move(const Evas_Text_Props *props, int pos,
      Eina_Bool right)
{
   int prop_pos = evas_common_text_props_index_find(props, pos);
   if (!right && (prop_pos > 0))
     {
#ifdef OT_SUPPORT
        int base_cluster = props->info->ot[props->start + prop_pos].source_cluster;
        prop_pos--;
        for ( ; prop_pos >= 0 ; prop_pos--)
          {
             int cur_cluster = props->info->ot[props->start + prop_pos].source_cluster;
             if (cur_cluster != base_cluster)
               {
                  return cur_cluster - props->text_offset;
               }
          }
#else
        return props->start + prop_pos - 1 - props->text_offset;
#endif
     }
   else if (right && (prop_pos < (int) (props->len - 1)))
     {
#ifdef OT_SUPPORT
        int base_cluster = props->info->ot[props->start + prop_pos].source_cluster;
        prop_pos++;
        for ( ; prop_pos < (int) props->len ; prop_pos++)
          {
             int cur_cluster = props->info->ot[props->start + prop_pos].source_cluster;
             if (cur_cluster != base_cluster)
               {
                  return cur_cluster - props->text_offset;
               }
          }
#else
        return props->start + prop_pos + 1 - props->text_offset;
#endif
     }

   return pos;
}

EVAS_API int
evas_common_text_props_cluster_next(const Evas_Text_Props *props, int pos)
{
   Eina_Bool right;
   /* Move right if we are in a non-rtl text */
   right = (props->bidi_dir != EVAS_BIDI_DIRECTION_RTL);
   return _evas_common_text_props_cluster_move(props, pos, right);
}

EVAS_API int
evas_common_text_props_cluster_prev(const Evas_Text_Props *props, int pos)
{
   Eina_Bool right;
   /* Move right if we are in an rtl text */
   right = (props->bidi_dir == EVAS_BIDI_DIRECTION_RTL);
   return _evas_common_text_props_cluster_move(props, pos, right);
}

/* Returns the index of the logical char in the props. */
EVAS_API int
evas_common_text_props_index_find(const Evas_Text_Props *props, int _cutoff)
{
#ifdef OT_SUPPORT
   Evas_Font_OT_Info *ot_info;
   int min = 0;
   int max = props->len - 1;
   int mid;

   /* Props can be empty, verify they are not before going on. */
   if (!props->info)
      return -1;

   _cutoff += props->text_offset;
   ot_info = props->info->ot + props->start;
   /* Should get us closer to the right place. */
   if ((min <= _cutoff) && (_cutoff <= max))
      mid = _cutoff;
   else
      mid = (min + max) / 2;

   if (props->bidi_dir == EVAS_BIDI_DIRECTION_RTL)
     {
        /* Monotonic in a descending order */
        do
          {
             if (_cutoff > (int) ot_info[mid].source_cluster)
                max = mid - 1;
             else if (_cutoff < (int) ot_info[mid].source_cluster)
                min = mid + 1;
             else
                break;

             mid = (min + max) / 2;
          }
        while (min <= max);
     }
   else
     {
        /* Monotonic in an ascending order */
        do
          {
             if (_cutoff < (int) ot_info[mid].source_cluster)
                max = mid - 1;
             else if (_cutoff > (int) ot_info[mid].source_cluster)
                min = mid + 1;
             else
                break;

             mid = (min + max) / 2;
          }
        while (min <= max);
     }

   /* If we didn't find, abort */
   if (min > max)
      return -1;

   ot_info += mid;
   if (props->bidi_dir == EVAS_BIDI_DIRECTION_RTL)
     {
        /* Walk to the last one of the same cluster */
        for ( ; mid < (int) props->len ; mid++, ot_info++)
          {
             if (ot_info->source_cluster != (size_t) _cutoff)
                break;
          }
        mid--;
     }
   else
     {
        /* Walk to the last one of the same cluster */
        for ( ; mid >= 0 ; mid--, ot_info--)
          {
             if (ot_info->source_cluster != (size_t) _cutoff)
                break;
          }
        mid++;
     }

   return mid;
#else
   if (props->bidi_dir == EVAS_BIDI_DIRECTION_RTL)
     {
        return props->len - _cutoff - 1;
     }
   else
     {
        return _cutoff;
     }
#endif
}

/* Won't work in the middle of ligatures, assumes cutoff < len.
 * Also won't work in the middle of indic words, should handle that in a
 * smart way. */
EVAS_API Eina_Bool
evas_common_text_props_split(Evas_Text_Props *base,
      Evas_Text_Props *ext, int _cutoff)
{
   size_t cutoff;

   /* Translate text cutoff pos to string object cutoff point */
   _cutoff = evas_common_text_props_index_find(base, _cutoff);

   if (_cutoff >= 0)
     {
        cutoff = (size_t) _cutoff;
     }
   else
     {
        ERR("Couldn't find the cutoff position. Is it inside a cluster?");
        return EINA_FALSE;
     }

   evas_common_text_props_content_copy_and_ref(ext, base);
   if (base->bidi_dir == EVAS_BIDI_DIRECTION_RTL)
     {
        ext->start = base->start;
        ext->len = cutoff + 1;
        base->start = base->start + ext->len;
        base->len = base->len - ext->len;

#ifdef OT_SUPPORT
        ext->text_offset =
           ext->info->ot[ext->start + ext->len - 1].source_cluster;
#else
        ext->text_offset = base->text_offset + base->len;
#endif
     }
   else
     {
        ext->start = base->start + cutoff;
        ext->len = base->len - cutoff;
        base->len = cutoff;

#ifdef OT_SUPPORT
        ext->text_offset = ext->info->ot[ext->start].source_cluster;
#else
        ext->text_offset = base->text_offset + base->len;
#endif
     }
   ext->text_len = base->text_len - (ext->text_offset - base->text_offset);
   base->text_len = (ext->text_offset - base->text_offset);
   PROPS_CHANGE(base);
   PROPS_CHANGE(ext);
   return EINA_TRUE;
}

/* Won't work in the middle of ligatures */
EVAS_API void
evas_common_text_props_merge(Evas_Text_Props *item1,
      const Evas_Text_Props *item2)
{
   if (item1->info != item2->info)
     {
        ERR("tried merge back items that weren't together in the first place.");
        return;
     }
   if (item1->bidi_dir == EVAS_BIDI_DIRECTION_RTL)
     {
        item1->start = item2->start;
     }

   item1->len += item2->len;
   item1->text_len += item2->text_len;
   PROPS_CHANGE(item1);
}

#ifdef OT_SUPPORT
static inline void
_content_create_ot(RGBA_Font_Int *fi, const Eina_Unicode *text,
      Evas_Text_Props *text_props, int len, Evas_Text_Props_Mode mode, const char *lang)
{
   size_t char_index;
   Evas_Font_Glyph_Info *gl_itr;
   Evas_Coord pen_x = 0, adjust_x = 0;

   evas_common_font_ot_populate_text_props(text, text_props, len, mode, lang);

   gl_itr = text_props->info->glyph;
   for (char_index = 0 ; char_index < text_props->len ; char_index++, gl_itr++)
     {
        FT_UInt idx;
        RGBA_Font_Glyph *fg;
        Eina_Bool is_replacement = EINA_FALSE;
        /* If we got a malformed index, show the replacement char instead */
        if (gl_itr->index == 0)
          {
             gl_itr->index = evas_common_get_char_index(fi, REPLACEMENT_CHAR, 0);
             is_replacement = EINA_TRUE;
          }
        idx = gl_itr->index;
        LKL(fi->ft_mutex);
        fg = evas_common_font_int_cache_glyph_get(fi, idx);
        if (!fg)
          {
             LKU(fi->ft_mutex);
             continue;
          }
        LKU(fi->ft_mutex);

        gl_itr->x_bear = fg->x_bear;
        gl_itr->y_bear = fg->y_bear;
        gl_itr->width = fg->width;
        /* text_props->info->glyph[char_index].advance =
         * text_props->info->glyph[char_index].index =
         * already done by the ot function */
        if (EVAS_FONT_CHARACTER_IS_INVISIBLE(
              text[text_props->info->ot[char_index].source_cluster]))
          {
             gl_itr->index = 0;
             /* Reduce the current advance */
             if (gl_itr > text_props->info->glyph)
               {
                  adjust_x -= gl_itr->pen_after - (gl_itr - 1)->pen_after;
               }
             else
               {
                  adjust_x -= gl_itr->pen_after;
               }
          }
        else
          {
             if (is_replacement)
               {
                  /* Update the advance accordingly */
                  adjust_x += (pen_x + (fg->advance.x >> 16)) -
                     gl_itr->pen_after;
               }
             pen_x = gl_itr->pen_after;
          }
        gl_itr->pen_after += adjust_x;

        fi = text_props->font_instance;
     }
}
#else
static inline void
_content_create_regular(RGBA_Font_Int *fi, const Eina_Unicode *text,
      Evas_Text_Props *text_props, const Evas_BiDi_Paragraph_Props *par_props,
      size_t par_pos, int len, Evas_Text_Props_Mode mode)
{
   /* We are walking the string in visual ordering */
   Evas_Font_Glyph_Info *gl_itr;
   Eina_Bool use_kerning;
   FT_UInt prev_index;
   FT_Face pface = NULL;
   Evas_Coord pen_x = 0;
   int adv_d, i;
#if !defined(OT_SUPPORT) && defined(BIDI_SUPPORT)
   Eina_Unicode *base_str = NULL;
   if (mode == EVAS_TEXT_PROPS_MODE_SHAPE)
     {
        if (text_props->bidi_dir == EVAS_BIDI_DIRECTION_RTL)
          {
             text = base_str = eina_unicode_strndup(text, len);
             evas_bidi_shape_string(base_str, par_props, par_pos, len);
          }
     }
#else
   (void) mode;
   (void) par_props;
   (void) par_pos;
#endif

   FTLOCK();
   use_kerning = FT_HAS_KERNING(fi->src->ft.face);
   FTUNLOCK();
   prev_index = 0;

   i = len;
   text_props->info->glyph = calloc(len,
                                    sizeof(Evas_Font_Glyph_Info));

   if (text_props->bidi_dir == EVAS_BIDI_DIRECTION_RTL)
     {
        text += len - 1;
        adv_d = -1;
     }
   else
     {
        adv_d = 1;
     }

   gl_itr = text_props->info->glyph;
   for ( ; i > 0 ; gl_itr++, text += adv_d, i--)
     {
        FT_UInt idx;
        RGBA_Font_Glyph *fg;
        int _gl, kern;
        Evas_Coord adv;
        _gl = *text;
        if (_gl == 0) break;

        idx = evas_common_get_char_index(fi, _gl, 0);
        if (idx == 0)
          {
             idx = evas_common_get_char_index(fi, REPLACEMENT_CHAR, 0);
          }

        fg = evas_common_font_int_cache_glyph_get(fi, idx);
        if (!fg) continue;
        kern = 0;

        if ((use_kerning) && (prev_index) && (idx) &&
            (pface == fi->src->ft.face))
          {
             if (evas_common_font_query_kerning(fi, prev_index, idx, &kern))
               {
                  pen_x += kern;
                  (gl_itr - 1)->pen_after +=
                     EVAS_FONT_ROUND_26_6_TO_INT(kern);
               }
          }

        pface = fi->src->ft.face;

        gl_itr->index = idx;
        gl_itr->x_bear = fg->x_bear;
        gl_itr->y_bear = fg->y_bear;
        adv = fg->advance.x >> 10;
        gl_itr->width = fg->width;

        if (EVAS_FONT_CHARACTER_IS_INVISIBLE(_gl))
          {
             gl_itr->index = 0;
          }
        else
          {
             pen_x += adv;
          }

        gl_itr->pen_after = EVAS_FONT_ROUND_26_6_TO_INT(pen_x);

        prev_index = idx;
     }
   text_props->len = len;
# if !defined(OT_SUPPORT) && defined(BIDI_SUPPORT)
   if (base_str)
      free(base_str);
# endif
}
#endif

EVAS_API Eina_Bool
evas_common_text_props_content_create(void *_fi, const Eina_Unicode *text,
      Evas_Text_Props *text_props, const Evas_BiDi_Paragraph_Props *par_props,
      size_t par_pos, int len, Evas_Text_Props_Mode mode, const char *lang)
{
   RGBA_Font_Int *fi = (RGBA_Font_Int *) _fi;

   if (text_props->info)
     {
        evas_common_text_props_content_unref(text_props);
     }
   if (len == 0)
     {
        text_props->info = NULL;
        text_props->start = text_props->len = text_props->text_offset = 0;
     }
   text_props->info = calloc(1, sizeof(Evas_Text_Props_Info));

   if (text_props->font_instance != fi)
     {
        if (text_props->font_instance)
          evas_common_font_int_unref(text_props->font_instance);
        text_props->font_instance = fi;
        fi->references++;
     }

   evas_common_font_int_reload(fi);
   if (fi->src->current_size != fi->size)
     {
        evas_common_font_source_reload(fi->src);
        FTLOCK();
        FT_Activate_Size(fi->ft.size);
        FTUNLOCK();
        fi->src->current_size = fi->size;
     }

   text_props->changed = EINA_TRUE;

#ifdef OT_SUPPORT
   (void) par_props;
   (void) par_pos;
   _content_create_ot(fi, text, text_props, len, mode, lang);
#else
   (void) lang;
   _content_create_regular(fi, text, text_props, par_props, par_pos, len, mode);
#endif

   text_props->text_len = len;
   text_props->info->refcount = 1;
   return EINA_TRUE;
}


/**
 * @internal
 * Returns the numeric value of HEX chars for example for ch = 'A'
 * the function will return 10.
 *
 * @param ch The HEX char.
 * @return numeric value of HEX.
 */
static int
_hex_string_get(char ch, Eina_Bool *ok)
{
   if ((ch >= '0') && (ch <= '9')) return (ch - '0');
   else if ((ch >= 'A') && (ch <= 'F')) return (ch - 'A' + 10);
   else if ((ch >= 'a') && (ch <= 'f')) return (ch - 'a' + 10);
   *ok = EINA_FALSE;
   return 0;
}

/**
 * @internal
 * copy lower source without space into dest buffer
 *
 * @param source source buffer
 * @param dest destination buffer
 * @param max maximum number of chars to be copied
 * @return number of spaces removed
 */

size_t remove_spaces_lowercase(const char* source,char * dest,size_t max)
{
  size_t spaces=0;
  while (*source && max)
    {
       if (*source != ' ')
         {
            *dest = ((*source >= 'A') && (*source <= 'Z')) ? *source + 32 : *source;
            dest++;
         }
       else
         {
              spaces++;
         }
         source++;
         max--;
    }
  return spaces;
}

/**
 * @internal
 * read color component from string
 *
 * @param source source string
 * @param[out] next position after the color compnent
 * @param byte store color value read from string
 * @return if read success
 */

Eina_Bool read_byte_color_component(const char* source,char ** next,unsigned char * byte)
{
  const char *p_start = source;
  char *p_stop = NULL;
  Eina_Bool b_ret = EINA_TRUE;
  long value = 0;
  value = strtol(p_start, &p_stop, 10);
  if ( value > 255 || value < 0 || p_start == p_stop )
    {
       b_ret = EINA_FALSE;
    }

   if (next)
     *next = p_stop;
   if (byte)
     *byte = (unsigned char) value;

   return b_ret;
}


/**
 * @internal
 * Parses a string of one of the formas:
 * 1. "#RRGGBB"
 * 2. "#RRGGBBAA"
 * 3. "#RGB"
 * 4. "#RGBA"
 * 5. "color names"
 * 6. "rgb(0-255,0-255,0-255)"
 * 7. "rgba(0-255,0-255,0-255,0.0-1.0)"
 * TODO (we may use specific color parser)
 * 8. "hsl(H,S,L)"
 * 9. "hsla(H,S,L,A)"
 * To the rgba values.
 *
 * @param[in] str The string to parse - NOT NULL.
 * @param[out] r The Red value - NOT NULL.
 * @param[out] g The Green value - NOT NULL.
 * @param[out] b The Blue value - NOT NULL.
 * @param[out] a The Alpha value - NOT NULL.
 */
Eina_Bool
evas_common_format_color_parse(const char *str, int slen,
                               unsigned char *r, unsigned char *g,
                               unsigned char *b, unsigned char *a)
{
   Eina_Bool v = EINA_TRUE;

   *r = *g = *b = *a = 0;
   if (slen>0 && str[0]=='#')
     {
        if (slen == 7) /* #RRGGBB */
          {
             *r = (_hex_string_get(str[1], &v) << 4) | (_hex_string_get(str[2], &v));
             *g = (_hex_string_get(str[3], &v) << 4) | (_hex_string_get(str[4], &v));
             *b = (_hex_string_get(str[5], &v) << 4) | (_hex_string_get(str[6], &v));
             *a = 0xff;
          }
        else if (slen == 9) /* #RRGGBBAA */
          {
             *r = (_hex_string_get(str[1], &v) << 4) | (_hex_string_get(str[2], &v));
             *g = (_hex_string_get(str[3], &v) << 4) | (_hex_string_get(str[4], &v));
             *b = (_hex_string_get(str[5], &v) << 4) | (_hex_string_get(str[6], &v));
             *a = (_hex_string_get(str[7], &v) << 4) | (_hex_string_get(str[8], &v));
          }
        else if (slen == 4) /* #RGB */
          {
             *r = _hex_string_get(str[1], &v);
             *r = (*r << 4) | *r;
             *g = _hex_string_get(str[2], &v);
             *g = (*g << 4) | *g;
             *b = _hex_string_get(str[3], &v);
             *b = (*b << 4) | *b;
             *a = 0xff;
          }
        else if (slen == 5) /* #RGBA */
          {
             *r = _hex_string_get(str[1], &v);
             *r = (*r << 4) | *r;
             *g = _hex_string_get(str[2], &v);
             *g = (*g << 4) | *g;
             *b = _hex_string_get(str[3], &v);
             *b = (*b << 4) | *b;
             *a = _hex_string_get(str[4], &v);
             *a = (*a << 4) | *a;
          }
        else v = EINA_FALSE;
      }
    else if (slen <= 25)/* search for rgb(),hsv(),colorname, 25 is length of rgba(255,255,255,1.0) */
      {
         /*remove spaces and convert name to lowercase*/
         char color_name[0xFF] = {0};
         slen = slen - (int) remove_spaces_lowercase(str,color_name,0xFF);

         if ((strncmp(color_name,"rgb(",4) == 0) && color_name[slen-1] == ')'&& slen >= 10 && slen <=16) /* rgb() */
           {
              char * p_color = &color_name[3];

              if (
                  (!read_byte_color_component(++p_color,&p_color,r)  || !p_color   || *p_color != ',') ||
                  (!read_byte_color_component(++p_color,&p_color,g)  || !p_color   || *p_color != ',') ||
                  (!read_byte_color_component(++p_color,&p_color,b)  || !p_color   || *p_color != ')')
                 )
                {
                   *r = *g = *b = *a = 0;
                   v = EINA_FALSE;
                }
              else
                {
                   *a = 0xff;
                }
           }
         else if ((strncmp(color_name,"rgba(",4) == 0) && color_name[slen-1] == ')'&& slen >= 13 && slen <=25) /* rgba() */
           {
              char * p_color = &color_name[4];

              if (
                  (!read_byte_color_component(++p_color,&p_color,r)  || !p_color   || *p_color != ',') ||
                  (!read_byte_color_component(++p_color,&p_color,g)  || !p_color   || *p_color != ',') ||
                  (!read_byte_color_component(++p_color,&p_color,b)  || !p_color   || *p_color != ',')
                 )
                {
                   *r = *g = *b = *a = 0;
                   v = EINA_FALSE;
                }
              else
                {
                   *a = (unsigned char)(strtof(++p_color, NULL) * 255);
                }
           }
         else
           {
              static size_t color_array_length = sizeof(color_name_value_sorted)/sizeof(Color_Name_Value);
              Color_Name_Value* pcolor = (Color_Name_Value*) bsearch(color_name, color_name_value_sorted, color_array_length, sizeof(Color_Name_Value),_color_name_search);
              if (pcolor)
                {
                   *r = pcolor->r;
                   *g = pcolor->g;
                   *b = pcolor->b;
                   *a = 0xff;
                }
              else
                {
                   v = EINA_FALSE;
                }
            }
      }

   if(*a != 0xFF)
     {
        *r = (*r * *a) / 255;
        *g = (*g * *a) / 255;
        *b = (*b * *a) / 255;
     }
   return v;
}

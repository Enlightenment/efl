dnl EFL_FORM_VOLTRON
dnl Outputs ascii art of Voltron if terminal has enough columns
dnl
dnl ascii art was found at http://www.codeismylife.com/ascii_voltron/5239.html
dnl and is the work of its original author.

AC_DEFUN([EFL_FORM_VOLTRON],
[
AC_ARG_ENABLE([voltron],
   [AC_HELP_STRING([--enable-voltron], [enable forming of voltron when all files combine @<:@default=no@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       have_voltron="yes"
    else
       have_voltron="no"
    fi
   ],
   [have_voltron="no"]
)

   if test "x$have_voltron" = "xyes" -a "x$do_amalgamation" = "xyes" -o "x${have_on_off_threads}" = "xyes"; then
     echo "//////////////////////////////////////////////////////////////////////////////////////////////////////////////////"
     echo "/////////////////////////////////////////////////////////////////////////////////////{#///////////////////////////"
     echo "/////////////////////////////////////////////////////////////////////////////////// EN3 xx&HWx////////////////////"
     echo "////////////////////////////////////////////////#{//xd3:& \/{:x////////////////////#WJNd_ HHNp#///////////////////"
     echo "///////////////////////////////////////////////\x WDMMM EDDzEQp&^x ^^_x #///////////&37z^xHHH7_///////////////////"
     echo "////////////////////////////////////////////////xPMMMMM LMMMLQQzzQDzH\{xx\{////////{x&H9@^&d^  {//////////////////"
     echo "///////////////////////////////////////////^JLE:PMMMQ9N^EDMMMMMMMLzJJ^ @&dx////////#dHdN^x{/{x {//////////////////"
     echo "//////////////////////////////////////////#LMMM EEE7p@H@NE777QMMMMMMM3H3_x/////////^HH3W^pH@x x///////////////////"
     echo "//////////////////////////////////////////#MMLN_^: {/:L&\{:@7EWWWzMMM3H3_x////{:x ^3dd3 HHNENd^{//////////////////"
     echo "//////////////////////////////////////////#WW7N^//////7Q:////{xHE93H3^^_ #////{H3H3JDdxx&H3Wz3:///////////////////"
     echo "//////////////////////////////////////////_E93 ///////{^#/#{{{{{{#x37dx77x////{Hp3pd   _ xx_7H////////////////////"
     echo "#/////////////////////////////////////////#&MHd{////////#x^ ^&__  xxzMJH9P ////{xx\#\:x\:d@_://///////////////////"
     echo "3H #///////////////////////////////////////{pp^Jpx{///{ &&__^:{{:xxxx7MQNWpx///@x//{# xxx#///////////////////////#"
     echo "3HN7Hx{//////////////////////////////////////\xd NQd#x \# &x{/x3pHHH&#&PP&@zNxx##:{////////////////////////////{ ^"
     echo "HH{@37N_x{////////////////////////////////////x_//#zDxx x  //#    ^x#//#E9_P&p://////////////////////////////{\&:\\"
     echo "H@/JMJ799H_:{/////////////////////////////////{\/// L7_xx^#/\LMMMMM3MLN@Mp7 dW{\////////////////////////////{  #^H"
     echo "H^/PMMMD9773d^\{////////////////////////////////////dzWLp/{x7MMMMMEJMM7DMHD#^z&#//////////////////////////{x&\ dHH"
     echo "H^{MMMMMMLEN33Hd_://////////////////////////////////#WNMLzzPPLMMMLHMML^NNNQW93JdE_ //////////////////////#_ :@H3HH"
     echo "HdxWMMMMMMMMzWdHHpNH {/////////////////////////////{z99QEJDQJ7@EM3LMM37M9LHLMNL:^\{////////////////////{x_\_HHH&&x"
     echo "HH_^zMMMMMMMML7dHH39zN_\//////////////////////////{WQLHEENpDPDLHQMMPHMDEzNMLWJ///////////////////////#_x d3Hd^dHx"
     echo "/\@HH HLMMMMMMMM9 3HHHp9DJ&#////////////////////////xN_\LzPMMMLpPMMPdLMWLdMMNQ3/////////////////////{ ^x&dddd HHH "
     echo "//{ HH@ 9MMMMMMLxdHHHHH3HpEQN /////////////////////:PL\/QMMMLE3LMMp#JMJJ9JMM&M //////////////////{#\ _ ^^&:x##HHHx"
     echo "////:d3H_&PMMMMN&3HHHHHHHHHHWEJH\/////////////////{zMd//^EE {/xHEH&HMLHLHMMJpL#////////{#x _^^&WpHHHHH3HHNxHHxHHHx"
     echo "/////#_HHH^WMMP3L9HHHHHHHHHHHHHWE7_{//////////////7Mz{///:7EWNEzPL3LMWQWDMM_zN/{\x ^& zLMDpHp973HHHHHH3HH7x3H dHHx"
     echo "///////xHHH@_E&3D9N3H3HHHHHHHHHH3HW7dx///////////dMM ////#QJEEJJ9:Hp9dz^WdH_&d3p3HHHHdLQE3pNNHHHHHHHHH3HH9xHH &HHx"
     echo "////////#@3H3^ HHN3WpHHHHHHHHHHHHHHH33d #/////// MM7//{x@pppH__ME_MME@xH3Hzz3H33HHHH@7pHH33HHHHHHHHHHH3HH9xHH__HHx"
     echo "/////////{ HHHHHHHH333HHHHHHHHHHHHHHHHH33&x{///:Pz9HpWp@ :::{/pM^EMMML H3HELQ3H3HHHH_pHHHHHHHHHHHHHHHHHHp9\@d& HHx"
     echo "///////////:dHHHHHHHH33HHHHHHHHHHHHHHHHHHHHd^\\H33d_x::##{/{x_PDxMMMME&HHHHH99H3HHHd&HHHHHHHHHHHHHHHHHH9Wx&^ x HHx"
     echo "////////////{^HHHHHHHHHHHHHHHHHHHHHHHHHHH@&HN33@H77ppppppWW3&JMdHMMMMHHHHHHHHWHHHHH&3pWHHHHHHHHHH3HHHpz_ HHH3&xHHx"
     echo "//////////////xHHHHHHHHHHHHHHHHHHHH3H3@d3p3@x:\{/# HN73& xx  DL\DMMML_HHHHHHHHpHHHpH7JHHHHHHH3p33d&_@d\ 3HHH3&:3Hx"
     echo "///////////////#&HHHHHHHHHHHHHHHHHHHH_H7^x\//#^ddH3^Nz@_&&&&_MN MMMMJ@H3HHHHH3p3H3pQQ3Hd@_^   xx ^@dH&\{x@HH3_\HHx"
     echo "////////////////{ HHHHHHHHHHHHHHH@ x{{z\/#^33d {Nz3:LPx&&&&^WMxNMMMM3HHH3N3Hp79zWH _ xx  _@dHHHHHHHHHHH^#{xd_{#@Hx"
     echo "//////////////////\@HH3HHHHHH@^x#{: &&7_@ddH:7d@__&@Mp^&&&&xPz#LMMML&3HJMMzNH@^ ^&:@HHHHHHHHHHHHHHHHHHd&^{/////:Hx"
     echo "/////////////{{{/{/{^dHHHd^x#{\ &HHH W ^@&Q9 _^ _d3QL ^   xWMW#dJMMz@HJMEx\x^HHHHHdx&HHHHHHHd&^ xx\#{//////////#Hx"
     echo "/////////{x ^&:xEDW^ x#:## d^dH3HHHHxd_HHdMzHx# NMMM7x&dH_JLp7 {x\&@HHD@\:{#x ^HHHd&#x x\#{////////////////////{Hx"
     echo "//////{#x^^&&\ _D9^x\{{:^NQP3HHHHHH pxHHd9MMdxx EMMM _^@@PD3PMMLEQH@3W& &^ x##\##{/////////////////////////////{@x"
     echo "/////#:x^ @d:^^7N#{\ &HHWHH99HHHHHH:7 d&^LMM3^_dNpH_#^_3LEWMMMMMPddHpd:HHHH@  ^ x{#:////////////////////{x&////{x:"
     echo "////:x ^x W:^^HWx3dHHHH333H39HHHH3@&&xxx@J7@#:_@&&d : ELp9MMMMMz&H333#dHHH3H\///{:3x##{/{##{/{#\\\##### W_{////^x/"
     echo "//{ :x^ #E:^^&N\ELWHHH3dHHH3333HHH E\:xx__^_ d^EPML^dDPHDMMMMM7&H33N#&HHHHH /////{3//////////////////\3@{//////{//"
     echo "/{ xx{x{7_ ^^N^xJNzN3Hp@HHHH39HH3H J: ^ NEQ__^zMMM9dLJ3LMMMML3@HHH9\ H3HHH&{/////^ ////////////////\3d{///////////"
     echo "/x x///:9:^^p&x@3W3p3HHHpddd@H9HHH_Lx^ EMM7x&^MMMEEMWNMMMMMD@d3HH9 \HHHHHH //////7{//////////////#3H#/////////////"
     echo "\_:#///@  ^d3^xHH33N3@x  ^&dH&@HHH M_^^MML&x_3QJWQLdzMMMMMJ_HH3HNd#@HHHHHHx/////:W/////////////{x #///////////////"
     echo "^&:///{p\^_W^x&HWH^x &_HHHHHHd HHH PNx9LQ7W_3N779d\WMMMMMW&HHH3WN{_HHHHHH3://///p ////////////////////////////////"
     echo "@dx////Ep3Np^\@_:^dHHHd^Hd_ :\xHH3&EP^3d@d3Hp7zLMQ_{ JMLd@HHHHpE:&HHHHHHHH\/////@{////////////////////////////////"
     echo "H@ :#//DLMM9_: :HHHHH@^x#/////#HHHd_NNNJPMMMMMMMMMMQ&{d_HHHHH3zx@HHHHHHHHH#&@@@@7@@dpdx{//////////////////////////"
     echo "dJx #//_E_d&&^::_d_x#//\///////&HHH_HPMMMMMMMMMMLJ7Wp@_HHHHHHz_ HHHHHHHHHHx7QzJE97^x{/////////////////////////////"
     echo "^z_ {/\{_9   ^{\#//////:///////xHHH3H&7MMMPJN3d__@HHHHHHH3HH73:HHHHHHHHHH  ^  x#{@{///////////////////////////////"
     echo "^dD \{ x#^7^x\/////////x///////#HH33HHHd3d37pHH3HHHHHHHHHHHp7#dHHHHHHHH&_EMMMMMMMPz7d #///////////////////////////"
     echo "^^_73x ^ :xd^{/////////x////////_HHHHHH33NNWpp3HHHHHHHHHHHHE:^HHHHHH3H WMMMMMMMMDH7:\ JQ7d {//////////////////////"
     echo "^^@JzE^\x^ :\x{////////_///\_WJHxHHHHHHHHHHH3HHHHHHHHHHHHH9&xHHHHHHd  9MMMMMMMMLJ J//xQ\7LMMQpx///////////////////"
     echo " _3zLLLpx  x{///////{_ zH9PP9H {{dHHHHHHHHHHHHHHHHHHHHHHHp9#dHHH3H@x^zMMMMMMMMMPJ d//93DMMMMMMMQH\////////////////"
     echo " x^NE9dxx x://///////#dNP_\/{^H79@pHHHHHHHHHHHHHHHHHHH3H3zx_HHHHHH_&DMMMMMMMMMMDE :/ 7QMMMMMMMMMMMJ&#/////////////"
     echo " @3pQMp^7N^x//////////// __7LMMMM JHHHHHHHHHHHHH3HHHHH3H9@ HHHHHHH&LMMMMMMMMMMMP7^#{EWMMMMMMMMMMMMMML7 {//////////"
     echo "^^^&HLWd7d {///////////#3LMMMMMMMHpJpHHHHHHHHHHH3JQJ7NDWE\HHHH3H@ QMMMMMMMMMMMMLp&##LMMMMMMMMMMMMMMMMMMPd{////////"
     echo "3WNNN7pH99Wd #/////////pHMMMMMMMMD N993HH3HHHHHHHH9LMMMLx@3HHHH_ JMMMMMMMMMMMMMMd@##MMMMMMMMMMMMMMMMMMMMMQ{///////"
     echo "MMMMM3ELMMMMLE@&@_x////EWMMMMMMMMM3JJpEN3HHHHHHHH7DDMMMH HHHHd ^NMMMMMMMMMMMMMMM_d{#MMMMMMMMMMMMMMMMMMMMM@////////"
     echo "MMMM7{DMMMMD3JMMMMDJ /#DzMMMMMMMMML^Qz7W9p3pWN7799zJE7d\x_HH@:_HMMMMMMMMMMMMMMMWx3{\MMMMMMMMMMMMMMMMMMMMD{////////"
     echo "MMML:HMMMM9@MMMLdHJEH{ PLMMMMMMMMMME&pNp^^_^  x   ^^__   \#x{&&LMMMMMMMMMMMMMMMJ:p{\MMMMMMMMMMMMMMMMMMMM_/////////"
     echo "MMMW/7MMMM&LM3LdQz@JM33MMMMMMMMMMMMM&x _@dHHHHHHH@&_^ ^_  ^x:^LMMMMMMPQJ7Np333pW 3 xEDMMMMMMMMMMMMMMMMMJ//////////"
     echo "MML\/EMMMP3_xNpJMM3^&WzMPMMMMMMMMMMML3x^^^^_&dH3W7EzPPpx\x\\xd333H&_&@pEQPLMMQPM7E9LHd@@3ELMMMMMMMMMMMM //////////"
     echo "MM3//zMMMNW//QdMMM3 \#LQWMMMMMMMMDp@__&dd@@&__&&@dHpHd33 &x@33Hd@_^^^xMMMMMMM3pMx{HM x9/{\:@pNQMMMMMMMQ///////////"
     echo "MMH::PMMMW7{/P_MMM7^{xM7^MMMMMMD@@W&&dH3Hx_x:7DzJ97Np &H^&x_dHd@&&@d3WMMMMMMME&M WMMN/7{/WHzMLEWWNELMMW///////////"
     echo "_H_^^dNJzWH@/7HPMM@_/dD&@MMMLzp W@^7zE7pxHHN99EzQQ7_p9^H^_:&d&^   ^_&&MMMMMMMD M xMML#^x/{JMMMMMMMDW3Wx///////////"
     echo "Hz^x::xx{/{x^p3dMQx /7N H3@^_@#dW#:_d@@HHHHHH3NPN&NLMQ H & NN799997WWzMMMMMMMLxM^&LMM^{&/#LMMMMMMMMMMLN^#/////////"
     echo " x WpH@x\//////{@ { _^_@d^ ^__\dH MEH_d3HHHHHHN&3MMMMD H & H&&______ HMMMMMMMM P_3MMMN/&/#LMMMMMMMMMMMMML@////////"
     echo "\{xH7x::\////////\7MM9@_^_@HHd d_ MMM9@7HHd@NWHp^WMMMDx3 & 3dHW7EzDLMLLDQzJE9NxHxxp33_/x{/NMMMMMMMMMMMMMMP#///////"
     echo "# ^_ ^ ^x///////{9&MMJ^^^  ^_3xH_ MMM^73@_WQz93&@&@LLN_H __9WH&^ x:\#{\^&& {///{p//////////{x&WzLMMMMMMMMMH///////"
     echo "^^  3^^^#///////H_LMME&@dHH@ _x3@xPMz&&_7MMMMLDJHx\ 3pd #{/////////{x@HH&\//////7{////{#////////{x&WzMMMMML#//////"
     echo ": :d^^^x/////// &PMMM&_dd@&^^ {^W7^p xxH@@&__37NNW7NH^:xxxx   ^_&&@dHH&:////////p ////{^/////////////\&ELMM3//////"
     echo "{\:xxxx#//////{Hp77p@ x#////////#&&  xx::\\:xxxx  ^_&&&&&@@@&&&&_____x//////////##/////&////////////////{^NQ#/////"
   fi
])

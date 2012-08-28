" Vim syntax file
" Language:	EDC
" Maintainer:	Viktor Kojouharov
" Last Change:	2007 02 24

" For version 5.x: Clear all syntax items
" For version 6.x: Quit when a syntax file was already loaded
if version < 600
  syntax clear
elseif exists("b:current_syntax")
  finish
endif

" A bunch of useful keywords
syn keyword	edcBlock	images data fonts collections group contained
syn keyword	edcBlock	part parts dragable description contained
syn keyword	edcBlock	text font fill origin size image contained
syn keyword	edcBlock	programs program styles style contained
syn keyword 	edcBlock 	gradient spectra spectrum contained
syn keyword 	edcBlock 	color_classes color_class rel1 rel2 contained
syn keyword 	edcBlock 	items item file params externals contained
syn keyword 	edcBlock 	map rotation perspective script lua_script contained

syn keyword	edcLabel	item name alias min max type effect contained
syn keyword	edcLabel	mouse_events repeat_events clip_to contained
syn keyword	edcLabel	x y z confine events scale contained
syn keyword	edcLabel	ignore_flags precise_is_inside contained
syn keyword	edcLabel	use_alternate_font_metrics entry_mode contained
syn keyword	edcLabel	source source2 source3 source4 contained
syn keyword	edcLabel	source5 source6 multiline pointer_mode contained
syn keyword	edcLabel	state visible step aspect fixed middle contained
syn keyword	edcLabel	aspect_preference ellipsis elipsis image contained
syn keyword	edcLabel	relative offset to to_x to_y contained
syn keyword	edcLabel	border border_scale scale_hint color color2 color3 font size contained
syn keyword	edcLabel	signal action transition in filter contained
syn keyword	edcLabel	target after fit align contained
syn keyword	edcLabel	text smooth inherit tag base style contained
syn keyword	edcLabel	text_source color_class text_class contained
syn keyword	edcLabel	spectrum angle spread normal tween contained
syn keyword	edcLabel	padding prefer weight aspect_mode contained
syn keyword	edcLabel	options layout position span contained
syn keyword	edcLabel	homogeneous contained
syn keyword	edcLabel	on perspective light perspective_on contained
syn keyword	edcLabel	backface_cull alpha center focus zplane contained
syn keyword	edcLabel	int double string external script_only contained

syn keyword	edcConstant 	COMP RAW LOSSY NONE ON_HOLD AUTOGRAB NOGRAB
syn keyword	edcConstant 	TEXT IMAGE RECT TEXTBLOCK SWALLOW GRADIENT GROUP
syn keyword	edcConstant 	NONE PLAIN OUTLINE SOFT_OUTLINE SHADOW
syn keyword	edcConstant 	SOFT_SHADOW OUTLINE_SHADOW OUTLINE_SOFT_SHADOW
syn keyword	edcConstant	GLOW FAR_SHADOW FAR_SOFT_SHADOW
syn keyword	edcConstant 	STATE_SET ACTION_STOP SIGNAL_EMIT FOCUS_SET
syn keyword	edcConstant	DRAG_VAL_SET DRAG_VAL_STEP DRAG_VAL_PAGE
syn keyword	edcConstant	LINEAR SINUSOIDAL ACCELERATE DECELERATE
syn keyword	edcConstant	VERTICAL HORIZONTAL BOTH BOX TABLE
syn keyword	edcConstant	EDITABLE PASSWORD "default"

syn keyword	edcTodo		contained TODO FIXME XXX

syn match 	edcLabelMatch 	"\w\+:" contains=edcLabel
syn match 	edcBlockMatch 	"\w\+\_s*{" contains=edcBlock
syn match 	edcBlockMatch 	"\w\+\.\a"me=e-2 contains=edcBlock
" edcCommentGroup allows adding matches for special things in comments
syn cluster	edcCommentGroup	contains=edcTodo

" String and Character constants
" Highlight special characters (those which have a backslash) differently
syn match	edcSpecial	display contained "\\\(x\x\+\|\o\{1,3}\|.\|$\)"
syn region	edcString	start=+L\="+ skip=+\\\\\|\\"+ end=+"+ contains=edcSpecial
syn match	edcFormat	display "%\(\d\+\$\)\=[-+' #0*]*\(\d*\|\*\|\*\d\+\$\)\(\.\(\d*\|\*\|\*\d\+\$\)\)\=\([hlL]\|ll\)\=\([diuoxXfeEgGcCsSpn]\|\[\^\=.[^]]*\]\)" contained
syn match	edcFormat	display "%%" contained
syn region	edcString	start=+L\="+ skip=+\\\\\|\\"+ end=+"+ contains=cSpecial,cFormat

syn match	edcCharacter	"L\='[^\\]'"
syn match	edcCharacter	"L'[^']*'" contains=edcSpecial
syn match	edcSpecialError	"L\='\\[^'\"?\\abfnrtv]'"
syn match	edcSpecialCharacter "L\='\\['\"?\\abfnrtv]'"
syn match	edcSpecialCharacter display "L\='\\\o\{1,3}'"
syn match	edcSpecialCharacter display "'\\x\x\{1,2}'"
syn match	edcSpecialCharacter display "L'\\x\x\+'"

"when wanted, highlight trailing white space
if exists("edc_space_errors")
  if !exists("edc_no_trail_space_error")
    syn match	edcSpaceError	display excludenl "\s\+$"
  endif
  if !exists("edc_no_tab_space_error")
    syn match	edcSpaceError	display " \+\t"me=e-1
  endif
endif

"catch errors caused by wrong parenthesis and brackets
syn cluster	edcParenGroup	contains=edcParenError,edcIncluded,edcSpecial,edcCommentSkip,edcCommentString,edcComment2String,@edcCommentGroup,edcCommentStartError,edcUserCont,edcUserLabel,edcBitField,edcCommentSkip,edcOctalZero,edcFormat,edcNumber,edcFloat,edcOctal,edcOctalError,edcNumbersCom
if exists("edc_no_bracket_error")
  syn region	edcParen	transparent start='(' end=')' contains=ALLBUT,@edcParenGroup
  syn match	edcParenError	display ")"
  syn match	edcErrInParen	display contained "[{}]"
else
  syn region	edcParen	transparent start='(' end=')' contains=ALLBUT,@edcParenGroup,edcErrInBracket
  syn match	edcParenError	display "[\])]"
  syn match	edcErrInParen	display contained "[\]{}]"
  syn region	edcBracket	transparent start='\[' end=']' contains=ALLBUT,@edcParenGroup,edcErrInParen
  syn match	edcErrInBracket	display contained "[);{}]"
endif

"integer number, or floating point number without a dot and with "f".
syn case ignore
syn match	edcNumbers	display transparent "\<\d\|\.\d" contains=edcNumber,edcFloat,edcOctalError,edcOctal
" Same, but without octal error (for comments)
syn match	edcNumbersCom	display contained transparent "\<\d\|\.\d" contains=edcNumber,edcFloat,edcOctal
syn match	edcNumber	display contained "\d\+\(u\=l\{0,2}\|ll\=u\)\>"
"hex number
syn match	edcNumber	display contained "0x\x\+\(u\=l\{0,2}\|ll\=u\)\>"
" Flag the first zero of an octal number as something special
syn match	edcOctal	display contained "0\o\+\(u\=l\{0,2}\|ll\=u\)\>" contains=edcOctalZero
syn match	edcOctalZero	display contained "\<0"
syn match	edcFloat	display contained "\d\+f"
"floating point number, with dot, optional exponent
syn match	edcFloat	display contained "\d\+\.\d*\(e[-+]\=\d\+\)\=[fl]\="
"floating point number, starting with a dot, optional exponent
syn match	edcFloat	display contained "\.\d\+\(e[-+]\=\d\+\)\=[fl]\=\>"
"floating point number, without dot, with exponent
syn match	edcFloat	display contained "\d\+e[-+]\=\d\+[fl]\=\>"
" flag an octal number with wrong digits
syn match	edcOctalError	display contained "0\o*[89]\d*"
syn case match

if exists("edc_comment_strings")
  " A comment can contain edcString, edcCharacter and edcNumber.
  " But a "*/" inside a edcString in a edcComment DOES end the comment!  So we
  " need to use a special type of edcString: edcCommentString, which also ends
  " on "*/", and sees a "*" at the start of the line as comment again.
  " Unfortunately this doesn't very well work for // type of comments :-(
  syntax match	edcCommentSkip		contained "^\s*\*\($\|\s\+\)"
  syntax region edcCommentString	contained start=+L\=\\\@<!"+ skip=+\\\\\|\\"+ end=+"+ end=+\*/+me=s-1 contains=edcSpecial,edcCommentSkip
  syntax region edcComment2String	contained start=+L\=\\\@<!"+ skip=+\\\\\|\\"+ end=+"+ end="$" contains=edcSpecial
  syntax region edcCommentL		start="//" skip="\\$" end="$" keepend contains=@edcCommentGroup,edcComment2String,edcCharacter,edcNumbersCom,edcSpaceError
  syntax region edcComment		matchgroup=edcCommentStart start="/\*" matchgroup=NONE end="\*/" contains=@edcCommentGroup,edcCommentStartError,edcCommentString,edcCharacter,edcNumbersCom,edcSpaceError
else
  syn region	edcCommentL		start="//" skip="\\$" end="$" keepend contains=@edcCommentGroup,edcSpaceError
  syn region	edcComment		matchgroup=edcCommentStart start="/\*" matchgroup=NONE end="\*/" contains=@edcCommentGroup,edcCommentStartError,edcSpaceError
endif
" keep a // comment separately, it terminates a preproc. conditional
syntax match	edcCommentError		display "\*/"
syntax match	edcCommentStartError 	display "/\*"me=e-1 contained

syn region	edcPreCondit	start="^\s*#\s*\(if\|ifdef\|ifndef\|elif\)\>" skip="\\$" end="$" end="//"me=s-1 contains=edcComment,edcCharacter,edcParenError,edcNumbers,edcCommentError,edcSpaceError
syn match	edcPreCondit	display "^\s*#\s*\(else\|endif\)\>"
syn region	edcIncluded	display contained start=+"+ skip=+\\\\\|\\"+ end=+"+
syn match	edcIncluded	display contained "<[^>]*>"
syn match	edcInclude	display "^\s*#\s*include\>\s*["<]" contains=edcIncluded
syn cluster	edcPreProcGroup	contains=edcPreCondit,edcIncluded,edcInclude,edcDefine,edcErrInParen,edcErrInBracket,edcCommentSkip,edcCommentString,edcComment2String,@edcCommentGroup,edcCommentStartError,edcParen,edcBracket,edcMulti,edcUserLabel
syn cluster 	edcAlphaNum 	contains=edcSpecial,edcOctalZero,edcFormat,edcNumber,edcFloat,edcOctal,edcOctalError,edcNumbersCom,edcString
syn region	edcDefine	start="^\s*#\s*\(define\|undef\)\>" skip="\\$" end="$" end="//"me=s-1 contains=ALLBUT,@edcPreProcGroup
syn region	edcPreProc	start="^\s*#\s*\(pragma\>\|line\>\|warning\>\|warn\>\|error\>\)" skip="\\$" end="$" keepend contains=ALLBUT,@edcPreProcGroup

syn match	edcUserLabel	display "\I\i*" contained

syn include 	@edcEmbryo 	syntax/embryo.vim
unlet b:current_syntax
syn region 	edcScript	matchgroup=edcScriptTag start="\<script\_s*{" end="}" contains=@edcEmbryo,edcScriptTag
syn keyword     edcScriptTag    contained script

syn include 	@edcLua 	syntax/lua.vim
unlet b:current_syntax
syn region 	edcLuaScript	matchgroup=edcLuaScriptTag start="\<lua_script\_s*{" end="}" contains=@edcLua,edcLuaScriptTag
syn keyword     edcLuaScriptTag contained script

if exists("edc_minlines")
  let b:edc_minlines = edc_minlines
else
  let b:edc_minlines = 50	" #if 0 constructs can be long
endif
exec "syn sync ccomment edcComment minlines=" . b:edc_minlines
"syn sync fromstart

" Define the default highlighting.
" For version 5.7 and earlier: only when not done already
" For version 5.8 and later: only when an item doesn't have highlighting yet
if version >= 508 || !exists("did_edc_syn_inits")
  if version < 508
    let did_edc_syn_inits = 1
    command -nargs=+ HiLink hi link <args>
  else
    command -nargs=+ HiLink hi def link <args>
  endif

  HiLink edcFormat		edcSpecial
  HiLink edcCommentL		edcComment
  HiLink edcCommentStart	edcComment
  HiLink edcLabel		Label
  HiLink edcUserLabel		Label
  HiLink edcConditional		Conditional
  HiLink edcRepeat		Repeat
  HiLink edcCharacter		Character
  HiLink edcSpecialCharacter	cSpecial
  HiLink edcNumber		Number
  HiLink edcOctal		Number
  HiLink edcOctalZero		PreProc	 " link this to Error if you want
  HiLink edcFloat		Float
  HiLink edcOctalError		edcError
  HiLink edcParenError		edcError
  HiLink edcErrInParen		edcError
  HiLink edcErrInBracket	edcError
  HiLink edcCommentError	edcError
  HiLink edcCommentStartError	edcError
  HiLink edcSpaceError		edcError
  HiLink edcSpecialError	edcError
  HiLink edcOperator		Operator
  HiLink edcStructure		Structure
  HiLink edcStorageClass	StorageClass
  HiLink edcInclude		Include
  HiLink edcPreProc		PreProc
  HiLink edcDefine		Macro
  HiLink edcIncluded		edcString
  HiLink edcError		Error
  HiLink edcBlock		Function
  HiLink edcScriptTag		Function
  HiLink edcLuaScriptTag	Function
  HiLink edcPreCondit		PreCondit
  HiLink edcConstant		Constant
  HiLink edcCommentString	edcString
  HiLink edcComment2String	edcString
  HiLink edcCommentSkip		edcComment
  HiLink edcString		String
  HiLink edcComment		Comment
  HiLink edcSpecial		SpecialChar
  HiLink edcTodo		Todo

  delcommand HiLink
endif

let b:current_syntax = "edc"

" vim: ts=8

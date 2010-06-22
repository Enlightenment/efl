" Vim syntax file
" Language:	Embryo
" Maintainer:	Viktor Kojouharov
" Last Change:	2006 10 06

" For version 5.x: Clear all syntax items
" For version 6.x: Quit when a syntax file was already loaded
if version < 600
  syntax clear
elseif exists("b:current_syntax")
  finish
endif

" A bunch of useful keywords
syn keyword embryoConditional	if else switch
syn keyword embryoRepeat	while for do in
syn keyword embryoBranch	break continue
syn keyword embryoOperator	new
syn keyword embryoType		Float State_Param Msg_Type enum
syn keyword embryoStatement	return with native stock forward
syn keyword embryoLabel		case default
syn keyword embryoReserved	public
syn keyword embryoEdjeKey	PART PROGRAM

syn keyword	embryoTodo		contained TODO FIXME XXX

" embryoCommentGroup allows adding matches for special things in comments
syn cluster	embryoCommentGroup	contains=embryoTodo

" String and Character constants
" Highlight special characters (those which have a backslash) differently
syn match	embryoSpecial	display contained "\\\(x\x\+\|\o\{1,3}\|.\|$\)"
syn region	embryoString	start=+L\="+ skip=+\\\\\|\\"+ end=+"+ contains=embryoSpecial
syn match	embryoFormat	display "%\(\d\+\$\)\=[-+' #0*]*\(\d*\|\*\|\*\d\+\$\)\(\.\(\d*\|\*\|\*\d\+\$\)\)\=\([hlL]\|ll\)\=\([diuoxXfeEgGcCsSpn]\|\[\^\=.[^]]*\]\)" contained
syn match	embryoFormat	display "%%" contained
syn region	embryoString	start=+L\="+ skip=+\\\\\|\\"+ end=+"+ contains=cSpecial,cFormat

syn match	embryoCharacter	"L\='[^\\]'"
syn match	embryoCharacter	"L'[^']*'" contains=embryoSpecial
syn match	embryoSpecialError	"L\='\\[^'\"?\\abfnrtv]'"
syn match	embryoSpecialCharacter "L\='\\['\"?\\abfnrtv]'"
syn match	embryoSpecialCharacter display "L\='\\\o\{1,3}'"
syn match	embryoSpecialCharacter display "'\\x\x\{1,2}'"
syn match	embryoSpecialCharacter display "L'\\x\x\+'"

"when wanted, highlight trailing white space
if exists("embryo_space_errors")
  if !exists("embryo_no_trail_space_error")
    syn match	embryoSpaceError	display excludenl "\s\+$"
  endif
  if !exists("embryo_no_tab_space_error")
    syn match	embryoSpaceError	display " \+\t"me=e-1
  endif
endif

"catch errors caused by wrong parenthesis and brackets
syn cluster	embryoParenGroup	contains=embryoParenError,embryoIncluded,embryoSpecial,embryoCommentSkip,embryoCommentString,embryoComment2String,@embryoCommentGroup,embryoCommentStartErr,embryoUserCont,embryoUserLabel,embryoBitField,embryoCommentSkip,embryoOctalZero,embryoFormat,embryoNumber,embryoFloat,embryoOctal,embryoOctalError,embryoNumbersCom
if exists("embryo_no_bracket_error")
  syn region	embryoParen	transparent start='(' end=')' contains=ALLBUT,@embryoParenGroup
  syn match	embryoParenError	display ")"
  syn match	embryoErrInParen	display contained "[{}]"
else
  syn region	embryoParen	transparent start='(' end=')' contains=ALLBUT,@embryoParenGroup,embryoErrInBracket
  syn match	embryoParenError	display "[\])]"
  syn match	embryoErrInParen	display contained "[\]{}]"
  syn region	embryoBracket	transparent start='\[' end=']' contains=ALLBUT,@embryoParenGroup,embryoErrInParen
  syn match	embryoErrInBracket	display contained "[);{}]"
endif

syn region embryoBrace start='{' end='}' transparent keepend
"integer number, or floating point number without a dot and with "f".
syn case ignore
syn match	embryoNumbers	display transparent "\<\d\|\.\d" contains=embryoNumber,embryoFloat,embryoOctalError,embryoOctal
" Same, but without octal error (for comments)
syn match	embryoNumbersCom	display contained transparent "\<\d\|\.\d" contains=embryoNumber,embryoFloat,embryoOctal
syn match	embryoNumber	display contained "\d\+\(u\=l\{0,2}\|ll\=u\)\>"
"hex number
syn match	embryoNumber	display contained "0x\x\+\(u\=l\{0,2}\|ll\=u\)\>"
" Flag the first zero of an octal number as something special
syn match	embryoOctal	display contained "0\o\+\(u\=l\{0,2}\|ll\=u\)\>" contains=embryoOctalZero
syn match	embryoOctalZero	display contained "\<0"
syn match	embryoFloat	display contained "\d\+f"
"floating point number, with dot, optional exponent
syn match	embryoFloat	display contained "\d\+\.\d*\(e[-+]\=\d\+\)\=[fl]\="
"floating point number, starting with a dot, optional exponent
syn match	embryoFloat	display contained "\.\d\+\(e[-+]\=\d\+\)\=[fl]\=\>"
"floating point number, without dot, with exponent
syn match	embryoFloat	display contained "\d\+e[-+]\=\d\+[fl]\=\>"
" flag an octal number with wrong digits
syn match	embryoOctalError	display contained "0\o*[89]\d*"
syn case match

if exists("embryo_comment_strings")
  " A comment can contain embryoString, embryoCharacter and embryoNumber.
  " But a "*/" inside a embryoString in a embryoComment DOES end the comment!  So we
  " need to use a special type of embryoString: embryoCommentString, which also ends
  " on "*/", and sees a "*" at the start of the line as comment again.
  " Unfortunately this doesn't very well work for // type of comments :-(
  syntax match	embryoCommentSkip		contained "^\s*\*\($\|\s\+\)"
  syntax region embryoCommentString	contained start=+L\=\\\@<!"+ skip=+\\\\\|\\"+ end=+"+ end=+\*/+me=s-1 contains=embryoSpecial,embryoCommentSkip
  syntax region embryoComment2String	contained start=+L\=\\\@<!"+ skip=+\\\\\|\\"+ end=+"+ end="$" contains=embryoSpecial
  syntax region embryoCommentL		start="//" skip="\\$" end="$" keepend contains=@embryoCommentGroup,embryoComment2String,embryoCharacter,embryoNumbersCom,embryoSpaceError
  syntax region embryoComment		matchgroup=embryoCommentStart start="/\*" matchgroup=NONE end="\*/" contains=@embryoCommentGroup,embryoCommentStartErr,embryoCommentString,embryoCharacter,embryoNumbersCom,embryoSpaceError
else
  syn region	embryoCommentL		start="//" skip="\\$" end="$" keepend contains=@embryoCommentGroup,embryoSpaceError
  syn region	embryoComment		matchgroup=embryoCommentStart start="/\*" matchgroup=NONE end="\*/" contains=@embryoCommentGroup,embryoCommentStartErr,embryoSpaceError
endif
" keep a // comment separately, it terminates a preproc. conditional
syntax match	embryoCommentError		display "\*/"
syntax match	embryoCommentStartErr 	display "/\*"me=e-1 contained

syn region	embryoPreCondit	start="^\s*#\s*\(if\|ifdef\|ifndef\|elif\)\>" skip="\\$" end="$" end="//"me=s-1 contains=embryoComment,embryoCharacter,embryoParenError,embryoNumbers,embryoCommentError,embryoSpaceError
syn match	embryoPreCondit	display "^\s*#\s*\(else\|endif\)\>"
syn region	embryoIncluded	display contained start=+"+ skip=+\\\\\|\\"+ end=+"+
syn match	embryoIncluded	display contained "<[^>]*>"
syn match	embryoInclude	display "^\s*#\s*include\>\s*["<]" contains=embryoIncluded
syn cluster	embryoPreProcGroup	contains=embryoPreCondit,embryoIncluded,embryoInclude,embryoDefine,embryoErrInParen,embryoErrInBracket,embryoCommentSkip,embryoCommentString,embryoComment2String,@embryoCommentGroup,embryoCommentStartErr,embryoParen,embryoBracket,embryoMulti,embryoUserLabel
syn cluster 	embryoAlphaNum 	contains=embryoSpecial,embryoOctalZero,embryoFormat,embryoNumber,embryoFloat,embryoOctal,embryoOctalError,embryoNumbersCom,embryoString
syn region	embryoDefine	start="^\s*#\s*\(define\|undef\)\>" skip="\\$" end="$" end="//"me=s-1 contains=ALLBUT,@embryoPreProcGroup
syn region	embryoPreProc	start="^\s*#\s*\(pragma\>\|line\>\|warning\>\|warn\>\|error\>\)" skip="\\$" end="$" keepend contains=ALLBUT,@embryoPreProcGroup

syn match	embryoUserLabel	display "\I\i*" contained

syn match 	embryoFunctionName	"\h\w*\s*\%((\@=\)"

if exists("embryo_minlines")
  let b:embryo_minlines = embryo_minlines
else
  let b:embryo_minlines = 50	" #if 0 constructs can be long
endif
exec "syn sync ccomment embryoComment minlines=" . b:embryo_minlines
"syn sync fromstart

" Define the default highlighting.
" For version 5.7 and earlier: only when not done already
" For version 5.8 and later: only when an item doesn't have highlighting yet
if version >= 508 || !exists("did_embryo_syn_inits")
  if version < 508
    let did_embryo_syn_inits = 1
    command -nargs=+ HiLink hi link <args>
  else
    command -nargs=+ HiLink hi def link <args>
  endif

  HiLink embryoFormat		embryoSpecial
  HiLink embryoCommentL		embryoComment
  HiLink embryoCommentStart	embryoComment
  HiLink embryoLabel		Label
  HiLink embryoUserLabel	Label
  HiLink embryoConditional	Conditional
  HiLink embryoRepeat		Repeat
  HiLink embryoBranch		Conditional
  HiLink embryoReserved		Keyword
  HiLink embryoCharacter	Character
  HiLink embryoSpecialCharacter	cSpecial
  HiLink embryoNumber		Number
  HiLink embryoOctal		Number
  HiLink embryoOctalZero	PreProc	 " link this to Error if you want
  HiLink embryoFloat		Float
  HiLink embryoOctalError	embryoError
  HiLink embryoParenError	embryoError
  HiLink embryoErrInParen	embryoError
  HiLink embryoErrInBracket	embryoError
  HiLink embryoCommentError	embryoError
  HiLink embryoCommentStartErr	embryoError
  HiLink embryoSpaceError	embryoError
  HiLink embryoSpecialError	embryoError
  HiLink embryoOperator		Operator
  HiLink embryoStructure	Structure
  HiLink embryoEdjeKey		Structure
  HiLink embryoStorageClass	StorageClass
  HiLink embryoInclude		Include
  HiLink embryoPreProc		PreProc
  HiLink embryoDefine		Macro
  HiLink embryoIncluded		embryoString
  HiLink embryoError		Error
  HiLink embryoStatement	Statement
  HiLink embryoPreCondit	PreCondit
  HiLink embryoType		Type
  HiLink embryoConstant		Constant
  HiLink embryoCommentString	embryoString
  HiLink embryoComment2String	embryoString
  HiLink embryoCommentSkip	embryoComment
  HiLink embryoString		String
  HiLink embryoComment		Comment
  HiLink embryoSpecial		SpecialChar
  HiLink embryoTodo		Todo
  HiLink embryoFunctionName	Function

  delcommand HiLink
endif

let b:current_syntax = "embryo"

" vim: ts=8

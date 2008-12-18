" Vimball Archiver by Charles E. Campbell, Jr., Ph.D.
UseVimball
finish
ftplugin/edc.vim	[[[1
24
" Vim filetype plugin file
" Language:         EDC
" Maintainer:       Viktor Kojouharov
" Latest Revision:  2006-10-29

if exists("b:did_ftplugin")
  finish
endif
let b:did_ftplugin = 1

let b:undo_ftplugin = "setl com< cms< inc< fo< efm< mp<"

setlocal comments=sO:*\ -,mO:*\ \ ,exO:*/,s1:/*,mb:*,ex:*/,://
setlocal commentstring=/*%s*/
setlocal formatoptions-=t formatoptions+=croql
setlocal include=^\s*#\s*include
setlocal efm=edje_cc:%s.\ %f:%l\ %m
setlocal mp=edje_cc\ %

if exists('&ofu')
  setlocal ofu=edccomplete#Complete
  setlocal cfu=edccomplete#Complete
endif

syntax/edc.vim	[[[1
225
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
syn keyword 	edcBlock 	items item contained

syn keyword	edcLabel	item name alias min max type effect contained
syn keyword	edcLabel	mouse_events repeat_events clip_to contained
syn keyword	edcLabel	x y confine events scale contained
syn keyword	edcLabel	ignore_flags precise_is_inside contained
syn keyword	edcLabel	use_alternate_font_metrics entry_mode contained
syn keyword	edcLabel	source source2 source3 source4 contained
syn keyword	edcLabel	source5 source6 multiline pointer_mode contained
syn keyword	edcLabel	state visible step aspect fixed middle contained
syn keyword	edcLabel	aspect_preference elipsis image contained
syn keyword	edcLabel	relative offset to to_x to_y contained
syn keyword	edcLabel	border color color2 color3 font size contained
syn keyword	edcLabel	signal action transition in contained
syn keyword	edcLabel	target after fit align contained
syn keyword	edcLabel	text smooth inherit tag base style contained
syn keyword	edcLabel	text_source color_class text_class contained
syn keyword	edcLabel	spectrum angle spread normal tween contained
syn keyword	edcLabel	padding prefer weight aspect_mode contained
syn keyword	edcLabel	options layout position span contained
syn keyword	edcLabel	homogeneous contained

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
indent/edc.vim	[[[1
83
" Vim indent file
" Language:         EDC
" Maintainer:       Viktor Kojouharov
" Latest Revision:  2007 02 24

if exists("b:did_indent")
  finish
endif
let b:did_indent = 1

setlocal indentexpr=GetEDCIndent()
setlocal indentkeys=0{,0},!^F,o,O

if exists("*GetEDCIndent")
  finish
endif

function s:prevnonblanknoncomment(lnum)
  let lnum = a:lnum
  while lnum > 1
    let lnum = prevnonblank(lnum)
    let line = getline(lnum)
    if line =~ '\*/'
      while lnum > 1 && line !~ '/\*'
	let lnum -= 1
      endwhile
      if line =~ '^\s*/\*'
	let lnum -= 1
      else
	break
      endif
    elseif line =~ '^\s*//'
      let lnum -= 1
    else
      break
    endif
  endwhile
  return lnum
endfunction

function s:count_braces(lnum, count_open)
  let n_open = 0
  let n_close = 0
  let line = getline(a:lnum)
  let pattern = '[{}]'
  let i = match(line, pattern)
  while i != -1
    if synIDattr(synID(a:lnum, i + 1, 0), 'name') !~ 'c\%(CommentL\|Comment\|StringQ\{1,2}\)'
      if line[i] == '{'
	let n_open += 1
      elseif line[i] == '}'
	if n_open > 0
	  let n_open -= 1
	else
	  let n_close += 1
	endif
      endif
    endif
    let i = match(line, pattern, i + 1)
  endwhile
  return a:count_open ? n_open : n_close
endfunction

function GetEDCIndent()
  let line = getline(v:lnum)
  if line =~ '^\s*\*' || line =~ '^\s*//' || line =~ '^\s*}'
    return cindent(v:lnum)
  endif

  let pnum = s:prevnonblanknoncomment(v:lnum - 1)
  if pnum == 0
    return 0
  endif

  let ind = indent(pnum) + s:count_braces(pnum, 1) * &sw

  let pline = getline(pnum)
  if pline =~ '}\s*$'
    let ind -= (s:count_braces(pnum, 0) - (pline =~ '^\s*}' ? 1 : 0)) * &sw
  endif

  return ind
endfunction
ftdetect/edc.vim	[[[1
3
au BufRead,BufNewFile *.edc	set filetype=edc
au BufRead,BufNewFile *.sma	set filetype=embryo
au BufRead,BufNewFile *.embryo	set filetype=embryo
autoload/edccomplete.vim	[[[1
774
" Vim completion script
" Language:	EDC
" Maintainer:	Viktor Kojouharov
" Last Change:	2007 02 24

function! edccomplete#Complete(findstart, base)
  if a:findstart
    " locate the start of the word
    let line = getline('.')
    let start = col('.') - 1
    let compl_begin = col('.') - 2
    let lastword = -1
    if line =~ ':' && line !~ '\.'
      while start > 0 && (line[start - 1] =~ '\k' || line[start - 1] =~ '"')
	let start -= 1
      endwhile
    else
      while start > 0
	if line[start - 1] =~ '\k'
	  let start -= 1
	elseif line[start - 1] =~ '\.'
	  if lastword == -1
	    let lastword = start - 2
	  endif
	  let start -= 1
	else
	  break
	endif
      endwhile
    endif
    let b:compl_context = getline('.')[0:compl_begin]

    if lastword == -1
      let ppe = searchpos('\.', 'bcn')
      let pps = searchpos('\w\+\.', 'bcn')
      let b:sparent = ''
      if ppe != [0, 0] && pps[0] == ppe[0] && pps[1] <= ppe[1] && pps[0] == line('.')
	let b:scontext = line[pps[1] -1 : ppe[1] - 2]
        call edccomplete#FindParent(pps[0], pps[1])
	return start
      endif

      let startpos = searchpair('{', '', '}', 'bnW')
      let lnum = startpos
      let line = getline(lnum)

      if line !~ '\a\+'
        let lnum = prevnonblank(lnum - 1)
	let line = getline(lnum)
      endif

      call edccomplete#FindParent(lnum, 1)
      let b:scontext = matchstr(line, '\w\+')

      return start
    else
      let b:scontext = line[start : lastword]

      return lastword + 2
    endif
  else
    " find months matching with "a:base"
    let res = []
    if exists("b:compl_context")
      let line = b:compl_context
      unlet! b:compl_context
    else
      let line = a:base
    endif

    if b:scontext == 'part'
      call edccomplete#AddLabel(res, line, a:base, s:partLabel)
      call edccomplete#AddStatement(res, line, a:base, s:partStatement)
      if line =~ 'type:\s*'
	call edccomplete#AddKeyword(res, a:base, s:partTypes)
      elseif line =~ 'effect:\s*'
	call edccomplete#AddKeyword(res, a:base, s:partEffects)
      elseif line =~ 'ignore_flags:\s*'
	call edccomplete#AddKeyword(res, a:base, s:partIgnoreFlags)
      elseif line =~ 'pointer_mode:\s*'
	call edccomplete#AddKeyword(res, a:base, s:partPointerMode)
      elseif line =~ 'editable_mode:\s*'
	call edccomplete#AddKeyword(res, a:base, s:partEditableMode)
      endif
      if line =~ 'image:\s*".\{-}"'
	call edccomplete#AddKeyword(res, a:base, s:imageStorageMethod)
      endif

    elseif b:scontext == 'dragable'
      call edccomplete#AddLabel(res, line, a:base, s:dragableLabel)

    elseif b:scontext == 'description'
      call edccomplete#AddLabel(res, line, a:base, s:descriptionLabel)
      call edccomplete#AddStatement(res, line, a:base, s:descriptionStatement)
      if line =~ 'aspect_preference:\s*'
	call edccomplete#AddKeyword(res, a:base, s:aspectPrefTypes)
      elseif line =~ 'inherit:\s*"\?'
	call edccomplete#FindStates(res, a:base, 1)
      endif

    elseif b:scontext == 'rel1' || b:scontext == 'rel2'
      call edccomplete#AddLabel(res, line, a:base, s:relLabel)
      if line =~ 'to\%(_[xy]\)\?:\s*"\?'
	call edccomplete#FindNamesIn(res, a:base, 'parts')
      endif

    elseif b:scontext == 'image'
      call edccomplete#AddLabel(res, line, a:base, s:imageLabel)
      call edccomplete#AddStatement(res, line, a:base, s:imageStatement)
      if line =~ 'image:\s*".\{-}"'
	call edccomplete#AddKeyword(res, a:base, s:imageStorageMethod)
      endif

    elseif b:scontext == 'fill'
      call edccomplete#AddLabel(res, line, a:base, s:fillLabel)
      call edccomplete#AddStatement(res, line, a:base, s:fillStatement)

    elseif b:scontext == 'origin' || b:scontext == 'size'
      call edccomplete#AddLabel(res, line, a:base, s:fillInnerStatement)

    elseif b:scontext == 'text'
      call edccomplete#AddLabel(res, line, a:base, s:textLabel)
      call edccomplete#AddStatement(res, line, a:base, s:textStatement)

    elseif b:scontext == 'program'
      call edccomplete#AddLabel(res, line, a:base, s:programLabel)
      call edccomplete#AddStatement(res, line, a:base, s:programStatement)
      if line =~ 'transition:\s*'
	call edccomplete#AddKeyword(res, a:base, s:transitionTypes)
      elseif line =~ 'STATE_SET\s*"\?'
	call edccomplete#FindStates(res, a:base, 0)
      elseif line =~ 'action:\s*'
	call edccomplete#AddKeyword(res, a:base, s:actionTypes)
      elseif line =~ 'target:\s*"\?'
	call edccomplete#FindNamesIn(res, a:base, 'parts')
      elseif line =~ 'after:\s*"\?'
	call edccomplete#FindNamesIn(res, a:base, 'programs')
      endif

    elseif b:scontext == 'programs'
      call edccomplete#AddLabel(res, line, a:base, s:programsLabel)
      call edccomplete#AddStatement(res, line, a:base, s:programsStatement)
      if line =~ 'image:\s*".\{-}"'
	call edccomplete#AddKeyword(res, a:base, s:imageStorageMethod)
      endif

    elseif b:scontext == 'box' && b:sparent == 'part'
      call edccomplete#AddStatement(res, line, a:base, s:boxStatement)

    elseif b:scontext == 'items'
      call edccomplete#AddStatement(res, line, a:base, s:boxItemsStatement)

    elseif b:scontext == 'item'
      call edccomplete#AddLabel(res, line, a:base, s:boxItemLabel)
      if line =~ 'type:\s*'
	call edccomplete#AddKeyword(res, a:base, s:boxItemTypes)
      elseif line =~ 'aspect_mode:\s*"\?'
	call edccomplete#AddKeyword(res, a:base, s:boxItemAspectMode)
      endif

    elseif b:scontext == 'box' && b:sparent == 'description'
      call edccomplete#AddLabel(res, line, a:base, s:boxDescLabel)
      if line =~ 'layout:\s*'
	call edccomplete#AddKeyword(res, a:base, s:boxLayout)
      endif

    elseif b:scontext == 'table' && b:sparent == 'description'
      call edccomplete#AddLabel(res, line, a:base, s:tableDescLabel)
      if line =~ 'homogeneous:\s*'
	call edccomplete#AddKeyword(res, a:base, s:tableHomogeneousMode)
      endif

    elseif b:scontext == 'group'
      call edccomplete#AddLabel(res, line, a:base, s:groupLabel)
      call edccomplete#AddStatement(res, line, a:base, s:groupStatement)
      if line =~ 'image:\s*".\{-}"'
	call edccomplete#AddKeyword(res, a:base, s:imageStorageMethod)
      endif

    elseif b:scontext == 'parts'
      call edccomplete#AddLabel(res, line, a:base, s:partsLabel)
      call edccomplete#AddStatement(res, line, a:base, s:partsStatement)
      if line =~ 'image:\s*".\{-}"'
	call edccomplete#AddKeyword(res, a:base, s:imageStorageMethod)
      endif

    elseif b:scontext == 'data'
      call edccomplete#AddLabel(res, line, a:base, s:dataLabel)

    elseif b:scontext == 'fonts'
      call edccomplete#AddLabel(res, line, a:base, s:fontsLabel)

    elseif b:scontext == 'spectra'
      call edccomplete#AddStatement(res, line, a:base, s:spectraStatement)

    elseif b:scontext == 'spectrum'
      call edccomplete#AddLabel(res, line, a:base, s:spectrumLabel)

    elseif b:scontext == 'gradient'
      call edccomplete#AddLabel(res, line, a:base, s:gradientLabel)
      call edccomplete#AddStatement(res, line, a:base, s:gradientStatement)

    elseif b:scontext == 'styles'
      call edccomplete#AddStatement(res, line, a:base, s:stylesStatement)

    elseif b:scontext == 'style'
      call edccomplete#AddLabel(res, line, a:base, s:styleLabel)

    elseif b:scontext == 'color_classes'
      call edccomplete#AddStatement(res, line, a:base, s:color_classesStatement)

    elseif b:scontext == 'color_class'
      call edccomplete#AddLabel(res, line, a:base, s:color_classLabel)

    elseif b:scontext == 'images'
      call edccomplete#AddLabel(res, line, a:base, s:imagesLabel)
      if line =~ 'image:\s*".\{-}"'
	call edccomplete#AddKeyword(res, a:base, s:imageStorageMethod)
      endif

    elseif b:scontext == 'collections'
      call edccomplete#AddLabel(res, line, a:base, s:collectionsLabel)
      call edccomplete#AddStatement(res, line, a:base, s:collectionsStatement)
      if line =~ 'image:\s*".\{-}"'
	call edccomplete#AddKeyword(res, a:base, s:imageStorageMethod)
      endif

    elseif strlen(b:scontext) == 0
      call edccomplete#AddStatement(res, line, a:base, s:topStatement)
    endif

    unlet! b:scontext

    return res
  endif
endfunction

function! edccomplete#AddLabel(res, line, base, label)
  if a:line =~ ':'
    return
  endif

  for m in sort(keys(a:label))
    if m =~ '^' . a:base
      call add(a:res, {'word': m . ':', 'menu': a:label[m]})
    endif
  endfor
endfunction

function! edccomplete#AddKeyword(res, base, label)
  for m in sort(keys(a:label))
    if m =~ '^' . a:base
      call add(a:res, {'word': m, 'menu': a:label[m]})
    endif
  endfor
endfunction

function! edccomplete#AddStatement(res, line, base, statement)
  if a:line =~ ':'
    return
  endif

  for m in sort(a:statement)
    if m =~ '^' . a:base
      call add(a:res, m . ' {')
    endif
  endfor
endfunction

function! edccomplete#FindStates(res, base, in_part)
  let curpos = getpos('.')
  call remove(curpos, 0, 0)

  let states_list = []
  if a:in_part == 1 	" in the current part only
    let part_start = search('^[ \t}]*\<part\>[ \t{]*$', 'bnW')
    if part_start != 0  " found it
      let line = getline(part_start)
      if line !~ '{'
	let part_start = nextnonblank(part_start)
      endif
      call cursor(part_start, 0)
      let part_end = searchpair('{', '', '}', 'nW')
    endif
  else 			" in the current parts group
    let part_start = search('^[ \t}]*\<parts\>[ \t{]*$', 'bnW')
    if part_start != 0  " found it
      let line = getline(part_start)
      if line !~ '{'
	let part_start = nextnonblank(part_start)
      endif
      call cursor(part_start, 0)
      let part_end = searchpair('{', '', '}', 'nW')
    endif
  endif

  let state_num = search('\%(state:\s*\)"\w\+"', 'W', part_end)
  while state_num
    let state = matchstr(getline(state_num), '\%(state:\s*\)\@<="\w\+"')
    call extend(states_list, [state])
    let state_num = search('\%(state:\s*\)"\w\+"', 'W', part_end)
  endwhile
  call cursor(curpos)

  for m in sort(states_list)
    if m =~ '^' . a:base
      call add(a:res, m)
    endif
  endfor
endfunction

function! edccomplete#FindNamesIn(res, base, str)
  let curpos = getpos('.')
  call remove(curpos, 0, 0)

  let names_list = []
  let part_start = search('^[ \t}]*\<' . a:str . '\>[ \t{]*$', 'bnW')
  if part_start != 0  " found it
    let line = getline(part_start)
    if line !~ '{'
      let part_start = nextnonblank(part_start)
    endif
    call cursor(part_start, 0)
    let part_end = searchpair('{', '', '}', 'nW')
  endif

  let name_num = search('\%(name:\s*\)"\w\+"', 'W', part_end)
  while name_num
    let name = matchstr(getline(name_num), '\%(name:\s*\)\@<="\w\+"')
    call extend(names_list, [name])
    let name_num = search('\%(name:\s*\)"\w\+"', 'W', part_end)
  endwhile
  call cursor(curpos)

  for m in sort(names_list)
    if m =~ '^' . a:base
      call add(a:res, m)
    endif
  endfor
endfunction

function! edccomplete#FindParent(lnum, cnum)
  call setpos('.', [0, a:lnum, a:cnum, 0])
  let ppe = searchpos('\.', 'bcn')
  let pps = searchpos('\w\+\.', 'bcn')
  if ppe != [0, 0] && pps[0] == ppe[0] && pps[1] <= ppe[1] && pps[0] == line('.')
    let b:sparent = line[pps[1] -1 : ppe[1] - 2]
    return
  endif

  let startpos = searchpair('{', '', '}', 'bnW')
  let lnum = startpos
  let line = getline(lnum)

  if line !~ '\a\+'
    let line = getline(prevnonblank(lnum - 1))
  endif

  let b:sparent = matchstr(line, '\w\+')
endfunction

" part
let s:partLabel = {
      \ 'name': 		        '"string"',
      \ 'type':			        '"keyword"',
      \ 'effect':		        '"keyword"',
      \ 'ignore_flags':		        '"keyword" ...',
      \ 'pointer_mode':		        '"keyword"',
      \ 'mouse_events':		        '"bool"',
      \ 'repeat_events':	        '"bool"',
      \ 'scale':	                '"bool"',
      \ 'precise_is_inside':	        '"bool"',
      \ 'use_alternate_font_metrics':	'"bool"',
      \ 'clip_to':		        '"string"',
      \ 'source':		        '"string"',
      \ 'image':		        '"string" "keyword"',
      \ 'font':			        '"string" "string"',
      \ 'entry_mode':		        '"keyword"',
      \ }
let s:partStatement = [
      \ 'dragable',
      \ 'images',
      \ 'fonts',
      \ 'description',
      \ 'styles',
      \ 'color_classes',
      \ 'program',
      \ 'programs',
      \ 'box',
      \ ]

" dragable
let s:dragableLabel = {
      \ 'x':		'"bool" "int" "int"',
      \ 'y':		'"bool" "int" "int"',
      \ 'confine':	'"string"',
      \ 'events':	'"string"',
      \ }

" description
let s:descriptionLabel = {
      \ 'state':		'"string" "float"',
      \ 'inherit':		'"string" "float"',
      \ 'visible':		'"bool"',
      \ 'align':		'"float" "float"',
      \ 'fixed': 		'"float" "float"',
      \ 'min':			'"int" "int"',
      \ 'max':			'"int" "int"',
      \ 'step':			'"int" "int"',
      \ 'aspect':		'"float" "float"',
      \ 'aspect_preference':	'"keyword"',
      \ 'color_class':		'"string"',
      \ 'color':		'"int" "int" "int" "int"',
      \ 'color2':		'"int" "int" "int" "int"',
      \ 'color3':		'"int" "int" "int" "int"',
      \ 'font':			'"string" "string"',
      \ }
let s:descriptionStatement = [
      \ 'rel1',
      \ 'rel2',
      \ 'image',
      \ 'fill',
      \ 'text',
      \ 'gradient',
      \ 'images',
      \ 'fonts',
      \ 'styles',
      \ 'color_classes',
      \ 'program',
      \ 'programs',
      \ 'box',
      \ ]

" rel
let s:relLabel = {
      \ 'relative':	'"float" "float"',
      \ 'offset':	'"int" "int"',
      \ 'to':		'"string"',
      \ 'to_x':		'"string"',
      \ 'to_y':		'"string"',
      \ }

" image
let s:imageStatement = [
      \ 'images',
      \ ]
let s:imageLabel = {
      \ 'image':	'"string" "keyword"',
      \ 'normal':	'"string"',
      \ 'tween':	'"string"',
      \ 'border':	'"int" "int" "int" "int"',
      \ 'middle':	'"bool"',
      \ }

" fill
let s:fillLabel = {
      \ 'smooth':	'"bool"',
      \ 'angle':	'"0-360"',
      \ 'spread':	'"bool"',
      \ }
let s:fillStatement = [
      \ 'origin',
      \ 'size',
      \ ]
" fill origin/size
let s:fillInnerStatement = {
      \ 'relative':	'"float" "float"',
      \ 'offset':	'"int" "int"',
      \ }

" text
let s:textLabel = {
      \ 'text':		'"string"',
      \ 'text_class':	'"string"',
      \ 'font':		'"string"',
      \ 'style':	'"string"',
      \ 'size':		'"int"',
      \ 'fit':		'"bool" "bool"',
      \ 'min':		'"bool" "bool"',
      \ 'max':		'"bool" "bool"',
      \ 'align':	'"float" "float"',
      \ 'elipsis':	'"float"',
      \ 'source':	'"string"',
      \ 'text_source':	'"string"',
      \ }
let s:textStatement = [
      \ 'fonts',
      \ ]

" program
let s:programLabel = {
      \ 'name':		'"string"',
      \ 'signal':	'"string"',
      \ 'source':	'"string"',
      \ 'action':	'"keyword" ...',
      \ 'transition':	'"keyword" "float"',
      \ 'target':	'"string"',
      \ 'after':	'"string"',
      \ 'in':		'"float" "float"',
      \ }
let s:programStatement = [
      \ 'script',
      \ ]


" programs
let s:programsLabel = {
      \ 'image':	'"string" "keyword"',
      \ 'font':		'"string" "string"',
      \ }
let s:programsStatement = [
      \ 'images',
      \ 'fonts',
      \ 'program',
      \ ]

" box and table
let s:boxStatement = [
      \ 'items',
      \ ]
let s:boxItemsStatement = [
      \ 'item',
      \ ]
let s:boxItemLabel = {
      \ 'type':	        '"keyword"',
      \ 'name':	        '"string"',
      \ 'source':	'"string"   # Group name',
      \ 'min':		'"int" "int"',
      \ 'prefer':	'"int" "int"',
      \ 'max':		'"int" "int"',
      \ 'padding':      '"int" "int" "int" "int"',
      \ 'align':	'"float" "float"',
      \ 'weight':	'"float" "float"',
      \ 'aspect':	'"float" "float"',
      \ 'aspect_mode':  '"keyword"',
      \ 'options':      '"string"',
      \ }
let s:boxDescLabel = {
      \ 'layout':       '"string" ["string"]',
      \ 'align':	'"float" "float"',
      \ 'padding':      '"int" "int"',
      \ }
let s:tableItemLabel = {
      \ 'position':	'"int" "int"',
      \ 'span':	        '"int" "int"',
      \ }
let s:tableDescLabel = {
      \ 'homogeneous':	'"keyword"',
      \ 'align':	'"float" "float"',
      \ 'padding':      '"int" "int"',
      \ }

" group
let s:groupLabel = {
      \ 'name':		'"string"',
      \ 'alias':	'"string"',
      \ 'min':		'"int" "int"',
      \ 'max':		'"int" "int"',
      \ 'image':	'"string" "keyword"',
      \ 'font':		'"string" "string"',
      \ }
let s:groupStatement = [
      \ 'data',
      \ 'script',
      \ 'parts',
      \ 'images',
      \ 'fonts',
      \ 'styles',
      \ 'color_classes',
      \ 'program',
      \ 'programs',
      \ ]

" parts
let s:partsStatement = [
      \ 'images',
      \ 'fonts',
      \ 'part',
      \ 'styles',
      \ 'color_classes',
      \ 'program',
      \ 'programs',
      \ ]
let s:partsLabel = {
      \ 'image':	'"string" "keyword"',
      \ 'font':		'"string" "string"',
      \ }

" data
let s:dataLabel = {
      \ 'item':		'"string" "string" ...',
      \ }

" fonts
let s:fontsLabel = {
      \ 'font':		'"string" "string"',
      \ }

"images
let s:imagesLabel = {
      \ 'image':	'"string" "keyword"',
      \ }

"collections
let s:collectionsStatement = [
      \ 'group',
      \ 'images',
      \ 'fonts',
      \ 'styles',
      \ 'color_classes',
      \ ]
let s:collectionsLabel = {
      \ 'image':	'"string" "keyword"',
      \ 'font':		'"string" "string"',
      \ }

" spectra
let s:spectraStatement = [
      \ 'spectrum',
      \ ]
" spectrum
let s:spectrumLabel = {
      \ 'name':		'"string"',
      \ 'color': 	'"int" "int" "int" "int" "int"',
      \ }
" gradient
let s:gradientLabel = {
      \ 'type':		'"string"',
      \ 'spectrum':	'"string"',
      \ }
let s:gradientStatement = [
      \ 'rel1',
      \ 'rel2',
      \ ]

" styles
let s:stylesStatement = [
      \ 'style',
      \ ]
" style
let s:styleLabel = {
      \ 'name':		'"string"',
      \ 'base': 	'"string"',
      \ 'tag': 		'"string"',
      \ }

" color_classes
let s:color_classesStatement = [
      \ 'color_class',
      \ ]
" color_class
let s:color_classLabel = {
      \ 'name':		'"string"',
      \ 'color':	'"int" "int" "int" "int"',
      \ 'color2':	'"int" "int" "int" "int"',
      \ 'color3':	'"int" "int" "int" "int"',
      \ }

" toplevel
let s:topStatement = [
      \ 'fonts',
      \ 'images',
      \ 'data',
      \ 'collections',
      \ 'spectra',
      \ 'styles',
      \ 'color_classes',
      \ ]

" images image storage method
let s:imageStorageMethod = {
      \ 'COMP':		'',
      \ 'RAW':		'',
      \ 'LOSSY':	'0-100',
      \ }

" part types
let s:partTypes = {
      \ 'TEXT':		'',
      \ 'IMAGE':	'',
      \ 'RECT':		'',
      \ 'TEXTBLOCK':	'',
      \ 'SWALLOW':	'',
      \ 'GRADIENT':	'',
      \ 'GROUP':	'',
      \ 'BOX':	        '',
      \ 'TABLE':        '',
      \ }
" part effects
let s:partEffects = {
      \ 'NONE':			'',
      \ 'PLAIN':		'',
      \ 'OUTLINE':		'',
      \ 'SOFT_OUTLINE':		'',
      \ 'SHADOW':		'',
      \ 'SOFT_SHADOW':		'',
      \ 'OUTLINE_SHADOW':	'',
      \ 'OUTLINE_SOFT_SHADOW':	'',
      \ 'FAR_SHADOW':	'',
      \ 'FAR_SOFT_SHADOW':	'',
      \ 'GLOW':	'',
      \ }
" part ignore flags 
let s:partIgnoreFlags = {
      \ 'NONE':	        '',
      \ 'ON_HOLD':	'',
      \ }
" part pointer mode
let s:partPointerMode = {
      \ 'AUTOGRAB':     '',
      \ 'NOGRAB':	'',
      \ }
" part editable_mode
let s:partEditableMode = {
      \ 'NONE':		'',
      \ 'PLAIN':	'',
      \ 'EDITABLE':	'',
      \ 'PASSWORD':	'',
      \ }

" aspect_preference types
let s:aspectPrefTypes = {
      \ 'VERTICAL':	'',
      \ 'HORIZONTAL':	'',
      \ 'BOTH':		'',
      \	}

" program transition types
let s:transitionTypes = {
      \ 'LINEAR':	'0.0 - 1.0',
      \ 'SINUSOIDAL':	'0.0 - 1.0',
      \ 'ACCELERATE':	'0.0 - 1.0',
      \ 'DECELERATE':	'0.0 - 1.0',
      \ }
" program action types
let s:actionTypes = {
      \ 'STATE_SET':		'"string" "0.0 - 1.0"',
      \ 'ACTION_STOP':		'',
      \ 'SIGNAL_EMIT':		'"string" "string"',
      \ 'DRAG_VAL_SET':		'"float" "float"',
      \ 'DRAG_VAL_STEP':	'"float" "float"',
      \ 'DRAG_VAL_PAGE':	'"float" "float"',
      \ 'FOCUS_SET':	        '',
      \ }
" box item types
let s:boxItemTypes = {
      \ 'GROUP':	'',
      \ }
" box item aspect mode
let s:boxItemAspectMode = {
      \ 'NONE':	        '',
      \ 'NEITHER':	'',
      \ 'VERTICAL':	'',
      \ 'HORIZONTAL':	'',
      \ 'BOTH':		'',
      \	}
" box layout
let s:boxLayout = {
      \ '"horizontal"':	        '',
      \ '"horizontal_homogeneous"':	'',
      \ '"horizontal_max"':	'',
      \ '"horizontal_flow"':	'',
      \ '"vertical"':	        '',
      \ '"vertical_homogeneous"':	'',
      \ '"vertical_max"':	'',
      \ '"vertical_flow"':	'',
      \ '"stack"':		'',
      \	}
" table homogeneous mode
let s:tableHomogeneousMode = {
      \ 'NONE':		'',
      \ 'TABLE':	'',
      \ 'ITEM':		'',
      \	}
syntax/embryo.vim	[[[1
195
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

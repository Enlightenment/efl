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
      elseif line =~ 'select_mode:\s*'
	call edccomplete#AddKeyword(res, a:base, s:partSelectMode)
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

    elseif b:scontext == 'map'
      call edccomplete#AddLabel(res, line, a:base, s:mapLabel)
      call edccomplete#AddStatement(res, line, a:base, s:mapStatement)

    elseif b:scontext == 'rotation'
      call edccomplete#AddLabel(res, line, a:base, s:rotationLabel)

    elseif b:scontext == 'perspective'
      call edccomplete#AddLabel(res, line, a:base, s:perspectiveLabel)

    elseif b:scontext == 'params'
      call edccomplete#AddLabel(res, line, a:base, s:paramsLabel)

    elseif b:scontext == 'image'
      call edccomplete#AddLabel(res, line, a:base, s:imageLabel)
      if line =~ 'image:\s*".\{-}"'
        call edccomplete#AddKeyword(res, a:base, s:imageStorageMethod)
      elseif line =~ 'middle:\s*'
        call edccomplete#AddKeyword(res, a:base, s:imageMiddleTypes)
      elseif line =~ 'scale_hint:\s*'
        call edccomplete#AddKeyword(res, a:base, s:imageScaleHint)
      endif

    elseif b:scontext == 'fill'
      call edccomplete#AddLabel(res, line, a:base, s:fillLabel)
      call edccomplete#AddStatement(res, line, a:base, s:fillStatement)
      if line =~ 'type:\s*'
	call edccomplete#AddKeyword(res, a:base, s:fillTypes)
      endif

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
      if line =~ 'type:\s*'
	call edccomplete#AddKeyword(res, a:base, s:gradientTypes)
      endif

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

    elseif b:scontext == 'externals'
      call edccomplete#AddLabel(res, line, a:base, s:externalsLabel)

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
      \ 'name': 		        '"name"',
      \ 'type':			        'keyword',
      \ 'effect':		        'keyword',
      \ 'clip_to':		        '"part_name"',
      \ 'scale':	                '0-1',
      \ 'mouse_events':		        '0-1',
      \ 'repeat_events':	        '0-1',
      \ 'ignore_flags':		        'keyword ...',
      \ 'pointer_mode':		        'keyword',
      \ 'select_mode':		        'keyword',
      \ 'precise_is_inside':	        '0-1',
      \ 'use_alternate_font_metrics':	'0-1',
      \ 'image':	                '"filename" keyword',
      \ 'font':		                '"filename" "name"',
      \ 'entry_mode':		        'keyword',
      \ 'multiline':    	        '0-1 (TEXTBLOCK only)',
      \ 'source':		        '"group_name" (GROUP or TEXTBLOCK only)',
      \ 'source2':		        '"group_name" (TEXTBLOCK only)',
      \ 'source3':		        '"group_name" (TEXTBLOCK only)',
      \ 'source4':		        '"group_name" (TEXTBLOCK only)',
      \ 'source5':		        '"group_name" (TEXTBLOCK only)',
      \ 'source6':		        '"group_name" (TEXTBLOCK only)',
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
      \ 'x':		'0-1 int int',
      \ 'y':		'0-1 int int',
      \ 'confine':	'"part_name"',
      \ 'events':	'"draggable_part_name"',
      \ }

" description
let s:descriptionLabel = {
      \ 'state':		'"name" index (float)',
      \ 'inherit':		'"description" index (float)',
      \ 'visible':		'0-1',
      \ 'align':		'x y (float)',
      \ 'fixed': 		'width height (0-1)',
      \ 'min':			'width height (int)',
      \ 'max':			'width height (int)',
      \ 'step':			'width height (int)',
      \ 'aspect':		'min max (float)',
      \ 'aspect_preference':	'keyword',
      \ 'color_class':		'"name"',
      \ 'color':	        '0-255 0-255 0-255 0-255',
      \ 'color2':	        '0-255 0-255 0-255 0-255',
      \ 'color3':	        '0-255 0-255 0-255 0-255',
      \ 'font': 		'"filename" "name"',
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
      \ 'map',
      \ ]

" rel
let s:relLabel = {
      \ 'relative':	'x y (float)',
      \ 'offset':	'x y (int)',
      \ 'to':		'"part_name"',
      \ 'to_x':		'"part_name"',
      \ 'to_y':		'"part_name"',
      \ }
" map
let s:mapLabel = {
      \ 'on':		'0-1',
      \ 'perspective':	'"part_name"',
      \ 'light':	'"part_name"',
      \ 'smooth':	'0-1',
      \ 'pespective_on':'0-1',
      \ 'backface_cull':'0-1',
      \ 'alpha':	'0-1',
      \ }
let s:mapStatement = [
      \ 'rotation',
      \ ]

let s:rotationLabel = {
      \ 'center':	'"part_name"',
      \ 'x':	        '"degrees (float)"',
      \ 'y':	        '"degrees (float)"',
      \ 'z':	        '"degrees (float)"',
      \ }

" params
let s:paramsLabel = {
      \ 'int':	        '"name" int',
      \ 'double':       '"name" double',
      \ 'string':       '"name" "string"',
      \ }

" perspective
let s:perspectiveLabel = {
      \ 'zplane':	'int',
      \ 'focal':        'int',
      \ }


" image
let s:imageLabel = {
      \ 'image':	'"filename" keyword',
      \ 'normal':	'"filename"',
      \ 'tween':	'"filename"',
      \ 'border':	'left right top bottom (int)',
      \ 'middle':	'keyword',
      \ 'border_scale': '0-1',
      \ 'scale_hint':	'keyword',
      \ }

" fill
let s:fillLabel = {
      \ 'smooth':	'0-1',
      \ 'angle':	'0-360 (GRADIENT)',
      \ 'spread':	'0-1',
      \ 'type':	        'keyword',
      \ }
let s:fillStatement = [
      \ 'origin',
      \ 'size',
      \ ]
" fill origin/size
let s:fillInnerStatement = {
      \ 'relative':	'width height (float)',
      \ 'offset':	'x y (int)',
      \ }
" fill types
let s:fillTypes = {
      \ 'SCALE':    '',
      \ 'TILE':	    '',
      \ }

" text
let s:textLabel = {
      \ 'text':		'"string"',
      \ 'font':		'"font_name"',
      \ 'size':		'size (int)',
      \ 'text_class':	'"class_name"',
      \ 'fit':		'x y (0-1)',
      \ 'min':		'x y (0-1)',
      \ 'max':		'x y (0-1)',
      \ 'align':	'x y (float)',
      \ 'source':	'"part_name"',
      \ 'text_source':	'"text_part_name"',
      \ 'style':	'"style_name"',
      \ 'elipsis':	'0.0-1.0',
      \ 'repch':	'"string" (PASSWORD mode)',
      \ }
let s:textStatement = [
      \ 'fonts',
      \ ]

" program
let s:programLabel = {
      \ 'name':		'"name"',
      \ 'signal':	'"signal_name"',
      \ 'source':	'"part_name"',
      \ 'action':	'keyword ...',
      \ 'transition':	'keyword time (float)',
      \ 'filter':	'"part_name" "state_name"',
      \ 'in':		'from range (float)',
      \ 'target':	'"part_name"',
      \ 'after':	'"program_name"',
      \ }
let s:programStatement = [
      \ 'script',
      \ 'lua_script',
      \ ]


" programs
let s:programsLabel = {
      \ 'image':	'"filename" keyword',
      \ 'font':		'"filename" "name"',
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
      \ 'type':	        'keyword',
      \ 'name':	        '"name"',
      \ 'source':	'"group_name"',
      \ 'min':		'width height (int)',
      \ 'prefer':	'width height (int)',
      \ 'max':		'width height (int)',
      \ 'padding':      'left right top bottom (int)',
      \ 'align':	'x y (float)',
      \ 'weight':	'x y (float)',
      \ 'aspect':	'w h (float)',
      \ 'aspect_mode':  'keyword',
      \ 'options':      '"extra options"',
      \ }
let s:boxDescLabel = {
      \ 'layout':       '"string" ["string"]',
      \ 'align':	'float float',
      \ 'padding':      'int int',
      \ }
let s:tableItemLabel = {
      \ 'position':     'col row (int)',
      \ 'span':	        'col row (int)',
      \ }
let s:tableDescLabel = {
      \ 'homogeneous':	'keyword',
      \ 'align':	'float float',
      \ 'padding':      'int int',
      \ }

" group
let s:groupLabel = {
      \ 'name':		'"name"',
      \ 'alias':	'"alias"',
      \ 'min':		'width height',
      \ 'max':		'width height',
      \ 'image':	'"filename" keyword',
      \ 'font':		'"filename" "name"',
      \ 'script_only':	'0-1',
      \ }
let s:groupStatement = [
      \ 'data',
      \ 'script',
      \ 'lua_script',
      \ 'parts',
      \ 'images',
      \ 'fonts',
      \ 'styles',
      \ 'color_classes',
      \ 'program',
      \ 'programs',
      \ 'externals',
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
      \ 'image':	'"filename" keyword',
      \ 'font':		'"filename" "name"',
      \ }

" data
let s:dataLabel = {
      \ 'item':		'"key" "value"',
      \ 'file':		'"key" "filename"',
      \ }

" fonts
let s:fontsLabel = {
      \ 'font':		'"filename" "name"',
      \ }

"images
let s:imagesLabel = {
      \ 'image':	'"filename" keyword',
      \ }

"collections
let s:collectionsStatement = [
      \ 'group',
      \ 'images',
      \ 'fonts',
      \ 'styles',
      \ 'color_classes',
      \ 'externals',
      \ ]
let s:collectionsLabel = {
      \ 'image':	'"filename" keyword',
      \ 'font':		'"filename" "name"',
      \ }

" externals
let s:externalsLabel = {
      \ 'external':		'"name"',
      \ }

" spectra
let s:spectraStatement = [
      \ 'spectrum',
      \ ]
" spectrum
let s:spectrumLabel = {
      \ 'name':		'"name"',
      \ 'color':	'0-255 0-255 0-255 0-255',
      \ }
" gradient
let s:gradientLabel = {
      \ 'type':		'"keyword"',
      \ 'spectrum':	'"spectrum_name"',
      \ }
let s:gradientStatement = [
      \ 'rel1',
      \ 'rel2',
      \ ]
" gradient types
let s:gradientTypes = {
      \ '"linear"':		'',
      \ '"radial"':		'',
      \ '"rectangular"':	'',
      \ '"angular"':		'',
      \ '"sinusoidal"':		'',
      \ }

" styles
let s:stylesStatement = [
      \ 'style',
      \ ]
" style
let s:styleLabel = {
      \ 'name':		'"name"',
      \ 'base': 	'".. default style properties .."',
      \ 'tag': 		'"tagname" "style properties"',
      \ }

" color_classes
let s:color_classesStatement = [
      \ 'color_class',
      \ ]
" color_class
let s:color_classLabel = {
      \ 'name':		'"name"',
      \ 'color':	'0-255 0-255 0-255 0-255',
      \ 'color2':	'0-255 0-255 0-255 0-255',
      \ 'color3':	'0-255 0-255 0-255 0-255',
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
      \ 'externals',
      \ ]

" images image storage method
let s:imageStorageMethod = {
      \ 'COMP':		'',
      \ 'RAW':		'',
      \ 'USER':		'',
      \ 'LOSSY':	'0-100',
      \ }
" image middle types
let s:imageMiddleTypes = {
      \ '0':		'',
      \ '1':		'',
      \ 'NONE':		'',
      \ 'DEFAULT':	'',
      \ 'SOLID':	'',
      \ }
" image scale hint
let s:imageScaleHint = {
      \ '0':		'',
      \ 'NONE':		'',
      \ 'DYNAMIC':	'',
      \ 'STATIC':	'',
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
      \ 'EXTERNAL':     '',
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
" part select_mode
let s:partSelectMode = {
      \ 'DEFAULT':		'',
      \ 'EXPLICIT':		'',
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
      \ 'DRAG_VAL_SET':		'float float',
      \ 'DRAG_VAL_STEP':	'float float',
      \ 'DRAG_VAL_PAGE':	'float float',
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

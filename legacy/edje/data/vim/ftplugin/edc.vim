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

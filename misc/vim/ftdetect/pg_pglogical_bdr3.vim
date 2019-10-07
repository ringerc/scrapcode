" Loaded via filetype plugin
"
" see :help filetype
"
" No need for autocmd group, done automatically

" Shared settings variables for filetype cmds
"
" These two control which sources Syntastic, CScope, etc look at
" when working on projects that use these as dependencies.
let b:pgver="2Qpg11"
let b:pglver="pglogical3-master"

let s:pgl3make="pgl3build 2>&1 2Qpg11 -w all install"
let s:pgl3check="pgl3build 2>&1 2Qpg11 -w all install check"
let s:bdr3make="bdr3build 2>&1 2Qpg11 -w all install"
let s:bdr3check="bdr3build 2>&1 2Qpg11 -w all install check"

let s:pgsrcdir = $HOME . "/projects/2Q/" . b:pgver
let s:pglsrcdir = $HOME . "/projects/2Q/" . b:pglver

" include-dir list for postgres version selected
"let b:pgincludes = s:pgdir . "/include"
"let b:pgincludes += s:pgdir . "/include"

" We check in the sourcetree not the installed tree because we don't want to
" find installed pglogical sources etc.
"
" We look for src/include/port/ but not its children so we don't get muddled
" by win32 wrappers etc.
"
function s:get_pg_INCLUDEDIRS()
	return systemlist("find " . s:pgsrcdir . " -path '*/include/port/*' -printf '%h\n' -prune -o  -type f -name \\*.h -printf '%h\n' | sort -u ")
endfunction

" Similarly for pgl3 we look in its source dir. Not bothering with compat
" dirs.
function s:get_pgl_INCLUDEDIRS()
	return s:get_pg_INCLUDEDIRS() + [ s:pglsrcdir ]
endfunction

" Set the make command to use for pgl3 and bdr3 when you run :make
autocmd BufRead,BufNewFile */pglogical*/*.[ch] let &makeprg=s:pgl3make
autocmd BufRead,BufNewFile */bdr3*/*.[ch] let &makeprg=s:bdr3make

" INCLUDEDIRS is set for cscope, which we use in the F5 binding,
" so it looks in project dependency directories too.

" Per filetype and/or directory settings
autocmd BufRead,BufNewFile */bdr*/\(*.[ch]\|*.cpp\|*.pl\|*.pm\)
			\ set tabstop=4 shiftwidth=4 noexpandtab autoindent |
			\ let g:syntastic_c_include_dirs=s:get_pgl_INCLUDEDIRS() |
			\ let g:syntastic_cpp_include_dirs=s:get_pgl_INCLUDEDIRS() |
			\ let $INCLUDEDIRS=join(s:get_pgl_INCLUDEDIRS(), ':')
autocmd BufRead,BufNewFile */pglogical*/*.[ch]
			\ set tabstop=4 shiftwidth=4 noexpandtab autoindent |
			\ let g:syntastic_c_include_dirs=s:get_pg_INCLUDEDIRS() |
			\ let g:syntastic_cpp_include_dirs=s:get_pg_INCLUDEDIRS() |
			\ let $INCLUDEDIRS=join(s:get_pg_INCLUDEDIRS(), ':')
autocmd BufRead,BufNewFile */pg*/*.[ch]
			\ set tabstop=4 shiftwidth=4 noexpandtab autoindent
autocmd BufRead,BufNewFile */postgres/*.pl
			\ set tabstop=4 shiftwidth=4 expandtab autoindent

autocmd BufRead,BufNewFile */pgjdbc/*.java
			\ set tabstop=2 shiftwidth=2 expandtab autoindent

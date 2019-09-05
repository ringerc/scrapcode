" Shared settings variables for filetype cmds
let pgver="2Qpg11"
let pgl3make="pgl3build 2Qpg11 all install"
let pgl3check="pgl3build 2Qpg11 all install check"
let bdr3make="bdr3build 2Qpg11 all install"
let bdr3check="bdr3build 2Qpg11 all install check"

" make targets for :make command
" note use of 'let &settingname' to access settings as var
autocmd BufRead,BufNewFile */pglogical*/*.[ch] let &makepgr=pgl3make
autocmd BufRead,BufNewFile */bdr3*/*.[ch] let &makeprg=bdr3make

" Per filetype and/or directory settings
autocmd BufRead,BufNewFile */bdr*/\(*.[ch]\|*.cpp\|*.pl\|*.pm\) set tabstop=4 shiftwidth=4 noexpandtab autoindent
autocmd BufRead,BufNewFile */pg*/*.[ch] set tabstop=4 shiftwidth=4 noexpandtab autoindent
autocmd BufRead,BufNewFile */pg*/*.cpp set tabstop=4 shiftwidth=4 noexpandtab autoindent
autocmd BufRead,BufNewFile */postgres/*.pl set tabstop=4 shiftwidth=4 expandtab autoindent

autocmd BufRead,BufNewFile */pgjdbc/*.java set tabstop=2 shiftwidth=2 expandtab autoindent

autocmd BufRead,BufNewFile *.groovy set tabstop=2 shiftwidth=2 expandtab autoindent smartindent
autocmd BufRead,BufNewFile Jenkinsfile* set tabstop=2 shiftwidth=2 expandtab autoindent smartindent

" Disable with :nolist enable with :list
autocmd BufRead,BufNewFile * set listchars=trail:·,precedes:«,extends:»,tab:▸\ 
" Dim listchars, also affects control-char display
autocmd BufRead,BufNewFile * hi SpecialKey ctermfg=19 guifg=#0000af

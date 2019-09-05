
	" Disable with :nolist enable with :list
	autocmd BufRead,BufNewFile *
				\ set listchars=trail:·,precedes:«,extends:»,tab:▸\ 
	" Dim listchars, also affects control-char display
	autocmd BufRead,BufNewFile *
				\ hi SpecialKey ctermfg=19 guifg=#0000af

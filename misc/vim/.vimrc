set exrc
set secure
set nocompatible

" Use , instead of \ for the <Leader> key
let mapleader = ","

" always use cscope as well as ctags if both present
set cscopetag

execute pathogen#infect()
filetype plugin indent on

" syntastic plugin
set statusline+=%#warningmsg#
set statusline+=%{SyntasticStatuslineFlag()}
set statusline+=%*
let g:syntastic_always_populate_loc_list = 1
let g:syntastic_auto_loc_list = 1
let g:syntastic_check_on_open = 1
let g:syntastic_check_on_wq = 0
" Config for C compilers is stored in .syntastic_c_config
" See https://github.com/scrooloose/syntastic/wiki/C:---gcc 
" One line per -I directive or other command

" localvimrc plugin
let g:localvimrc_sandbox = 0
let g:localvimrc_whitelist = "/home/craig/projects/.*"

" Vim's wildcard ignore for uninteresting files
set wildignore+=*/tmp/*,*.so,*.sw?,*.zip,*.tar,*.tgz,*.tar.gz,*.o,*/.git/*

" ctrlp config
let g:ctrlp_extensions = ['tag', 'mixed']

" Use the nearest revision control dir as the ctrlp root
let g:ctrlp_working_path_mode = 'r'

" Easy bindings for its various modes
nmap <F2> :CtrlPBuffer<cr>
nmap <F3> :CtrlPMixed<cr>

" Allow buffers to be hidden with changes unwritten
set hidden

" In the buffer menu use menu expansion for tab
set wildchar=<Tab> wildmenu wildmode=full

" Open buffer menu with F7
set wildcharm=<C-Z>
nnoremap <F7> :b <C-Z>

" Enable the list of buffers
let g:airline#extensions#tabline#enabled = 1
" Show just the filename
let g:airline#extensions#tabline#fnamemod = ':t'

" Page up for next buffer
nmap <A-PageUp> :bnext<CR>
" Page down for previous buffer
nmap <A-PageDown> :bprevious<CR>
" Home for new blank buffer
nmap <A-Home> :enew<CR>
" End for close buffer
nmap <A-End> :bp <BAR> bd #<CR>


" Do not put filetype autocmds here. Put them in ~/.vim/filetype.vim

" Highlight extra whitespace
" see http://vim.wikia.com/wiki/Highlight_unwanted_spaces
autocmd ColorScheme * highlight ExtraWhitespace ctermbg=red guibg=red
" Show trailing whitepace and spaces before a tab:
autocmd Syntax * syn match ExtraWhitespace /\s\+$\| \+\ze\t/

nmap <F8> :TagbarToggle<CR>

colorscheme molokai

" Use pgsql syntax for all .sql files
let g:sql_type_default = 'pgsql'

" Based on Otavio Cordeiro's vimrc.
set nocompatible

" Recognize file types
filetype plugin indent on

" Show spaces and tabs
" set listchars=tab:»·,trail:·
" set list!

" Hides the mouse pointer when typing
" set mousehide

" Replace TABs by spaces
" set expandtab

" Status bar
set laststatus=2

" Auto-complete
function Cfunctions()
	imap ( ()<Left>
	imap [ []<Left>
	imap { {}<Left>
"	imap { {<CR>}<Up><End><CR>
endfunction

function Texfunctions()
	imap ( ()<Left>
	imap [ []<Left>
	imap { {}<Left>
	imap \itemi \begin{itemize<RIGHT><CR><Tab>\item <CR><LEFT>\end{itemize<UP><END>
endfunction

function InsertTabWrapper()
	let col = col('.') - 1
			if !col || getline('.')[col - 1] !~ '\k'
		return "\<tab>"
	else
		return "\<c-p>"
	endif
endfunction

" Auto-brackets for some languages:
" au FileType c,cpp,h,java,perl,python call Cfunctions()
" au FileType tex,bib call Texfunctions()

" Turn Tab into auto-completion.
" inoremap <tab> <c-r>=InsertTabWrapper()<cr>


" Resizes the window of the current buffer
if bufwinnr(1)
	map + <C-W>+
	map - <C-W>-
endif


" Enumerate the lines
" set number
set ruler

" Removes the menu from graphical mode (gvim)
" set guioptions-=T

set ts=4
set sw=4
set bs=2                " allow backspacing over everything in insert mode
set ai                  " always set autoindenting on
set nobackup            " do not keep a backup file, use versions instead
set viminfo='20,\"50    " read/write a .viminfo file, don't store more
                        " than 50 lines of registers
set history=50          " keep 50 lines of command line history
map Q gq                " Don't use Ex mode, use Q for formatting

set filetype=c
augroup cprog
" Remove all cprog autocommands
au!

" When starting to edit a file:
"   For C and C++ files set formatting of comments and set C-indenting on.
"   For other files switch it off.
"   Don't change the order, it's important that the line with * comes first.
autocmd FileType *       set formatoptions=tcql nocindent expandtab comments&
autocmd FileType c,cpp   set formatoptions=croql cindent noexpandtab comments=sr:/*,mb:*,el:*/,://
autocmd FileType python  set formatoptions=croql cindent noexpandtab comments=sr:/*,mb:*,el:*/,://
autocmd BufNewFile *.java exe "normal Opublic class " .  expand('%:t:r') . "\n{\n}\<Esc>1G"
augroup END

syntax on
set hlsearch

" Make p in Visual mode replace the selected text with the "" register.
vnoremap p <Esc>:let current_reg = @"<CR>gvdi<C-R>=current_reg<CR><Esc>

if &t_Co > 2 || has("gui_running")
"  set guifont=-adobe-courier-medium-r-normal-*-*-120-*-*-m-*-*
"  set guifont=-misc-fixed-bold-r-normal-*-*-120-*-*-c-*-iso8859-1
  set guifont=Bitstream\ Vera\ Sans\ Mono\ Bold\ 10
  syntax on
  set hlsearch
endif

" Set background to blue
" :colors blue

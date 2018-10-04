"
" this file is an example .vimrc for the vim editor.  it should be in your home directory.
"
" also, this config uses the elflord color scheme.  this may require installing the full
" vim package, e.g.: sudo apt install vim

" cranks up the register saved buffer size to 1000 lines, since otherwise the cut buffer will only
" preserve the default 50 lines for a paste to another file.  this makes all registers go up to
" this size though, so if you save to a lot of different ones, you will use more space in your
" viminfo file.
set viminfo='100,<1000,s10,h

" tells vim not to make a backup of the file before editing it; this is living dangerously.
set nobk

" nocompatible|nocp turns off compatibility mode, allowing filename completion.
set nocompatible

" elflord just happens to be the color scheme i prefer.  there are a bunch.
colorscheme elflord

" map the normal copy, cut and paste keys (ctrl-c, ctrl-x & ctrl-v) for vim.
nmap <C-V> "+gP
imap <C-V> <ESC><C-V>i
vmap <C-C> "+y 
vmap <C-X> "+c

" map ctrl-a to select all of the file.  ctrl-i increments, whatever that is.
noremap <C-I> <C-A>
noremap <C-A> gggH<C-O>G
inoremap <C-A> <C-O>gg<C-O>gH<C-O>G
cnoremap <C-A> <C-C>gggH<C-O>G
onoremap <C-A> <C-C>gggH<C-O>G
snoremap <C-A> <C-C>gggH<C-O>G
xnoremap <C-A> <C-C>ggVG

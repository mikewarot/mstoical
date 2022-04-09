" Onyx filetype detection


augroup filetypedetect
  au! BufRead,BufNewFile *.stl		setfiletype stoical
  au! BufRead,BufNewFile *.stc		setfiletype stoical
augroup END


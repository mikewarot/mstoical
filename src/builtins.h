
/* STOICAL
 * Copyright (C) 2002 Jonathan Moore Liles. <wantingwaiting@users.sf.net>
 *
 * STOICAL is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * STOICAL is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with STOICAL; see the file COPYING.  If not,write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/* Mapping of words to code adresses to be added to the initial vocabulary.
 * This file may be automatically generated. */

struct voc_item builtins[] = {
#ifdef THREADS
{ A_WRD,"thread",	adr(thread) },
{ A_WRD,"detach",	adr(detach) },
{ A_WRD,"join",	adr(join) },
{ A_WRD,"me",	adr(me) },
{ A_WRD,"cancel",	adr(cancel) },
{ A_WRD,"shared",adr(shared) },
#endif
{ A_WRD,"delay",	adr(delay) },
{ A_WRD,"exit",	adr(exit) },
{ A_WRD,"abort",	adr(abort) },
{ A_WRD,"system", adr(system) },
{ A_WRD,"<env", adr(left_angle_env) },
{ A_WRD,"env>", adr(env_right_angle) },

{ A_IMM,"stoical<",adr(stoical) },
{ A_WRD,"+check", adr(plus_check) },
{ A_WRD,"-check", adr(minus_check) },

{ A_WRD,"check",	adr(check) },
{ A_WRD,"rtn",	adr(rtn) },
{ A_WRD,"bye",	adr(bye) },
{ A_WRD,"undefined0",adr(undefined) },

{ A_WRD,"mkdir",	adr(mkdir) },
{ A_WRD,"rmdir",	adr(rmdir) },
{ A_WRD,"chdir",	adr(chdir) },
{ A_WRD,"unlink",	adr(unlink) },
{ A_WRD,"mkfifo",	adr(mkfifo) },
{ A_WRD,"umask",	adr(umask) },

{ A_WRD,"open",	adr(open) },
{ A_WRD,"close",  adr(close) },
{ A_WRD,"read",	adr(read) },
{ A_WRD,"readln",	adr(readln) },
{ A_WRD,"write",	adr(write) },
{ A_WRD,"writeln",adr(writeln) },
{ A_WRD,"seek",adr(seek) },
{ A_WRD,"flush",	adr(flush) },
{ A_WRD,"stat",	adr(stat) },

{ A_WRD,"tib",	adr(tib) },
{ A_WRD,"pad",	adr(pad) },

{ A_WRD,"opendir",	adr(opendir) },
{ A_WRD,"closedir",	adr(closedir) },
{ A_WRD,"readdir",	adr(readdir) },

{ A_WRD,"socket", adr(socket) },
{ A_WRD,"bind", adr(bind) },
{ A_WRD,"listen", adr(listen) },
{ A_WRD,"accept", adr(accept) },
{ A_WRD,"connect", adr(connect) },
{ A_WRD,"shutdown", adr(shutdown) },
#ifdef HAVE_SYS_SENDFILE_H
{ A_WRD,"sendfile", adr(sendfile) },
#endif

{ A_WRD,"rand",	adr(rand) },
{ A_WRD,"srand",	adr(srand) },
{ A_WRD,"cells",	adr(cells) },
{ A_WRD,"mark",	adr(mark) },
{ A_WRD,"mark?",	adr(mark_query) },
{ A_WRD,"exec",	adr(exec) },
{ A_WRD,"self",	adr(self) },
{ A_WRD,"redo",	adr(redo) },


{ A_WRD,"string",	adr(string) },
{ A_WRD,"int",	adr(int) },
{ A_WRD,"abs",	adr(abs) },
{ A_WRD,"bool",	adr(bool) },
{ A_WRD,"iliteral",adr(iliteral) },
{ A_WRD,"radix",	adr(radix) },
{ A_WRD,"#match",	adr(hash_match) },
{ A_WRD,"args[",	adr(args_bracket) },
{ A_WRD,"load",	adr(load) },
{ A_WRD,"include",adr(include) },
{ A_WRD,";f",	adr(semicolon_f) },

{ A_WRD,"immediate",adr(immediate) },
{ A_WRD,"inspect", adr(inspect) },
{ A_WRD,"decompile", adr(decompile) },
{ A_WRD,"stack",	adr(stack) },
{ A_WRD,"lstack",	adr(lstack) },
{ A_WRD,"vstack",	adr(vstack) },

{ A_WRD,".r",	adr(dot_r) },
{ A_WRD,".l",	adr(dot_l) },
{ A_WRD,".c",	adr(dot_c) },
{ A_WRD,".v",	adr(dot_v) },
{ A_WRD,".d",	adr(dot_d) },

{ A_WRD,"(get)",adr(_get) },
{ A_WRD,"(put)",adr(_put) },
{ A_WRD,"put",adr(put) },
{ A_WRD,"get",adr(get) },
{ A_WRD,"ttyput",adr(ttyput) },
{ A_WRD,"ttyget",adr(ttyget) },
{ A_WRD,"(getln)",adr(_getln) },
{ A_WRD,"(putln)",adr(_putln) },
{ A_WRD,"putln",adr(putln) },
{ A_WRD,"getln",adr(getln) },
{ A_WRD,"ttyputln",adr(ttyputln) },
{ A_WRD,"ttygetln",adr(ttygetln) },


{ A_WRD,"vocab",	adr(vocab) },
{ A_WRD,"ttyin",	adr(ttyin) },
{ A_WRD,"ttyout", adr(ttyout) },
{ A_WRD,"count",	adr(count) },
{ A_WRD,"stype",	adr(stype) },
{ A_WRD,"type",	adr(type) },
{ A_WRD,"msg",	adr(msg) },


{ A_WRD,"<#",	adr(left_angle_hash) },
{ A_WRD,"#ptr",	adr(hash_ptr) },
{ A_WRD,"#cnt",	adr(hash_cnt) },
{ A_WRD,"#put",	adr(hash_put) },
{ A_WRD,"#a",	adr(hash_a) },
{ A_WRD,"#>",	adr(hash_right_angle) },

{ A_WRD,"swap",   adr(swap) },
{ A_WRD,"dup",	adr(dup) },
{ A_WRD,"drop",   adr(drop) },
{ A_WRD,"over",   adr(over) },
{ A_WRD,"nip",	adr(nip) },
{ A_WRD,"-rot",	adr(minus_rot) },
{ A_WRD,"+rot",	adr(plus_rot) },
{ A_WRD,"flip",	adr(flip) },
{ A_WRD,"tuck",	adr(tuck) },
{ A_WRD,"idup",	adr(idup) },
{ A_WRD,"idrop",	adr(idrop) },

{ A_IMM,"^",	adr(circumflex) },
{ A_IMM,"#!",	adr(comment) },
{ A_IMM,"%",	adr(comment) },
{ A_IMM,"%%",	adr(print) },

{ A_WRD,"1-",	adr(one_minus) },
{ A_WRD,"1+",	adr(one_plus) },
{ A_WRD,"0",	adr(zero) },
{ A_WRD,"2",	adr(two) },
{ A_WRD,"1",	adr(one) },
{ A_WRD,"add",	adr(add) },
{ A_WRD,"cat",	adr(cat) },
{ A_WRD,"-",	adr(sub) },
{ A_WRD,"/",	adr(div) },
{ A_WRD,"*",	adr(mul) },
{ A_WRD,"mod",	adr(mod) },
{ A_WRD,"/mod",	adr(divmod) },

{ A_WRD,"retype",	adr(retype) },
{ A_WRD,"@",	adr(at) },
{ A_WRD,"!",	adr(bang) },

{ A_WRD,"]@",	adr(bracket_at) },
{ A_WRD,"]!",	adr(bracket_bang) },
{ A_WRD,"]push",	adr(bracket_push) },
{ A_WRD,"]pop",	adr(bracket_pop) },
{ A_WRD,"]delete",adr(bracket_delete) },
{ A_WRD,"]insert",adr(bracket_insert) },

{ A_WRD,")@",	adr(paren_at) },
{ A_WRD,")!",	adr(paren_bang) },
{ A_WRD,")delete",adr(paren_delete) },
{ A_WRD,"()do)",adr(_paren_do) },
{ A_WRD,"(){each)",adr(_paren_brace_each) },
{ A_IMM,"){each",adr(paren_brace_each) },

{ A_WRD,"eqz",	adr(eqz) },
{ A_WRD,"ltz",	adr(ltz) },

{ A_WRD,"feq",	adr(feq) },
{ A_WRD,"$eq",	adr(dolar_eq) },

{ A_WRD,"fne",	adr(fne) },
{ A_WRD,"$ne",	adr(dolar_ne) },

{ A_WRD,"lt",	adr(lt) },
{ A_WRD,"gt",	adr(gt) },
{ A_WRD,"ge",	adr(ge) },
{ A_WRD,"le",	adr(le) },
{ A_WRD,"xor",	adr(xor) },
{ A_WRD,"or",	adr(or) },
{ A_WRD,"and",	adr(and) },
{ A_WRD,"not",	adr(not) },
{ A_WRD,"eoc",	adr(eoc) },
{ A_WRD,"eol",	adr(eol) },
{ A_WRD,"disregard",adr(disregard) },
{ A_WRD,"warnings",adr(warnings) },
{ A_WRD,"current",adr(current) },
{ A_WRD,"branch", adr(branch) },
{ A_WRD,"definitions", adr(definitions) },
{ A_WRD,"l()",	adr(l_) },
#ifdef REGEX
{ A_WRD,"rs()",	adr(rs_) },
{ A_WRD,"r()",	adr(r_) },
{ A_IMM,"x",	adr(x) },
#endif
{ A_WRD,"address",adr(address) },
{ A_IMM,"()",	adr(_) },
{ A_IMM,"ascii",	adr(ascii) },
{ A_IMM,"//",	adr(fslash_fslash) },

{ A_WRD,"eval",	adr(eval) },
{ A_WRD,"prompt",	adr(prompt) },
{ A_WRD,"frdline",adr(frdline) },
{ A_WRD,"rdline",	adr(rdline) },
{ A_WRD,"word",	adr(word) },
{ A_WRD,"lookup",	adr(lookup) },
{ A_WRD,"compile",adr(compile) },
{ A_WRD,"execc",	adr(execc) },
{ A_WRD,"clearcst",adr(clearcst) },
{ A_WRD,"enter",  adr(enter) },

{ A_WRD,"...",	adr(next) },
{ A_WRD,"nop",	adr(next) },
{ A_WRD,"fconstant",adr(fconstant) },
{ A_WRD,"$constant",adr(dolar_constant) },
{ A_WRD,"((variable))",adr(__variable) },

{ A_WRD,"array", adr(array) },
{ A_WRD,"hash", adr(hash) },

{ A_WRD,"f.",	adr(fdot) },
{ A_WRD,"p.",	adr(pdot) },

{ A_WRD,".t",	adr(dott) },
{ A_WRD,"true",	adr(true) },
{ A_WRD,"false",	adr(false) },

{ A_WRD,"{until",	adr(brace_until) },
{ A_IMM,"{while",	adr(brace_while) },
{ A_WRD,"break",	adr(break) },
// { A_WRD,"succeed",	adr(succeed) },
{ A_WRD,"fail",		adr(fail) },
{ A_WRD,"{if",	adr(brace_if) },
{ A_IMM,"{else",	adr(brace_else) },
{ A_WRD,"(if)",	adr(_if) },
{ A_WRD,"(else)",adr(_else) },
{ A_IMM,"{loop",	adr(brace_loop) },
{ A_IMM,"{+loop",	adr(brace_plus_loop) },
{ A_WRD,"({do)",	adr(_brace_do) },
{ A_WRD,"({loop)",adr(_brace_loop) },
{ A_WRD,"({+loop)",adr(_brace_plus_loop) },

{ A_WRD,"[[",	adr(lleft_bracket) },
{ A_WRD,"]]",	adr(rright_bracket) },
{ A_WRD,"l]]",	adr(l_rright_bracket) },

{ A_WRD,"([)",	adr(_left_bracket) },
{ A_WRD,"(])",	adr(_right_bracket) },
{ A_IMM,"[",	adr(left_bracket) },
{ A_IMM,"]",	adr(right_bracket) },

{ A_IMM,">",	adr(right_angle) },
{ A_WRD,"<l",	adr(left_angle_l) },
{ A_WRD,"l>",	adr(l_right_angle) },
{ A_WRD,"<c",	adr(left_angle_c) },
{ A_WRD,"c>",	adr(c_right_angle) },

{ A_WRD,"i",	adr(i) },
{ A_WRD,"j",	adr(j) },
{ A_WRD,"k",	adr(k) },
{ A_WRD,"-i",	adr(minus_i) },
{ A_WRD,"-j",	adr(minus_j) },
{ A_WRD,"-k",	adr(minus_k) },

{ A_WRD,"(;)",   adr(_semicolon) },
{ A_WRD,"(:)",   adr(_colon) },

{ A_WRD,"{ifelse", adr(brace_ifelse) },
{ A_WRD,"({else)",adr(_brace_else) },
{ A_WRD,"({)",   adr(_left_brace) },
{ A_WRD,"(})",   adr(_right_brace) },
{ A_IMM,"{",	adr(left_brace) },
{ A_IMM,"}",	adr(right_brace) },
{ A_WRD,"(})",   adr(_right_brace) },
{ A_IMM,"{}>",	adr(bb_right_angle) },

{ A_WRD,"(:{)",	adr(_colon_brace) },
{ A_IMM,":{",	adr(colon_brace) },

{ A_WRD, "({:)",	adr(_brace_colon) },

{ A_IMM,"):{",	adr(paren_colon_brace) },
{ A_WRD,"():{)",	adr(_paren_colon_brace) },

{ A_IMM,"{(",	adr(brace_left_paren) },
{ A_WRD,"({()",adr(_brace_left_paren) },
{ A_WRD,"({))",adr(_brace_right_paren) },

{ 0, NULL, NULL },
};



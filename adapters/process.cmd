@echo off
setlocal EnableDelayedExpansion

set DEFS=doboz_cs_adapter.h -undef -E -Iincludes\ -CC
gcc %DEFS% > gen_doboz.c
gcc %DEFS% -DGEN_SAFE > gen_doboz_safe.c

rx -p doboz_cs_replace.xml gen_doboz.c
rx -p doboz_cs_replace_safe.xml gen_doboz_safe.c

endlocal
@echo off
setlocal EnableDelayedExpansion

set DEFS=doboz_cs_adapter.h -undef
gcc %DEFS% -E -Iincludes\ > gen_doboz.c
gcc %DEFS% -DGEN_SAFE -E -Iincludes\ > gen_doboz_safe.c

endlocal
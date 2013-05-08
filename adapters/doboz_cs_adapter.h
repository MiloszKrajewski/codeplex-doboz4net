#define DOBOZ_MK_OPT
#define DOBOZ_MK_ADAPTER

#define size_t int
#define int64_t long
#define uint64_t ulong
#define int32_t int
#define uint32_t uint
#define int16_t short
#define uint16_t ushort
#define int8_t sbyte
#define uint8_t byte
#define UCHAR_MAX byte.MaxValue
#define USHRT_MAX ushort.MaxValue
#define UINT_MAX uint.MaxValue

#define outputIterator dst_p
#define inputIterator src_p
#define outputEnd dst_end
#define inputEnd src_end
#define inputBuffer src
#define outputBuffer dst
#define literalRunLengthTable LITERAL_RUN_LENGTH_TABLE

#define assert(c) Debug.Assert(c)
#define fastRead(s,l) PeekN_##l(s)
#define PeekN_WORD_SIZE Peek4
#define PeekN_1 Peek1
#define fastWrite(d,v,l) PokeN_##l(d,v)
#define PokeN_WORD_SIZE Poke4
#define PokeN_TRAILING_DUMMY_SIZE Poke4
#define PokeN_1 Poke1
#define PokeN_size(d,v) PokeN(d, v, size)
#define PokeN(d,v,l) { switch (l) { case 4: case 3: Poke4(d, v); break; case 2: Poke2(d, v); break; default: Poke1(d, v); } }

#ifdef GEN_SAFE
	#define Peek4(s) Peek4(xxx, s)
	#define Peek1(s) xxx[s]
	#define Poke4(d,v) Poke4(xxx, (uint)(v))
	#define Poke2(d,v) Poke2(xxx, (ushort)(v))
	#define Poke1(d,v) xxx[d] = ((byte)(v))
#else
	#define Peek4(s) (*((uint*)s))
	#define Peek1(s) (*(s))
	#define Poke4(d,v) *((uint*)(d)) = ((uint)(v))
	#define Poke2(d,v) *((ushort*)(d)) = ((ushort)(v))
	#define Poke1(d,v) *(d) = ((byte)(v))
#endif

//---- Decompressor ----------------------------------------------------------

#include "..\original\Source\Doboz\Decompressor.cpp"

//---- Compressor ------------------------------------------------------------

#include "..\original\Source\Doboz\Compressor.cpp"
#include "..\original\Source\Doboz\Dictionary.cpp"

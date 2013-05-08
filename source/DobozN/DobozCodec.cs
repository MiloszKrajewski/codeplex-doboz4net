using System.Diagnostics;
using System;

namespace DobozN
{
	public partial class DobozCodec
	{
		public const int VERSION = 0; // encoding format

		private const int WORD_SIZE = 4; // uint32_t

		private const int MIN_MATCH_LENGTH = 3;
		private const int MAX_MATCH_LENGTH = 255 + MIN_MATCH_LENGTH;
		private const int MAX_MATCH_CANDIDATE_COUNT = 128;
		private const int DICTIONARY_SIZE = 1 << 21; // 2 MB, must be a power of 2!

		private const int TAIL_LENGTH = 2 * WORD_SIZE; // prevents fast write operations from writing beyond the end of the buffer during decoding
		private const int TRAILING_DUMMY_SIZE = WORD_SIZE; // safety trailing bytes which decrease the number of necessary buffer checks

		private static readonly sbyte[] LITERAL_RUN_LENGTH_TABLE 
			= new sbyte[] { 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0 };

		// Use a decoding lookup table in order to avoid expensive branches
		private struct LUTEntry
		{
			public uint mask; // the mask for the entire encoded match
			public byte offsetShift;
			public byte lengthMask;
			public byte lengthShift;
			public sbyte size; // the size of the encoded match in bytes
		}

		private static readonly LUTEntry[] LUT =
		{
			new LUTEntry { mask = 0xff, offsetShift = 2, lengthMask = 0, lengthShift = 0, size = 1 }, // (0)00
			new LUTEntry { mask = 0xffff, offsetShift = 2, lengthMask = 0, lengthShift = 0, size = 2 }, // (0)01
			new LUTEntry { mask = 0xffff, offsetShift = 6, lengthMask = 15, lengthShift = 2, size = 2 }, // (0)10
			new LUTEntry { mask = 0xffffff, offsetShift = 8, lengthMask = 31, lengthShift = 3, size = 3 }, // (0)11
			new LUTEntry { mask = 0xff, offsetShift = 2, lengthMask = 0, lengthShift = 0, size = 1 }, // (1)00 = (0)00
			new LUTEntry { mask = 0xffff, offsetShift = 2, lengthMask = 0, lengthShift = 0, size = 2 }, // (1)01 = (0)01
			new LUTEntry { mask = 0xffff, offsetShift = 6, lengthMask = 15, lengthShift = 2, size = 2 }, // (1)10 = (0)10
			new LUTEntry { mask = 0xffffffff, offsetShift = 11, lengthMask = 255, lengthShift = 3, size = 4 }, // 111
		};

		private enum Result
		{
			RESULT_OK,
			RESULT_ERROR_BUFFER_TOO_SMALL,
			RESULT_ERROR_CORRUPTED_DATA,
			RESULT_ERROR_UNSUPPORTED_VERSION,
		};

		private struct Match
		{
			public int length;
			public int offset;
		};

		private struct Header
		{
			public ulong uncompressedSize;
			public ulong compressedSize;
			public int version;
			public bool isStored;
		};

		public struct CompressionInfo
		{
			public ulong uncompressedSize;
			public ulong compressedSize;
			public int version;
		};

		private static int GetSizeCodedSize(int size)
		{
			if (size <= byte.MaxValue) return 1;
			if (size <= ushort.MaxValue) return 2;
			return 4;
		}

		private static int GetHeaderSize(int size)
		{
			return 1 + 2 * GetSizeCodedSize(size);
		}

		public static int MaximumOutputLength(int size)
		{
			// The header + the original uncompressed data
			return GetHeaderSize(size) + size;
		}
	}
}

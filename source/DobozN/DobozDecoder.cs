﻿// ReSharper disable InconsistentNaming

using System;

namespace DobozN
{
	public partial class DobozDecoder
	{
		public const int VERSION = 0; // encoding format

		protected const int WORD_SIZE = 4; // uint32_t
		protected internal const int MIN_MATCH_LENGTH = 3;
		protected internal const int TAIL_LENGTH = 2 * WORD_SIZE; // prevents fast write operations from writing beyond the end of the buffer during decoding

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

		private static readonly LUTEntry[] LUT = {
			new LUTEntry {mask = 0xff, offsetShift = 2, lengthMask = 0, lengthShift = 0, size = 1}, // (0)00
			new LUTEntry {mask = 0xffff, offsetShift = 2, lengthMask = 0, lengthShift = 0, size = 2}, // (0)01
			new LUTEntry {mask = 0xffff, offsetShift = 6, lengthMask = 15, lengthShift = 2, size = 2}, // (0)10
			new LUTEntry {mask = 0xffffff, offsetShift = 8, lengthMask = 31, lengthShift = 3, size = 3}, // (0)11
			new LUTEntry {mask = 0xff, offsetShift = 2, lengthMask = 0, lengthShift = 0, size = 1}, // (1)00 = (0)00
			new LUTEntry {mask = 0xffff, offsetShift = 2, lengthMask = 0, lengthShift = 0, size = 2}, // (1)01 = (0)01
			new LUTEntry {mask = 0xffff, offsetShift = 6, lengthMask = 15, lengthShift = 2, size = 2}, // (1)10 = (0)10
			new LUTEntry {mask = 0xffffffff, offsetShift = 11, lengthMask = 255, lengthShift = 3, size = 4} // 111
		};

		protected internal enum Result
		{
			RESULT_OK,
			RESULT_ERROR_BUFFER_TOO_SMALL,
			RESULT_ERROR_CORRUPTED_DATA,
			RESULT_ERROR_UNSUPPORTED_VERSION,
		};

		protected internal struct Match
		{
			public int length;
			public int offset;
		};

		internal struct Header
		{
			public int uncompressedSize;
			public int compressedSize;
			public int version;
			public bool isStored;
		};

		public struct CompressionInfo
		{
			public int uncompressedSize;
			public int compressedSize;
			public int version;
		};

		internal static int GetSizeCodedSize(int size)
		{
			return
				size <= byte.MaxValue ? sizeof(byte) :
					size <= ushort.MaxValue ? sizeof(ushort) :
						sizeof(uint);
		}

		protected internal static int GetHeaderSize(int size)
		{
			return 1 + 2 * GetSizeCodedSize(size);
		}

		public static int MaximumOutputLength(int size)
		{
			// The header + the original uncompressed data
			return GetHeaderSize(size) + size;
		}

		protected static void CheckArguments(
			byte[] input, int inputOffset, ref int inputLength)
		{
			if (input == null) 
				throw new ArgumentNullException("input");
			if (inputLength < 0) 
				inputLength = input.Length - inputOffset;
			if (inputOffset < 0 || inputOffset + inputLength > input.Length)
				throw new ArgumentException("inputOffset and inputLength are invalid for given input");
		}

		protected static void CheckArguments(
			byte[] input, int inputOffset, ref int inputLength,
			byte[] output, int outputOffset, ref int outputLength)
		{
			if (input == null) 
				throw new ArgumentNullException("input");
			if (output == null)
				throw new ArgumentNullException("output");

			if (inputLength < 0) 
				inputLength = input.Length - inputOffset;
			if (inputOffset < 0 || inputOffset + inputLength > input.Length)
				throw new ArgumentException("inputOffset and inputLength are invalid for given input");

			if (outputLength < 0) 
				outputLength = output.Length - outputOffset;
			if (outputOffset < 0 || outputOffset + outputLength > output.Length)
				throw new ArgumentException("outputOffset and outputLength are invalid for given output");
		}

	}
}

// ReSharper restore InconsistentNaming
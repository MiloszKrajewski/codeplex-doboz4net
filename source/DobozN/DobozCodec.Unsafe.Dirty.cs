using System.Diagnostics;

namespace DobozN
{
	public partial class DobozCodec
	{
		/// <summary>Copies block of memory.</summary>
		/// <param name="src">The source.</param>
		/// <param name="dst">The destination.</param>
		/// <param name="len">The length (in bytes).</param>
		private unsafe static void BlockCopy(byte* src, byte* dst, int len)
		{
			while (len >= 8) { *(ulong*)dst = *(ulong*)src; dst += 8; src += 8; len -= 8; }
			if (len >= 4) { *(uint*)dst = *(uint*)src; dst += 4; src += 4; len -= 4; }
			if (len >= 2) { *(ushort*)dst = *(ushort*)src; dst += 2; src += 2; len -= 2; }
			if (len >= 1) { *dst = *src; /* d++; s++; l--; */ }
		}

		private unsafe static Result GetCompressionInfo(byte* source, int sourceSize, ref CompressionInfo compressionInfo)
		{
			Debug.Assert(source != null);

			// Decode the header
			Header header = new Header();
			int headerSize = 0;
			Result decodeHeaderResult = decodeHeader(ref header, source, sourceSize, ref headerSize);

			if (decodeHeaderResult != Result.RESULT_OK)
			{
				return decodeHeaderResult;
			}

			// Return the requested info
			compressionInfo.uncompressedSize = header.uncompressedSize;
			compressionInfo.compressedSize = header.compressedSize;
			compressionInfo.version = header.version;

			return Result.RESULT_OK;
		}

		static unsafe Result decompress(byte* source, int sourceSize, byte* destination, int destinationSize)
		{
			fixed (LUTEntry* lut = &LUT[0])
			{
				Debug.Assert(source != null);
				Debug.Assert(destination != null);

				byte* src = (byte*)(source);
				byte* src_p = src;

				byte* dst = (byte*)(destination);
				byte* dst_p = dst;

				Debug.Assert(
					(src + sourceSize <= dst || src >= dst + destinationSize),
					"The source and destination buffers must not overlap.");

				// Decode the header
				Header header = new Header();
				int headerSize = 0;
				Result decodeHeaderResult = decodeHeader(ref header, source, sourceSize, ref headerSize);

				if (decodeHeaderResult != Result.RESULT_OK)
				{
					return decodeHeaderResult;
				}

				src_p += headerSize;

				if (header.version != VERSION)
				{
					return Result.RESULT_ERROR_UNSUPPORTED_VERSION;
				}

				// Check whether the supplied buffers are large enough
				if (sourceSize < (int)header.compressedSize || destinationSize < (int)header.uncompressedSize)
				{
					return Result.RESULT_ERROR_BUFFER_TOO_SMALL;
				}

				int uncompressedSize = (int)(header.uncompressedSize);

				// If the data is simply stored, copy it to the destination buffer and we're done
				if (header.isStored)
				{
					BlockCopy(src_p, dst, uncompressedSize);
					return Result.RESULT_OK;
				}

				byte* src_end = src + ((int)(header.compressedSize));
				byte* dst_end = dst + uncompressedSize;

				// Compute pointer to the first byte of the output 'tail'
				// Fast write operations can be used only before the tail, because those may write beyond the end of the output buffer
				byte* outputTail = (uncompressedSize > TAIL_LENGTH) ? (dst_end - TAIL_LENGTH) : dst;

				// Initialize the control word to 'empty'
				uint controlWord = 1;

				// Decoding loop
				while (true)
				{
					// Check whether there is enough data left in the input buffer
					// In order to decode the next literal/match, we have to read up to 8 bytes (2 words)
					// Thanks to the trailing dummy, there must be at least 8 remaining input bytes
					if (src_p + 2 * WORD_SIZE > src_end)
					{
						return Result.RESULT_ERROR_CORRUPTED_DATA;
					}

					// Check whether we must read a control word
					if (controlWord == 1)
					{
						Debug.Assert(src_p + WORD_SIZE <= src_end);
						controlWord = *((uint*)src_p);
						src_p += WORD_SIZE;
					}

					// Detect whether it's a literal or a match
					if ((controlWord & 1) == 0)
					{
						// It's a literal

						// If we are before the tail, we can safely use fast writing operations
						if (dst_p < outputTail)
						{
							// We copy literals in runs of up to 4 because it's faster than copying one by one

							// Copy implicitly 4 literals regardless of the run length
							Debug.Assert(src_p + WORD_SIZE <= src_end);
							Debug.Assert(dst_p + WORD_SIZE <= dst_end);
							*((uint*)(dst_p)) = *((uint*)src_p);

							// Get the run length using a lookup table
							int runLength = LITERAL_RUN_LENGTH_TABLE[controlWord & 0xf];

							// Advance the src and dst pointers with the run length
							src_p += runLength;
							dst_p += runLength;

							// Consume as much control word bits as the run length
							controlWord >>= runLength;
						}
						else
						{
							// We have reached the tail, we cannot output literals in runs anymore
							// Output all remaining literals
							while (dst_p < dst_end)
							{
								// Check whether there is enough data left in the input buffer
								// In order to decode the next literal, we have to read up to 5 bytes
								if (src_p + WORD_SIZE + 1 > src_end)
								{
									return Result.RESULT_ERROR_CORRUPTED_DATA;
								}

								// Check whether we must read a control word
								if (controlWord == 1)
								{
									Debug.Assert(src_p + WORD_SIZE <= src_end);
									controlWord = *((uint*)src_p);
									src_p += WORD_SIZE;
								}

								// Output one literal
								// We cannot use fast read/write functions
								Debug.Assert(src_p + 1 <= src_end);
								Debug.Assert(dst_p + 1 <= dst_end);
								*dst_p++ = *src_p++;

								// Next control word bit
								controlWord >>= 1;
							}

							// Done
							return Result.RESULT_OK;
						}
					}
					else
					{
						// It's a match

						// Decode the match
						Debug.Assert(src_p + WORD_SIZE <= src_end);
						Match match;

						// src_p += decodeMatch(ref match, src_p);
						{
							// Read the maximum number of bytes a match is coded in (4)
							uint w = *((uint*)src_p);

							// Compute the decoding lookup table entry index: the lowest 3 bits of the encoded match
							uint u = w & 7;

							// Compute the match offset and length using the lookup table entry
							match.offset = (int)((w & lut[u].mask) >> lut[u].offsetShift);
							match.length = (int)(((w >> lut[u].lengthShift) & lut[u].lengthMask) + MIN_MATCH_LENGTH);

							src_p += lut[u].size;
						}

						// Copy the matched string
						// In order to achieve high performance, we copy characters in groups of machine words
						// Overlapping matches require special care
						byte* matchString = dst_p - match.offset;

						// Check whether the match is out of range
						if (matchString < dst || dst_p + match.length > outputTail)
						{
							return Result.RESULT_ERROR_CORRUPTED_DATA;
						}

						int i = 0;

						if (match.offset < WORD_SIZE)
						{
							// The match offset is less than the word size
							// In order to correctly handle the overlap, we have to copy the first three bytes one by one
							do
							{
								Debug.Assert(matchString + i >= dst);
								Debug.Assert(matchString + i + WORD_SIZE <= dst_end);
								Debug.Assert(dst_p + i + WORD_SIZE <= dst_end);
								*(dst_p + i) = *(matchString + i);
								++i;
							}
							while (i < 3);

							// With this trick, we increase the distance between the source and destination pointers
							// This enables us to use fast copying for the rest of the match
							matchString -= 2 + (match.offset & 1);
						}

						// Fast copying
						// There must be no overlap between the source and destination words
						do
						{
							Debug.Assert(matchString + i >= dst);
							Debug.Assert(matchString + i + WORD_SIZE <= dst_end);
							Debug.Assert(dst_p + i + WORD_SIZE <= dst_end);
							*((uint*)(dst_p + i)) = *((uint*)(matchString + i));
							i += WORD_SIZE;
						}
						while (i < match.length);

						dst_p += match.length;

						// Next control word bit
						controlWord >>= 1;
					}
				}
			}
		}

		// Decodes a header and returns its size in bytes
		// If the header is not valid, the function returns 0
		static unsafe Result decodeHeader(ref Header header, byte* source, int sourceSize, ref int headerSize)
		{
			byte* src_p = (byte*)(source);

			// Decode the attribute bytes
			if (sourceSize < 1)
			{
				return Result.RESULT_ERROR_BUFFER_TOO_SMALL;
			}

			uint attributes = *src_p++;

			header.version = (int)(attributes & 7);
			int sizeCodedSize = (int)(((attributes >> 3) & 7) + 1);

			// Compute the size of the header
			headerSize = 1 + 2 * sizeCodedSize;

			if (sourceSize < ((int)(headerSize)))
			{
				return Result.RESULT_ERROR_BUFFER_TOO_SMALL;
			}

			header.isStored = (attributes & 128) != 0;

			// Decode the uncompressed and compressed sizes
			switch (sizeCodedSize)
			{
				case 1:
					header.uncompressedSize = *((byte*)(src_p));
					header.compressedSize = *((byte*)(src_p + sizeCodedSize));
					break;

				case 2:
					header.uncompressedSize = *((ushort*)(src_p));
					header.compressedSize = *((ushort*)(src_p + sizeCodedSize));
					break;

				case 4:
					header.uncompressedSize = *((uint*)(src_p));
					header.compressedSize = *((uint*)(src_p + sizeCodedSize));
					break;

				default:
					return Result.RESULT_ERROR_CORRUPTED_DATA;
			}

			return Result.RESULT_OK;
		}

	}
}

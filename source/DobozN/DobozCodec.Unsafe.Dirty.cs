using System.Diagnostics;

namespace DobozN
{
	public partial class DobozCodec
	{
		// Store the source
		private static unsafe Result Store(byte* source, int sourceSize, byte* destination, out int compressedSize)
		{
			var dst = destination;
			var dst_p = dst;

			// Encode the header
			var maxCompressedSize = MaximumOutputLength(sourceSize);
			var headerSize = GetHeaderSize(maxCompressedSize);

			compressedSize = headerSize + sourceSize;

			var header = new Header {
				version = VERSION,
				isStored = true,
				uncompressedSize = sourceSize,
				compressedSize = compressedSize
			};

			EncodeHeader(ref header, maxCompressedSize, destination);
			dst_p += headerSize;

			// Store the data
			BlockCopy(source, dst_p, sourceSize);

			return Result.RESULT_OK;
		}

		private static unsafe void EncodeHeader(ref Header header, int maxCompressedSize, byte* destination)
		{
			Debug.Assert(header.version < 8);

			var dst_p = destination;

			// Encode the attribute byte
			var attributes = header.version;

			var sizeCodedSize = GetSizeCodedSize(maxCompressedSize);
			attributes |= (sizeCodedSize - 1) << 3;

			if (header.isStored)
			{
				attributes |= 128;
			}

			*dst_p++ = (byte)(attributes);

			// Encode the uncompressed and compressed sizes
			switch (sizeCodedSize)
			{
				case 1:
					*dst_p = (byte)(header.uncompressedSize);
					*(dst_p + sizeCodedSize) = (byte)(header.compressedSize);
					break;

				case 2:
					*((ushort*)(dst_p)) = (ushort)(header.uncompressedSize);
					*((ushort*)(dst_p + sizeCodedSize)) = (ushort)(header.compressedSize);
					break;

				case 4:
					*((uint*)(dst_p)) = (uint)(header.uncompressedSize);
					*((uint*)(dst_p + sizeCodedSize)) = (uint)(header.compressedSize);
					break;
			}
		}

		private static unsafe Result Compress(
			byte* source, int sourceSize, byte* destination, int destinationSize, out int compressedSize)
		{
			Debug.Assert(source != null);
			Debug.Assert(destination != null);

			if (sourceSize == 0)
			{
				compressedSize = 0;
				return Result.RESULT_ERROR_BUFFER_TOO_SMALL;
			}

			var storedSize = MaximumOutputLength(sourceSize);
			var maxCompressedSize = destinationSize;

			var src = (source);
			var dst = (destination);
			var dst_end = dst + destinationSize;
			Debug.Assert(src + sourceSize <= dst || src >= dst_end, "The source and destination buffers must not overlap.");

			// Compute the maximum output end pointer
			// We use this to determine whether we should store the data instead of compressing it
			var maxOutputEnd = dst + ((maxCompressedSize));

			// Allocate the header
			var dst_p = dst;
			dst_p += GetHeaderSize(maxCompressedSize);

			// Initialize the dictionary
			var dictionary = new Dictionary(src, sourceSize);

			// Initialize the control word which contains the literal/match bits
			// The highest bit of a control word is a guard bit, which marks the end of the bit list
			// The guard bit simplifies and speeds up the decoding process, and it
			const int controlWordBitCount = WORD_SIZE * 8 - 1;
			const uint controlWordGuardBit = 1u << controlWordBitCount;
			var controlWord = controlWordGuardBit;
			var controlWordBit = 0;

			// Since we do not know the contents of the control words in advance, we allocate space for them and subsequently fill them with data as soon as we can
			// This is necessary because the decoder must encounter a control word *before* the literals and matches it refers to
			// We begin the compressed data with a control word
			var controlWordPointer = dst_p;
			dst_p += WORD_SIZE;

			// The match located at the current inputIterator position

			// The match located at the next inputIterator position
			// Initialize it to 'no match', because we are at the beginning of the inputIterator buffer
			// A match with a length of 0 means that there is no match
			var nextMatch = new Match { length = 0 };

			// The dictionary matching look-ahead is 1 character, so set the dictionary position to 1
			// We don't have to worry about getting matches beyond the inputIterator, because the dictionary ignores such requests
			dictionary.Skip();

			// At each position, we select the best match to encode from a list of match candidates provided by the match finder
			// var matchCandidates = new Match[MAX_MATCH_CANDIDATE_COUNT];

			fixed (Match* mc = new Match[MAX_MATCH_CANDIDATE_COUNT])
			{
				// Iterate while there is still data left
				while (dictionary.Position - 1 < sourceSize)
				{
					// Check whether the output is too large
					// During each iteration, we may output up to 8 bytes (2 words), and the compressed stream ends with 4 dummy bytes
					if (dst_p + 2 * WORD_SIZE + TRAILING_DUMMY_SIZE > maxOutputEnd)
					{
						compressedSize = 0;
						return
							storedSize <= destinationSize
								? Store(source, sourceSize, destination, out compressedSize)
								: Result.RESULT_ERROR_BUFFER_TOO_SMALL;
					}

					// Check whether the control word must be flushed
					if (controlWordBit == controlWordBitCount)
					{
						// Flush current control word
						*((uint*)(controlWordPointer)) = (controlWord);

						// New control word
						controlWord = controlWordGuardBit;
						controlWordBit = 0;

						controlWordPointer = dst_p;
						dst_p += WORD_SIZE;
					}

					// The current match is the previous 'next' match
					var match = nextMatch;

					// Find the best match at the next position
					// The dictionary position is automatically incremented
					var matchCandidateCount = dictionary.FindMatches(mc);
					nextMatch = GetBestMatch(mc, matchCandidateCount);

					// If we have a match, do not immediately use it, because we may miss an even better match (lazy evaluation)
					// If encoding a literal and the next match has a higher compression ratio than encoding the current match, discard the current match
					if (match.length > 0 && (1 + nextMatch.length) * GetMatchCodedSize(ref match) > match.length * (1 + GetMatchCodedSize(ref nextMatch)))
					{
						match.length = 0;
					}

					// Check whether we must encode a literal or a match
					if (match.length == 0)
					{
						// Encode a literal (0 control word flag)
						// In order to efficiently decode literals in runs, the literal bit (0) must differ from the guard bit (1)

						// The current dictionary position is now two characters ahead of the literal to encode
						Debug.Assert(dst_p + 1 <= dst_end);
						*(dst_p) = (src[dictionary.Position - 2]);
						++dst_p;
					}
					else
					{
						// Encode a match (1 control word flag)
						controlWord |= 1u << controlWordBit;

						Debug.Assert(dst_p + WORD_SIZE <= dst_end);
						dst_p += EncodeMatch(ref match, dst_p);

						// Skip the matched characters
						for (var i = 0; i < match.length - 2; ++i)
						{
							dictionary.Skip();
						}

						matchCandidateCount = dictionary.FindMatches(mc);
						nextMatch = GetBestMatch(mc, matchCandidateCount);
					}

					// Next control word bit
					++controlWordBit;
				}

				// Flush the control word
				*((uint*)(controlWordPointer)) = controlWord;

				// Output trailing safety dummy bytes
				// This reduces the number of necessary buffer checks during decoding
				Debug.Assert(dst_p + TRAILING_DUMMY_SIZE <= dst_end);
				*(uint*)(dst_p) = 0;
				dst_p += TRAILING_DUMMY_SIZE;

				// Done, compute the compressed size
				compressedSize = (int)(dst_p - dst);

				// Encode the header
				var header = new Header {
					version = VERSION,
					isStored = false,
					uncompressedSize = sourceSize,
					compressedSize = compressedSize
				};

				EncodeHeader(ref header, maxCompressedSize, dst);

				// Return the compressed size
				return Result.RESULT_OK;
			}
		}

		// Selects the best match from a list of match candidates provided by the match finder
		private static unsafe Match GetBestMatch(Match* matchCandidates, int matchCandidateCount)
		{
			var bestMatch = new Match { length = 0 };

			// Select the longest match which can be coded efficiently (coded size is less than the length)
			for (var i = matchCandidateCount - 1; i >= 0; --i)
			{
				if (matchCandidates[i].length > GetMatchCodedSize(ref matchCandidates[i]))
				{
					bestMatch = matchCandidates[i];
					break;
				}
			}

			return bestMatch;
		}

		private static unsafe int GetMatchCodedSize(ref Match match)
		{
			return EncodeMatch(ref match, null);
		}

		private static unsafe int EncodeMatch(ref Match match, byte* destination)
		{
			Debug.Assert(match.length <= MAX_MATCH_LENGTH);
			Debug.Assert(match.length == 0 || match.offset < DICTIONARY_SIZE);

			uint word;
			int size;

			var lengthCode = (uint)(match.length - MIN_MATCH_LENGTH);
			var offsetCode = (uint)(match.offset);

			if (lengthCode == 0 && offsetCode < 64)
			{
				word = offsetCode << 2; // 00
				size = 1;
			}
			else if (lengthCode == 0 && offsetCode < 16384)
			{
				word = (offsetCode << 2) | 1; // 01
				size = 2;
			}
			else if (lengthCode < 16 && offsetCode < 1024)
			{
				word = (offsetCode << 6) | (lengthCode << 2) | 2; // 10
				size = 2;
			}
			else if (lengthCode < 32 && offsetCode < 65536)
			{
				word = (offsetCode << 8) | (lengthCode << 3) | 3; // 11
				size = 3;
			}
			else
			{
				word = (offsetCode << 11) | (lengthCode << 3) | 7; // 111
				size = 4;
			}

			if (destination != null)
			{
				{
					switch (size)
					{
						case 4:
						case 3:
							*(uint*)(destination) = word;
							break;
						case 2:
							*(ushort*)(destination) = (ushort)word;
							break;
						default:
							*destination = (byte)word;
							break;
					}
				}
			}

			return size;
		}
	}
}

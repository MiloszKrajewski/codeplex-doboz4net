# 1 "doboz_cs_adapter.h"
# 1 "<command-line>"
# 1 "doboz_cs_adapter.h"
# 50 "doboz_cs_adapter.h"
//---- Decompressor ----------------------------------------------------------

# 1 "..\\original\\Source\\Doboz\\Decompressor.cpp" 1
/*

 * Doboz Data Compression Library

 * Copyright (C) 2010-2011 Attila T. Afra <attila.afra@gmail.com>

 *

 * This software is provided 'as-is', without any express or implied warranty. In no event will

 * the authors be held liable for any damages arising from the use of this software.

 *

 * Permission is granted to anyone to use this software for any purpose, including commercial

 * applications, and to alter it and redistribute it freely, subject to the following restrictions:

 *

 * 1. The origin of this software must not be misrepresented; you must not claim that you wrote the

 *    original software. If you use this software in a product, an acknowledgment in the product

 *    documentation would be appreciated but is not required.

 * 2. Altered source versions must be plainly marked as such, and must not be misrepresented as

 *    being the original software.

 * 3. This notice may not be removed or altered from any source distribution.

 */
# 19 "..\\original\\Source\\Doboz\\Decompressor.cpp"
# 1 "includes/cstring" 1
# 1 "includes/empty.h" 1
// this is fake header
// it is used for mocking headers which I don't need to be included
# 1 "includes/cstring" 2
# 20 "..\\original\\Source\\Doboz\\Decompressor.cpp" 2
# 1 "..\\original\\Source\\Doboz\\Decompressor.h" 1
/*

 * Doboz Data Compression Library

 * Copyright (C) 2010-2011 Attila T. Afra <attila.afra@gmail.com>

 *

 * This software is provided 'as-is', without any express or implied warranty. In no event will

 * the authors be held liable for any damages arising from the use of this software.

 *

 * Permission is granted to anyone to use this software for any purpose, including commercial

 * applications, and to alter it and redistribute it freely, subject to the following restrictions:

 *

 * 1. The origin of this software must not be misrepresented; you must not claim that you wrote the

 *    original software. If you use this software in a product, an acknowledgment in the product

 *    documentation would be appreciated but is not required.

 * 2. Altered source versions must be plainly marked as such, and must not be misrepresented as

 *    being the original software.

 * 3. This notice may not be removed or altered from any source distribution.

 */
# 19 "..\\original\\Source\\Doboz\\Decompressor.h"

# 1 "..\\original\\Source\\Doboz\\Common.h" 1
/*

 * Doboz Data Compression Library

 * Copyright (C) 2010-2011 Attila T. Afra <attila.afra@gmail.com>

 *

 * This software is provided 'as-is', without any express or implied warranty. In no event will

 * the authors be held liable for any damages arising from the use of this software.

 *

 * Permission is granted to anyone to use this software for any purpose, including commercial

 * applications, and to alter it and redistribute it freely, subject to the following restrictions:

 *

 * 1. The origin of this software must not be misrepresented; you must not claim that you wrote the

 *    original software. If you use this software in a product, an acknowledgment in the product

 *    documentation would be appreciated but is not required.

 * 2. Altered source versions must be plainly marked as such, and must not be misrepresented as

 *    being the original software.

 * 3. This notice may not be removed or altered from any source distribution.

 */
# 19 "..\\original\\Source\\Doboz\\Common.h"

# 1 "includes/stdint.h" 1
# 1 "includes/empty.h" 1
// this is fake header
// it is used for mocking headers which I don't need to be included
# 1 "includes/stdint.h" 2
# 22 "..\\original\\Source\\Doboz\\Common.h" 2
# 1 "includes/climits" 1
# 1 "includes/empty.h" 1
// this is fake header
// it is used for mocking headers which I don't need to be included
# 1 "includes/climits" 2
# 23 "..\\original\\Source\\Doboz\\Common.h" 2
# 1 "includes/cassert" 1
# 1 "includes/empty.h" 1
// this is fake header
// it is used for mocking headers which I don't need to be included
# 1 "includes/cassert" 2
# 24 "..\\original\\Source\\Doboz\\Common.h" 2
# 33 "..\\original\\Source\\Doboz\\Common.h"
namespace doboz {

const int VERSION = 0; // encoding format

enum Result
{
 RESULT_OK,
 RESULT_ERROR_BUFFER_TOO_SMALL,
 RESULT_ERROR_CORRUPTED_DATA,
 RESULT_ERROR_UNSUPPORTED_VERSION,
};

namespace detail {

struct Match
{
 int length;
 int offset;
};

struct Header
{
 ulong uncompressedSize;
 ulong compressedSize;
 int version;
 bool isStored;
};

const int WORD_SIZE = 4; // uint32_t

const int MIN_MATCH_LENGTH = 3;
const int MAX_MATCH_LENGTH = 255 + MIN_MATCH_LENGTH;
const int MAX_MATCH_CANDIDATE_COUNT = 128;
const int DICTIONARY_SIZE = 1 << 21; // 2 MB, must be a power of 2!

const int TAIL_LENGTH = 2 * WORD_SIZE; // prevents fast write operations from writing beyond the end of the buffer during decoding
const int TRAILING_DUMMY_SIZE = WORD_SIZE; // safety trailing bytes which decrease the number of necessary buffer checks

// Reads up to 4 bytes and returns them in a word
// WARNING: May read more bytes than requested!
inline uint PeekN_size_t size(byte* source)
{
 Debug.Assert(size <= WORD_SIZE);

 switch (size)
 {
 case 4:
  return *((uint*)(source));

 case 3:
  return *((uint*)(source));

 case 2:
  return *((ushort*)(source));

 case 1:
  return *((byte*)(source));

 default:
  return 0; // dummy
 }
}

// Writes up to 4 bytes specified in a word
// WARNING: May write more bytes than requested!
inline void PokeN_size_t size(byte* destination,uint word)
{
 Debug.Assert(size <= WORD_SIZE);

 switch (size)
 {
 case 4:
  *((uint*)(destination)) = word;
  break;

 case 3:
  *((uint*)(destination)) = word;
  break;

 case 2:
  *((ushort*)(destination)) = (ushort)(word);
  break;

 case 1:
  *((byte*)(destination)) = (byte)(word);
  break;
 }
}

} // namespace detail

} // namespace doboz
# 22 "..\\original\\Source\\Doboz\\Decompressor.h" 2

namespace doboz {

struct CompressionInfo
{
 ulong uncompressedSize;
 ulong compressedSize;
 int version;
};

class Decompressor
{
public:
 // Decompresses a block of data
 // The source and destination buffers must not overlap
 // This operation is memory safe
 // On success, returns RESULT_OK
 Result decompress(byte* source, int sourceSize, byte* destination, int destinationSize);

 // Retrieves information about a compressed block of data
 // This operation is memory safe
 // On success, returns RESULT_OK and outputs the compression information
 Result getCompressionInfo(byte* source, int sourceSize, CompressionInfo& compressionInfo);

private:
 int decodeMatch(detail::Match& match, byte* source);
 Result decodeHeader(detail::Header& header, byte* source, int sourceSize, int& headerSize);
};

} // namespace doboz
# 21 "..\\original\\Source\\Doboz\\Decompressor.cpp" 2

namespace doboz {

using namespace detail;

Result Decompressor::decompress(byte* source, int sourceSize, byte* destination, int destinationSize)
{
 Debug.Assert(source != 0);
 Debug.Assert(destination != 0);

 byte* src = (byte*)(source);
 byte* src_p = src;

 byte* dst = (byte*)(destination);
 byte* dst_p = dst;

 Debug.Assert((src + sourceSize <= dst || src >= dst + destinationSize) && "The source and destination buffers must not overlap.")
                                                         ;

 // Decode the header
 Header header;
 int headerSize;
 Result decodeHeaderResult = decodeHeader(header, source, sourceSize, headerSize);

 if (decodeHeaderResult != RESULT_OK)
 {
  return decodeHeaderResult;
 }

 src_p += headerSize;

 if (header.version != VERSION)
 {
  return RESULT_ERROR_UNSUPPORTED_VERSION;
 }

 // Check whether the supplied buffers are large enough
 if (sourceSize < header.compressedSize || destinationSize < header.uncompressedSize)
 {
  return RESULT_ERROR_BUFFER_TOO_SMALL;
 }

 int uncompressedSize = (int)(header.uncompressedSize);

 // If the data is simply stored, copy it to the destination buffer and we're done
 if (header.isStored)
 {
  memcpy(dst, src_p, uncompressedSize);
  return RESULT_OK;
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
   return RESULT_ERROR_CORRUPTED_DATA;
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
    *((uint*)(dst_p)) = (uint)(*((uint*)src_p));

    // Get the run length using a lookup table
    static const sbyte LITERAL_RUN_LENGTH_TABLE[16] = {4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0};
    int runLength = LITERAL_RUN_LENGTH_TABLE[controlWord & 0xf];

    // Advance the inputBuffer and outputBuffer pointers with the run length
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
      return RESULT_ERROR_CORRUPTED_DATA;
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
    return RESULT_OK;
   }
  }
  else
  {
   // It's a match

   // Decode the match
   Debug.Assert(src_p + WORD_SIZE <= src_end);
   Match match;
   src_p += decodeMatch(match, src_p);

   // Copy the matched string
   // In order to achieve high performance, we copy characters in groups of machine words
   // Overlapping matches require special care
   byte* matchString = dst_p - match.offset;

   // Check whether the match is out of range
   if (matchString < dst || dst_p + match.length > outputTail)
   {
    return RESULT_ERROR_CORRUPTED_DATA;
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
     *(dst_p + i) = (byte)(*(matchString + i));
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
    *((uint*)(dst_p + i)) = (uint)(*((uint*)matchString + i));
    i += WORD_SIZE;
   }
   while (i < match.length);

   dst_p += match.length;

   // Next control word bit
   controlWord >>= 1;
  }
 }
}

// Decodes a match and returns its size in bytes
inline int Decompressor::decodeMatch(Match& match, byte* source)
{
 // Use a decoding lookup table in order to avoid expensive branches
 static const struct
 {
  uint mask; // the mask for the entire encoded match
  byte offsetShift;
  byte lengthMask;
  byte lengthShift;
  sbyte size; // the size of the encoded match in bytes
 }
 lut[] =
 {
  {0xff, 2, 0, 0, 1}, // (0)00
  {0xffff, 2, 0, 0, 2}, // (0)01
  {0xffff, 6, 15, 2, 2}, // (0)10
  {0xffffff, 8, 31, 3, 3}, // (0)11
  {0xff, 2, 0, 0, 1}, // (1)00 = (0)00
  {0xffff, 2, 0, 0, 2}, // (1)01 = (0)01
  {0xffff, 6, 15, 2, 2}, // (1)10 = (0)10
  {0xffffffff, 11, 255, 3, 4}, // 111
 };

 // Read the maximum number of bytes a match is coded in (4)
 uint word = *((uint*)source);

 // Compute the decoding lookup table entry index: the lowest 3 bits of the encoded match
 uint i = word & 7;

 // Compute the match offset and length using the lookup table entry
 match.offset = (int)((word & lut[i].mask) >> lut[i].offsetShift);
 match.length = (int)(((word >> lut[i].lengthShift) & lut[i].lengthMask) + MIN_MATCH_LENGTH);

 return lut[i].size;
}

// Decodes a header and returns its size in bytes
// If the header is not valid, the function returns 0
Result Decompressor::decodeHeader(Header& header, byte* source, int sourceSize, int& headerSize)
{
 byte* src_p = (byte*)(source);

 // Decode the attribute bytes
 if (sourceSize < 1)
 {
  return RESULT_ERROR_BUFFER_TOO_SMALL;
 }

 uint attributes = *src_p++;

 header.version = attributes & 7;
 int sizeCodedSize = ((attributes >> 3) & 7) + 1;

 // Compute the size of the header
 headerSize = 1 + 2 * sizeCodedSize;

 if (sourceSize < ((int)(headerSize)))
 {
  return RESULT_ERROR_BUFFER_TOO_SMALL;
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

 case 8:
  header.uncompressedSize = *((ulong*)(src_p));
  header.compressedSize = *((ulong*)(src_p + sizeCodedSize));
  break;

 default:
  return RESULT_ERROR_CORRUPTED_DATA;
 }

 return RESULT_OK;
}

Result Decompressor::getCompressionInfo(byte* source, int sourceSize, CompressionInfo& compressionInfo)
{
 Debug.Assert(source != 0);

 // Decode the header
 Header header;
 int headerSize;
 Result decodeHeaderResult = decodeHeader(header, source, sourceSize, headerSize);

 if (decodeHeaderResult != RESULT_OK)
 {
  return decodeHeaderResult;
 }

 // Return the requested info
 compressionInfo.uncompressedSize = header.uncompressedSize;
 compressionInfo.compressedSize = header.compressedSize;
 compressionInfo.version = header.version;

 return RESULT_OK;
}

}
# 53 "doboz_cs_adapter.h" 2

//---- Compressor ------------------------------------------------------------

# 1 "..\\original\\Source\\Doboz\\Compressor.cpp" 1
/*

 * Doboz Data Compression Library

 * Copyright (C) 2010-2011 Attila T. Afra <attila.afra@gmail.com>

 *

 * This software is provided 'as-is', without any express or implied warranty. In no event will

 * the authors be held liable for any damages arising from the use of this software.

 *

 * Permission is granted to anyone to use this software for any purpose, including commercial

 * applications, and to alter it and redistribute it freely, subject to the following restrictions:

 *

 * 1. The origin of this software must not be misrepresented; you must not claim that you wrote the

 *    original software. If you use this software in a product, an acknowledgment in the product

 *    documentation would be appreciated but is not required.

 * 2. Altered source versions must be plainly marked as such, and must not be misrepresented as

 *    being the original software.

 * 3. This notice may not be removed or altered from any source distribution.

 */
# 19 "..\\original\\Source\\Doboz\\Compressor.cpp"
# 1 "includes/cstring" 1
# 1 "includes/empty.h" 1
// this is fake header
// it is used for mocking headers which I don't need to be included
# 1 "includes/cstring" 2
# 20 "..\\original\\Source\\Doboz\\Compressor.cpp" 2
# 1 "includes/algorithm" 1
# 1 "includes/empty.h" 1
// this is fake header
// it is used for mocking headers which I don't need to be included
# 1 "includes/algorithm" 2
# 21 "..\\original\\Source\\Doboz\\Compressor.cpp" 2
# 1 "..\\original\\Source\\Doboz\\Compressor.h" 1
/*

 * Doboz Data Compression Library

 * Copyright (C) 2010-2011 Attila T. Afra <attila.afra@gmail.com>

 *

 * This software is provided 'as-is', without any express or implied warranty. In no event will

 * the authors be held liable for any damages arising from the use of this software.

 *

 * Permission is granted to anyone to use this software for any purpose, including commercial

 * applications, and to alter it and redistribute it freely, subject to the following restrictions:

 *

 * 1. The origin of this software must not be misrepresented; you must not claim that you wrote the

 *    original software. If you use this software in a product, an acknowledgment in the product

 *    documentation would be appreciated but is not required.

 * 2. Altered source versions must be plainly marked as such, and must not be misrepresented as

 *    being the original software.

 * 3. This notice may not be removed or altered from any source distribution.

 */
# 19 "..\\original\\Source\\Doboz\\Compressor.h"

# 1 "..\\original\\Source\\Doboz\\Dictionary.h" 1
/*

 * Doboz Data Compression Library

 * Copyright (C) 2010-2011 Attila T. Afra <attila.afra@gmail.com>

 *

 * This software is provided 'as-is', without any express or implied warranty. In no event will

 * the authors be held liable for any damages arising from the use of this software.

 *

 * Permission is granted to anyone to use this software for any purpose, including commercial

 * applications, and to alter it and redistribute it freely, subject to the following restrictions:

 *

 * 1. The origin of this software must not be misrepresented; you must not claim that you wrote the

 *    original software. If you use this software in a product, an acknowledgment in the product

 *    documentation would be appreciated but is not required.

 * 2. Altered source versions must be plainly marked as such, and must not be misrepresented as

 *    being the original software.

 * 3. This notice may not be removed or altered from any source distribution.

 */
# 19 "..\\original\\Source\\Doboz\\Dictionary.h"

namespace doboz {
namespace detail {

class Dictionary
{
public:
 Dictionary();
 ~Dictionary();

 void setBuffer(byte* buffer, int bufferLength);

 int findMatches(Match* matchCandidates);
 void skip();

 int position() const
 {
  return absolutePosition_;
 }

private:
 static const int HASH_TABLE_SIZE = 1 << 20;
 static const int CHILD_COUNT = DICTIONARY_SIZE * 2;
 static const int INVALID_POSITION = -1;
 static const int REBASE_THRESHOLD = (INT_MAX - DICTIONARY_SIZE + 1) / DICTIONARY_SIZE * DICTIONARY_SIZE; // must be a multiple of DICTIONARY_SIZE!

 // Buffer
 byte* buffer_; // pointer to the beginning of the buffer inside which we look for matches
 byte* bufferBase_; // bufferBase_ > buffer_, relative positions are necessary to support > 2 GB buffers
 int bufferLength_;
 int matchableBufferLength_;
 int absolutePosition_; // position from the beginning of buffer_

 // Cyclic dictionary
 int* hashTable_; // relative match positions to bufferBase_
 int* children_; // children of the binary tree nodes (relative match positions to bufferBase_)

 void initialize();

 int computeRelativePosition();
 uint hash(byte* data);
};

} // namespace detail
} // namespace doboz
# 23 "..\\original\\Source\\Doboz\\Compressor.h" 2

namespace doboz {

class Compressor
{
public:
 // Returns the maximum compressed size of any block of data with the specified size
 // This function should be used to determine the size of the compression destination buffer
 static ulong getMaxCompressedSize(ulong size);

 // Compresses a block of data
 // The source and destination buffers must not overlap and their size must be greater than 0
 // This operation is memory safe
 // On success, returns RESULT_OK and outputs the compressed size
 Result compress(byte* source, int sourceSize, byte* destination, int destinationSize, int& compressedSize);

private:
 detail::Dictionary dictionary_;

 static int getSizeCodedSize(ulong size);
 static int getHeaderSize(ulong maxCompressedSize);

 Result store(byte* source, int sourceSize, byte* destination, int& compressedSize);
 detail::Match getBestMatch(detail::Match* matchCandidates, int matchCandidateCount);
 int encodeMatch(const detail::Match& match, byte* destination);
 int getMatchCodedSize(const detail::Match& match);
 void encodeHeader(const detail::Header& header, ulong maxCompressedSize, byte* destination);
};

} // namespace doboz
# 22 "..\\original\\Source\\Doboz\\Compressor.cpp" 2

namespace doboz {

using namespace detail;

Result Compressor::compress(byte* source, int sourceSize, byte* destination, int destinationSize, int& compressedSize)
{
 Debug.Assert(source != 0);
 Debug.Assert(destination != 0);

 if (sourceSize == 0)
 {
  return RESULT_ERROR_BUFFER_TOO_SMALL;
 }
# 48 "..\\original\\Source\\Doboz\\Compressor.cpp"
 ulong storedSize = getMaxCompressedSize(sourceSize);
 ulong maxCompressedSize = destinationSize;

 byte* src = (byte*)(source);
 byte* dst = (byte*)(destination);
 byte* dst_end = dst + destinationSize;
 Debug.Assert((src + sourceSize <= dst || src >= dst_end) && "The source and destination buffers must not overlap.");

 // Compute the maximum output end pointer
 // We use this to determine whether we should store the data instead of compressing it
 byte* maxOutputEnd = dst + ((int)(maxCompressedSize));

 // Allocate the header
 byte* dst_p = dst;
 dst_p += getHeaderSize(maxCompressedSize);

 // Initialize the dictionary
 dictionary_.setBuffer(src, sourceSize);

 // Initialize the control word which contains the literal/match bits
 // The highest bit of a control word is a guard bit, which marks the end of the bit list
 // The guard bit simplifies and speeds up the decoding process, and it
 const int controlWordBitCount = WORD_SIZE * 8 - 1;
 const uint controlWordGuardBit = 1u << controlWordBitCount;
 uint controlWord = controlWordGuardBit;
 int controlWordBit = 0;

 // Since we do not know the contents of the control words in advance, we allocate space for them and subsequently fill them with data as soon as we can
 // This is necessary because the decoder must encounter a control word *before* the literals and matches it refers to
 // We begin the compressed data with a control word
 byte* controlWordPointer = dst_p;
 dst_p += WORD_SIZE;

 // The match located at the current inputIterator position
 Match match;

 // The match located at the next inputIterator position
 // Initialize it to 'no match', because we are at the beginning of the inputIterator buffer
 // A match with a length of 0 means that there is no match
 Match nextMatch;
 nextMatch.length = 0;

 // The dictionary matching look-ahead is 1 character, so set the dictionary position to 1
 // We don't have to worry about getting matches beyond the inputIterator, because the dictionary ignores such requests
 dictionary_.skip();

 // At each position, we select the best match to encode from a list of match candidates provided by the match finder
 Match matchCandidates[MAX_MATCH_CANDIDATE_COUNT];
 int matchCandidateCount;

 // Iterate while there is still data left
 while (dictionary_.position() - 1 < sourceSize)
 {
  // Check whether the output is too large
  // During each iteration, we may output up to 8 bytes (2 words), and the compressed stream ends with 4 dummy bytes
  if (dst_p + 2 * WORD_SIZE + TRAILING_DUMMY_SIZE > maxOutputEnd)
  {

   if (storedSize <= destinationSize)
   {
    return store(source, sourceSize, destination, compressedSize);
   }
   return RESULT_ERROR_BUFFER_TOO_SMALL;

  }

  // Check whether the control word must be flushed
  if (controlWordBit == controlWordBitCount)
  {
   // Flush current control word
   *((uint*)(controlWordPointer)) = (uint)(controlWord);

   // New control word
   controlWord = controlWordGuardBit;
   controlWordBit = 0;

   controlWordPointer = dst_p;
   dst_p += WORD_SIZE;
  }

  // The current match is the previous 'next' match
  match = nextMatch;

  // Find the best match at the next position
  // The dictionary position is automatically incremented
  matchCandidateCount = dictionary_.findMatches(matchCandidates);
  nextMatch = getBestMatch(matchCandidates, matchCandidateCount);

  // If we have a match, do not immediately use it, because we may miss an even better match (lazy evaluation)
  // If encoding a literal and the next match has a higher compression ratio than encoding the current match, discard the current match
  if (match.length > 0 && (1 + nextMatch.length) * getMatchCodedSize(match) > match.length * (1 + getMatchCodedSize(nextMatch)))
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
   *(dst_p) = (byte)(src[dictionary_.position() - 2]);
   ++dst_p;
  }
  else
  {
   // Encode a match (1 control word flag)
   controlWord |= 1 << controlWordBit;

   Debug.Assert(dst_p + WORD_SIZE <= dst_end);
   dst_p += encodeMatch(match, dst_p);

   // Skip the matched characters
   for (int i = 0; i < match.length - 2; ++i)
   {
    dictionary_.skip();
   }

   matchCandidateCount = dictionary_.findMatches(matchCandidates);
   nextMatch = getBestMatch(matchCandidates, matchCandidateCount);
  }

  // Next control word bit
  ++controlWordBit;
 }

 // Flush the control word
 *((uint*)(controlWordPointer)) = (uint)(controlWord);

 // Output trailing safety dummy bytes
 // This reduces the number of necessary buffer checks during decoding
 Debug.Assert(dst_p + TRAILING_DUMMY_SIZE <= dst_end);
 *((uint*)(dst_p)) = (uint)(0);
 dst_p += TRAILING_DUMMY_SIZE;

 // Done, compute the compressed size
 compressedSize = dst_p - dst;

 // Encode the header
 Header header;
 header.version = VERSION;
 header.isStored = false;
 header.uncompressedSize = sourceSize;
 header.compressedSize = compressedSize;

 encodeHeader(header, maxCompressedSize, dst);

 // Return the compressed size
 return RESULT_OK;
}

// Store the source
Result Compressor::store(byte* source, int sourceSize, byte* destination, int& compressedSize)
{
 byte* dst = (byte*)(destination);
 byte* dst_p = dst;

 // Encode the header
 ulong maxCompressedSize = getMaxCompressedSize(sourceSize);
 int headerSize = getHeaderSize(maxCompressedSize);

 compressedSize = headerSize + sourceSize;

 Header header;

 header.version = VERSION;
 header.isStored = true;
 header.uncompressedSize = sourceSize;
 header.compressedSize = compressedSize;

 encodeHeader(header, maxCompressedSize, destination);
 dst_p += headerSize;

 // Store the data
 memcpy(dst_p, source, sourceSize);

 return RESULT_OK;
}

// Selects the best match from a list of match candidates provided by the match finder
Match Compressor::getBestMatch(Match* matchCandidates, int matchCandidateCount)
{
 Match bestMatch;
 bestMatch.length = 0;

 // Select the longest match which can be coded efficiently (coded size is less than the length)
 for (int i = matchCandidateCount - 1; i >= 0; --i)
 {
  if (matchCandidates[i].length > getMatchCodedSize(matchCandidates[i]))
  {
   bestMatch = matchCandidates[i];
   break;
  }
 }

 return bestMatch;
}

int Compressor::encodeMatch(const Match& match, byte* destination)
{
 Debug.Assert(match.length <= MAX_MATCH_LENGTH);
 Debug.Assert(match.length == 0 || match.offset < DICTIONARY_SIZE);

 uint word;
 int size;

 uint lengthCode = (uint)(match.length - MIN_MATCH_LENGTH);
 uint offsetCode = (uint)(match.offset);

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

 if (destination != 0)
 {
  { switch (size) { case 4: case 3: *((uint*)(destination)) = (uint)(word); break; case 2: *((ushort*)(destination)) = (ushort)(word); break; default: *(destination) = (byte)(word); } };
 }

 return size;
}

int Compressor::getMatchCodedSize(const Match& match)
{
 return encodeMatch(match, 0);
}

int Compressor::getSizeCodedSize(ulong size)
{
 if (size <= byte.MaxValue)
 {
  return 1;
 }

 if (size <= ushort.MaxValue)
 {
  return 2;
 }

 if (size <= uint.MaxValue)
 {
  return 4;
 }

 return 8;
}

int Compressor::getHeaderSize(ulong maxCompressedSize)
{
 return 1 + 2 * getSizeCodedSize(maxCompressedSize);
}

void Compressor::encodeHeader(const Header& header, ulong maxCompressedSize, byte* destination)
{
 Debug.Assert(header.version < 8);

 byte* dst_p = (byte*)(destination);

 // Encode the attribute byte
 uint attributes = header.version;

 uint sizeCodedSize = getSizeCodedSize(maxCompressedSize);
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
  *((byte*)(dst_p)) = (byte)(header.uncompressedSize);
  *((byte*)(dst_p + sizeCodedSize)) = (byte)(header.compressedSize);
  break;

 case 2:
  *((ushort*)(dst_p)) = (ushort)(header.uncompressedSize);
  *((ushort*)(dst_p + sizeCodedSize)) = (ushort)(header.compressedSize);
  break;

 case 4:
  *((uint*)(dst_p)) = (uint)(header.uncompressedSize);
  *((uint*)(dst_p + sizeCodedSize)) = (uint)(header.compressedSize);
  break;

 case 8:
  *((ulong*)(dst_p)) = header.uncompressedSize;;
  *((ulong*)(dst_p + sizeCodedSize)) = header.compressedSize;
  break;
 }
}

ulong Compressor::getMaxCompressedSize(ulong size)
{
 // The header + the original uncompressed data
 return getHeaderSize(size) + size;
}

} // namespace doboz
# 57 "doboz_cs_adapter.h" 2
# 1 "..\\original\\Source\\Doboz\\Dictionary.cpp" 1
/*

 * Doboz Data Compression Library

 * Copyright (C) 2010-2011 Attila T. Afra <attila.afra@gmail.com>

 *

 * This software is provided 'as-is', without any express or implied warranty. In no event will

 * the authors be held liable for any damages arising from the use of this software.

 *

 * Permission is granted to anyone to use this software for any purpose, including commercial

 * applications, and to alter it and redistribute it freely, subject to the following restrictions:

 *

 * 1. The origin of this software must not be misrepresented; you must not claim that you wrote the

 *    original software. If you use this software in a product, an acknowledgment in the product

 *    documentation would be appreciated but is not required.

 * 2. Altered source versions must be plainly marked as such, and must not be misrepresented as

 *    being the original software.

 * 3. This notice may not be removed or altered from any source distribution.

 */
# 19 "..\\original\\Source\\Doboz\\Dictionary.cpp"
# 1 "includes/algorithm" 1
# 1 "includes/empty.h" 1
// this is fake header
// it is used for mocking headers which I don't need to be included
# 1 "includes/algorithm" 2
# 20 "..\\original\\Source\\Doboz\\Dictionary.cpp" 2

namespace doboz {
namespace detail {

Dictionary::Dictionary()
 : hashTable_(0), children_(0)
{
 Debug.Assert(INVALID_POSITION < 0);
 Debug.Assert(REBASE_THRESHOLD > DICTIONARY_SIZE && REBASE_THRESHOLD % DICTIONARY_SIZE == 0);
}

Dictionary::~Dictionary()
{
 delete[] hashTable_;
 delete[] children_;
}

void Dictionary::initialize()
{
 // Create the hash table
 hashTable_ = new int[HASH_TABLE_SIZE];

 // Create the tree nodes
 // The number of nodes is equal to the size of the dictionary, and every node has two children
 children_ = new int[CHILD_COUNT];
}

void Dictionary::setBuffer(byte* buffer, int bufferLength)
{
 // Set the buffer
 buffer_ = buffer;
 bufferLength_ = bufferLength;
 absolutePosition_ = 0;

 // Compute the matchable buffer length
 if (bufferLength_ > TAIL_LENGTH + MIN_MATCH_LENGTH)
 {
  matchableBufferLength_ = bufferLength_ - (TAIL_LENGTH + MIN_MATCH_LENGTH);
 }
 else
 {
  matchableBufferLength_ = 0;
 }

 // Since we always store 32-bit positions in the dictionary, we need relative positions in order to support buffers larger then 2 GB
 // This can be possible, because the difference between any two positions stored in the dictionary never exceeds the size of the dictionary
 // We don't store larger (64-bit) positions, because that can significantly degrade performance
 // Initialize the relative position base pointer
 bufferBase_ = buffer_;

 // Initialize if necessary
 if (hashTable_ == 0)
 {
  initialize();
 }

 // Clear the hash table
 for (int i = 0; i < HASH_TABLE_SIZE; ++i)
 {
  hashTable_[i] = INVALID_POSITION;
 }
}

// Finds match candidates at the current buffer position and slides the matching window to the next character
// Call findMatches/update with increasing positions
// The match candidates are stored in the supplied array, ordered by their length (ascending)
// The return value is the number of match candidates in the array
int Dictionary::findMatches(Match* matchCandidates)
{
 Debug.Assert(hashTable_ != 0 && "No buffer is set.");

 // Check whether we can find matches at this position
 if (absolutePosition_ >= matchableBufferLength_)
 {
  // Slide the matching window with one character
  ++absolutePosition_;

  return 0;
 }

 // Compute the maximum match length
 int maxMatchLength = (int)(std::min(bufferLength_ - TAIL_LENGTH - absolutePosition_, ((int)(MAX_MATCH_LENGTH))));

 // Compute the position relative to the beginning of bufferBase_
 // All other positions (including the ones stored in the hash table and the binary trees) are relative too
 // From now on, we can safely ignore this position technique
 int position = computeRelativePosition();

 // Compute the minimum match position
 int minMatchPosition = (position < DICTIONARY_SIZE) ? 0 : (position - DICTIONARY_SIZE + 1);

 // Compute the hash value for the current string
 int hashValue = hash(bufferBase_ + position) % HASH_TABLE_SIZE;

 // Get the position of the first match from the hash table
 int matchPosition = hashTable_[hashValue];

 // Set the current string as the root of the binary tree corresponding to the hash table entry
 hashTable_[hashValue] = position;

 // Compute the current cyclic position in the dictionary
 int cyclicInputPosition = position % DICTIONARY_SIZE;

 // Initialize the references to the leaves of the new root's left and right subtrees
 int leftSubtreeLeaf = cyclicInputPosition * 2;
 int rightSubtreeLeaf = cyclicInputPosition * 2 + 1;

 // Initialize the match lenghts of the lower and upper bounds of the current string (lowMatch < match < highMatch)
 // We use these to avoid unneccesary character comparisons at the beginnings of the strings
 int lowMatchLength = 0;
 int highMatchLength = 0;

 // Initialize the longest match length
 int longestMatchLength = 0;

 // Find matches
 // We look for the current string in the binary search tree and we rebuild the tree at the same time
 // The deeper a node is in the tree, the lower is its position, so the root is the string with the highest position (lowest offset)

 // We count the number of match attempts, and exit if it has reached a certain threshold
 int matchCount = 0;

 // Match candidates are matches which are longer than any previously encountered ones
 int matchCandidateCount = 0;

 while (true)
 {
  // Check whether the current match position is valid
  if (matchPosition < minMatchPosition || matchCount == MAX_MATCH_CANDIDATE_COUNT)
  {
   // We have checked all valid matches, so finish the new tree and exit
   children_[leftSubtreeLeaf] = INVALID_POSITION;
   children_[rightSubtreeLeaf] = INVALID_POSITION;
   break;
  }

  ++matchCount;

  // Compute the cyclic position of the current match in the dictionary
  int cyclicMatchPosition = matchPosition % DICTIONARY_SIZE;

  // Use the match lengths of the low and high bounds to determine the number of characters that surely match
  int matchLength = std::min(lowMatchLength, highMatchLength);

  // Determine the match length
  while (matchLength < maxMatchLength && bufferBase_[position + matchLength] == bufferBase_[matchPosition + matchLength])
  {
   ++matchLength;
  }

  // Check whether this match is the longest so far
  int matchOffset = position - matchPosition;

  if (matchLength > longestMatchLength && matchLength >= MIN_MATCH_LENGTH)
  {
   longestMatchLength = matchLength;

   // Add the current best match to the list of good match candidates
   if (matchCandidates != 0)
   {
    matchCandidates[matchCandidateCount].length = matchLength;
    matchCandidates[matchCandidateCount].offset = matchOffset;
    ++matchCandidateCount;
   }

   // If the match length is the maximum allowed value, the current string is already inserted into the tree: the current node
   if (matchLength == maxMatchLength)
   {
    // Since the current string is also the root of the tree, delete the current node
    children_[leftSubtreeLeaf] = children_[cyclicMatchPosition * 2];
    children_[rightSubtreeLeaf] = children_[cyclicMatchPosition * 2 + 1];
    break;
   }
  }

  // Compare the two strings
  if (bufferBase_[position + matchLength] < bufferBase_[matchPosition + matchLength])
  {
   // Insert the matched string into the right subtree
   children_[rightSubtreeLeaf] = matchPosition;

   // Go left
   rightSubtreeLeaf = cyclicMatchPosition * 2;
   matchPosition = children_[rightSubtreeLeaf];

   // Update the match length of the high bound
   highMatchLength = matchLength;
  }
  else
  {
   // Insert the matched string into the left subtree
   children_[leftSubtreeLeaf] = matchPosition;

   // Go right
   leftSubtreeLeaf = cyclicMatchPosition * 2 + 1;
   matchPosition = children_[leftSubtreeLeaf];

   // Update the match length of the low bound
   lowMatchLength = matchLength;
  }
 }

 // Slide the matching window with one character
 ++absolutePosition_;

 return matchCandidateCount;
}

// Increments the match window position with one character
int Dictionary::computeRelativePosition()
{
 int position = (int)(absolutePosition_ - (bufferBase_ - buffer_));

 // Check whether the current position has reached the rebase threshold
 if (position == REBASE_THRESHOLD)
 {
  // Rebase
  int rebaseDelta = REBASE_THRESHOLD - DICTIONARY_SIZE;
  Debug.Assert(rebaseDelta % DICTIONARY_SIZE == 0);

  bufferBase_ += rebaseDelta;
  position -= rebaseDelta;

  // Rebase the hash entries
  for (int i = 0; i < HASH_TABLE_SIZE; ++i)
  {
   hashTable_[i] = (hashTable_[i] >= rebaseDelta) ? (hashTable_[i] - rebaseDelta) : INVALID_POSITION;
  }

  // Rebase the binary tree nodes
  for (int i = 0; i < CHILD_COUNT; ++i)
  {
   children_[i] = (children_[i] >= rebaseDelta) ? (children_[i] - rebaseDelta) : INVALID_POSITION;
  }
 }

 return position;
}

// Slides the matching window to the next character without looking for matches, but it still has to update the dictionary
void Dictionary::skip()
{
 findMatches(0);
}

uint Dictionary::hash(byte* data)
{
 // FNV-1a hash
 const uint prime = 16777619;
 uint result = 2166136261;

 result = (result ^ data[0]) * prime;
 result = (result ^ data[1]) * prime;
 result = (result ^ data[2]) * prime;

 return result;
}

} // namespace detail
} // namespace doboz
# 58 "doboz_cs_adapter.h" 2

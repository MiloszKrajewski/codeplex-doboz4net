# 1 "doboz_cs_adapter.h"
# 1 "<command-line>"
# 1 "doboz_cs_adapter.h"
# 46 "doboz_cs_adapter.h"
# 1 "..\\original\\Source\\Doboz\\Decompressor.cpp" 1
# 19 "..\\original\\Source\\Doboz\\Decompressor.cpp"
# 1 "includes/cstring" 1
# 1 "includes/empty.h" 1
# 1 "includes/cstring" 2
# 20 "..\\original\\Source\\Doboz\\Decompressor.cpp" 2
# 1 "..\\original\\Source\\Doboz\\Decompressor.h" 1
# 19 "..\\original\\Source\\Doboz\\Decompressor.h"
       

# 1 "..\\original\\Source\\Doboz\\Common.h" 1
# 19 "..\\original\\Source\\Doboz\\Common.h"
       

# 1 "includes/stdint.h" 1
# 1 "includes/empty.h" 1
# 1 "includes/stdint.h" 2
# 22 "..\\original\\Source\\Doboz\\Common.h" 2
# 1 "includes/climits" 1
# 1 "includes/empty.h" 1
# 1 "includes/climits" 2
# 23 "..\\original\\Source\\Doboz\\Common.h" 2
# 1 "includes/cassert" 1
# 1 "includes/empty.h" 1
# 1 "includes/cassert" 2
# 24 "..\\original\\Source\\Doboz\\Common.h" 2
# 33 "..\\original\\Source\\Doboz\\Common.h"
namespace doboz {

const int VERSION = 0;

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


const int WORD_SIZE = 4;

const int MIN_MATCH_LENGTH = 3;
const int MAX_MATCH_LENGTH = 255 + MIN_MATCH_LENGTH;
const int MAX_MATCH_CANDIDATE_COUNT = 128;
const int DICTIONARY_SIZE = 1 << 21;

const int TAIL_LENGTH = 2 * WORD_SIZE;
const int TRAILING_DUMMY_SIZE = WORD_SIZE;




inline uint PeekN_size_t size(const void* source)
{
 Debug.Assert(size <= WORD_SIZE);

 switch (size)
 {
 case 4:
  return *reinterpret_cast<const uint*>(source);

 case 3:
  return *reinterpret_cast<const uint*>(source);

 case 2:
  return *reinterpret_cast<const uint16_t*>(source);

 case 1:
  return *reinterpret_cast<const byte*>(source);

 default:
  return 0;
 }
}



inline void PokeN_size_t size(void* destination,uint word)
{
 Debug.Assert(size <= WORD_SIZE);

 switch (size)
 {
 case 4:
  *reinterpret_cast<uint*>(destination) = word;
  break;

 case 3:
  *reinterpret_cast<uint*>(destination) = word;
  break;

 case 2:
  *reinterpret_cast<uint16_t*>(destination) = static_cast<uint16_t>(word);
  break;

 case 1:
  *reinterpret_cast<byte*>(destination) = static_cast<byte>(word);
  break;
 }
}

}

}
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




 Result decompress(const void* source, int sourceSize, void* destination, int destinationSize);




 Result getCompressionInfo(const void* source, int sourceSize, CompressionInfo& compressionInfo);

private:
 int decodeMatch(detail::Match& match, const void* source);
 Result decodeHeader(detail::Header& header, const void* source, int sourceSize, int& headerSize);
};

}
# 21 "..\\original\\Source\\Doboz\\Decompressor.cpp" 2

namespace doboz {

using namespace detail;

Result Decompressor::decompress(const void* source, int sourceSize, void* destination, int destinationSize)
{
 Debug.Assert(source != 0);
 Debug.Assert(destination != 0);

 const byte* inputBuffer = static_cast<const byte*>(source);
 const byte* src_p = inputBuffer;

 byte* outputBuffer = static_cast<byte*>(destination);
 byte* dst_p = outputBuffer;

 Debug.Assert((inputBuffer + sourceSize <= outputBuffer || inputBuffer >= outputBuffer + destinationSize) && "The source and destination buffers must not overlap.")
                                                         ;


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


 if (sourceSize < header.compressedSize || destinationSize < header.uncompressedSize)
 {
  return RESULT_ERROR_BUFFER_TOO_SMALL;
 }

 int uncompressedSize = static_cast<int>(header.uncompressedSize);


 if (header.isStored)
 {
  memcpy(outputBuffer, src_p, uncompressedSize);
  return RESULT_OK;
 }

 const byte* inputEnd = inputBuffer + static_cast<int>(header.compressedSize);
 byte* dst_end = outputBuffer + uncompressedSize;



 byte* outputTail = (uncompressedSize > TAIL_LENGTH) ? (dst_end - TAIL_LENGTH) : outputBuffer;


 uint controlWord = 1;


 for (; ;)
 {



  if (src_p + 2 * WORD_SIZE > inputEnd)
  {
   return RESULT_ERROR_CORRUPTED_DATA;
  }


  if (controlWord == 1)
  {
   Debug.Assert(src_p + WORD_SIZE <= inputEnd);
   controlWord = (*((uint*)src_p));
   src_p += WORD_SIZE;
  }


  if ((controlWord & 1) == 0)
  {



   if (dst_p < outputTail)
   {



    Debug.Assert(src_p + WORD_SIZE <= inputEnd);
    Debug.Assert(dst_p + WORD_SIZE <= dst_end);
    *((uint*)(dst_p)) = ((uint)((*((uint*)src_p))));


    static const sbyte literalRunLengthTable[16] = {4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0};
    int runLength = literalRunLengthTable[controlWord & 0xf];


    src_p += runLength;
    dst_p += runLength;


    controlWord >>= runLength;
   }
   else
   {


    while (dst_p < dst_end)
    {


     if (src_p + WORD_SIZE + 1 > inputEnd)
     {
      return RESULT_ERROR_CORRUPTED_DATA;
     }


     if (controlWord == 1)
     {
      Debug.Assert(src_p + WORD_SIZE <= inputEnd);
      controlWord = (*((uint*)src_p));
      src_p += WORD_SIZE;
     }



     Debug.Assert(src_p + 1 <= inputEnd);
     Debug.Assert(dst_p + 1 <= dst_end);
     *dst_p++ = *src_p++;


     controlWord >>= 1;
    }


    return RESULT_OK;
   }
  }
  else
  {



   Debug.Assert(src_p + WORD_SIZE <= inputEnd);
   Match match;
   src_p += decodeMatch(match, src_p);




   byte* matchString = dst_p - match.offset;


   if (matchString < outputBuffer || dst_p + match.length > outputTail)
   {
    return RESULT_ERROR_CORRUPTED_DATA;
   }

   int i = 0;

   if (match.offset < WORD_SIZE)
   {


    do
    {
     Debug.Assert(matchString + i >= outputBuffer);
     Debug.Assert(matchString + i + WORD_SIZE <= dst_end);
     Debug.Assert(dst_p + i + WORD_SIZE <= dst_end);
     *(dst_p + i) = ((byte)((*(matchString + i))));
     ++i;
    }
    while (i < 3);



    matchString -= 2 + (match.offset & 1);
   }




   do
   {
    Debug.Assert(matchString + i >= outputBuffer);
    Debug.Assert(matchString + i + WORD_SIZE <= dst_end);
    Debug.Assert(dst_p + i + WORD_SIZE <= dst_end);
    *((uint*)(dst_p + i)) = ((uint)((*((uint*)matchString + i))));
    i += WORD_SIZE;
   }
   while (i < match.length);

   dst_p += match.length;


   controlWord >>= 1;
  }
 }
}


inline int Decompressor::decodeMatch(Match& match, const void* source)
{

 static const struct
 {
  uint mask;
  byte offsetShift;
  byte lengthMask;
  byte lengthShift;
  sbyte size;
 }
 lut[] =
 {
  {0xff, 2, 0, 0, 1},
  {0xffff, 2, 0, 0, 2},
  {0xffff, 6, 15, 2, 2},
  {0xffffff, 8, 31, 3, 3},
  {0xff, 2, 0, 0, 1},
  {0xffff, 2, 0, 0, 2},
  {0xffff, 6, 15, 2, 2},
  {0xffffffff, 11, 255, 3, 4},
 };


 uint word = (*((uint*)source));


 uint i = word & 7;


 match.offset = static_cast<int>((word & lut[i].mask) >> lut[i].offsetShift);
 match.length = static_cast<int>(((word >> lut[i].lengthShift) & lut[i].lengthMask) + MIN_MATCH_LENGTH);

 return lut[i].size;
}



Result Decompressor::decodeHeader(Header& header, const void* source, int sourceSize, int& headerSize)
{
 const byte* src_p = static_cast<const byte*>(source);


 if (sourceSize < 1)
 {
  return RESULT_ERROR_BUFFER_TOO_SMALL;
 }

 uint attributes = *src_p++;

 header.version = attributes & 7;
 int sizeCodedSize = ((attributes >> 3) & 7) + 1;


 headerSize = 1 + 2 * sizeCodedSize;

 if (sourceSize < static_cast<int>(headerSize))
 {
  return RESULT_ERROR_BUFFER_TOO_SMALL;
 }

 header.isStored = (attributes & 128) != 0;


 switch (sizeCodedSize)
 {
 case 1:
  header.uncompressedSize = *reinterpret_cast<const byte*>(src_p);
  header.compressedSize = *reinterpret_cast<const byte*>(src_p + sizeCodedSize);
  break;

 case 2:
  header.uncompressedSize = *reinterpret_cast<const uint16_t*>(src_p);
  header.compressedSize = *reinterpret_cast<const uint16_t*>(src_p + sizeCodedSize);
  break;

 case 4:
  header.uncompressedSize = *reinterpret_cast<const uint*>(src_p);
  header.compressedSize = *reinterpret_cast<const uint*>(src_p + sizeCodedSize);
  break;

 case 8:
  header.uncompressedSize = *reinterpret_cast<const ulong*>(src_p);
  header.compressedSize = *reinterpret_cast<const ulong*>(src_p + sizeCodedSize);
  break;

 default:
  return RESULT_ERROR_CORRUPTED_DATA;
 }

 return RESULT_OK;
}

Result Decompressor::getCompressionInfo(const void* source, int sourceSize, CompressionInfo& compressionInfo)
{
 Debug.Assert(source != 0);


 Header header;
 int headerSize;
 Result decodeHeaderResult = decodeHeader(header, source, sourceSize, headerSize);

 if (decodeHeaderResult != RESULT_OK)
 {
  return decodeHeaderResult;
 }


 compressionInfo.uncompressedSize = header.uncompressedSize;
 compressionInfo.compressedSize = header.compressedSize;
 compressionInfo.version = header.version;

 return RESULT_OK;
}

}
# 47 "doboz_cs_adapter.h" 2



# 1 "..\\original\\Source\\Doboz\\Compressor.cpp" 1
# 19 "..\\original\\Source\\Doboz\\Compressor.cpp"
# 1 "includes/cstring" 1
# 1 "includes/empty.h" 1
# 1 "includes/cstring" 2
# 20 "..\\original\\Source\\Doboz\\Compressor.cpp" 2
# 1 "includes/algorithm" 1
# 1 "includes/empty.h" 1
# 1 "includes/algorithm" 2
# 21 "..\\original\\Source\\Doboz\\Compressor.cpp" 2
# 1 "..\\original\\Source\\Doboz\\Compressor.h" 1
# 19 "..\\original\\Source\\Doboz\\Compressor.h"
       


# 1 "..\\original\\Source\\Doboz\\Dictionary.h" 1
# 19 "..\\original\\Source\\Doboz\\Dictionary.h"
       



namespace doboz {
namespace detail {

class Dictionary
{
public:
 Dictionary();
 ~Dictionary();

 void setBuffer(const byte* buffer, int bufferLength);

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
 static const int REBASE_THRESHOLD = (INT_MAX - DICTIONARY_SIZE + 1) / DICTIONARY_SIZE * DICTIONARY_SIZE;


 const byte* buffer_;
 const byte* bufferBase_;
 int bufferLength_;
 int matchableBufferLength_;
 int absolutePosition_;


 int* hashTable_;
 int* children_;

 void initialize();

 int computeRelativePosition();
 uint hash(const byte* data);
};

}
}
# 23 "..\\original\\Source\\Doboz\\Compressor.h" 2

namespace doboz {

class Compressor
{
public:


 static ulong getMaxCompressedSize(ulong size);





 Result compress(const void* source, int sourceSize, void* destination, int destinationSize, int& compressedSize);

private:
 detail::Dictionary dictionary_;

 static int getSizeCodedSize(ulong size);
 static int getHeaderSize(ulong maxCompressedSize);

 Result store(const void* source, int sourceSize, void* destination, int& compressedSize);
 detail::Match getBestMatch(detail::Match* matchCandidates, int matchCandidateCount);
 int encodeMatch(const detail::Match& match, void* destination);
 int getMatchCodedSize(const detail::Match& match);
 void encodeHeader(const detail::Header& header, ulong maxCompressedSize, void* destination);
};

}
# 22 "..\\original\\Source\\Doboz\\Compressor.cpp" 2

namespace doboz {

using namespace detail;

Result Compressor::compress(const void* source, int sourceSize, void* destination, int destinationSize, int& compressedSize)
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


 const byte* inputBuffer = static_cast<const byte*>(source);
 byte* outputBuffer = static_cast<byte*>(destination);
 byte* dst_end = outputBuffer + destinationSize;
 Debug.Assert((inputBuffer + sourceSize <= outputBuffer || inputBuffer >= dst_end) && "The source and destination buffers must not overlap.");



 byte* maxOutputEnd = outputBuffer + static_cast<int>(maxCompressedSize);


 byte* dst_p = outputBuffer;
 dst_p += getHeaderSize(maxCompressedSize);


 dictionary_.setBuffer(inputBuffer, sourceSize);




 const int controlWordBitCount = WORD_SIZE * 8 - 1;
 const uint controlWordGuardBit = 1u << controlWordBitCount;
 uint controlWord = controlWordGuardBit;
 int controlWordBit = 0;




 byte* controlWordPointer = dst_p;
 dst_p += WORD_SIZE;


 Match match;




 Match nextMatch;
 nextMatch.length = 0;



 dictionary_.skip();


 Match matchCandidates[MAX_MATCH_CANDIDATE_COUNT];
 int matchCandidateCount;


 while (dictionary_.position() - 1 < sourceSize)
 {


  if (dst_p + 2 * WORD_SIZE + TRAILING_DUMMY_SIZE > maxOutputEnd)
  {




   if (storedSize <= destinationSize)
   {
    return store(source, sourceSize, destination, compressedSize);
   }
   return RESULT_ERROR_BUFFER_TOO_SMALL;

  }


  if (controlWordBit == controlWordBitCount)
  {

   *((uint*)(controlWordPointer)) = ((uint)(controlWord));


   controlWord = controlWordGuardBit;
   controlWordBit = 0;

   controlWordPointer = dst_p;
   dst_p += WORD_SIZE;
  }


  match = nextMatch;



  matchCandidateCount = dictionary_.findMatches(matchCandidates);
  nextMatch = getBestMatch(matchCandidates, matchCandidateCount);



  if (match.length > 0 && (1 + nextMatch.length) * getMatchCodedSize(match) > match.length * (1 + getMatchCodedSize(nextMatch)))
  {
   match.length = 0;
  }


  if (match.length == 0)
  {




   Debug.Assert(dst_p + 1 <= dst_end);
   *(dst_p) = ((byte)(inputBuffer[dictionary_.position() - 2]));
   ++dst_p;
  }
  else
  {

   controlWord |= 1 << controlWordBit;

   Debug.Assert(dst_p + WORD_SIZE <= dst_end);
   dst_p += encodeMatch(match, dst_p);


   for (int i = 0; i < match.length - 2; ++i)
   {
    dictionary_.skip();
   }

   matchCandidateCount = dictionary_.findMatches(matchCandidates);
   nextMatch = getBestMatch(matchCandidates, matchCandidateCount);
  }


  ++controlWordBit;
 }


 *((uint*)(controlWordPointer)) = ((uint)(controlWord));



 Debug.Assert(dst_p + TRAILING_DUMMY_SIZE <= dst_end);
 *((uint*)(dst_p)) = ((uint)(0));
 dst_p += TRAILING_DUMMY_SIZE;


 compressedSize = dst_p - outputBuffer;


 Header header;
 header.version = VERSION;
 header.isStored = false;
 header.uncompressedSize = sourceSize;
 header.compressedSize = compressedSize;

 encodeHeader(header, maxCompressedSize, outputBuffer);


 return RESULT_OK;
}


Result Compressor::store(const void* source, int sourceSize, void* destination, int& compressedSize)
{
 byte* outputBuffer = static_cast<byte*>(destination);
 byte* dst_p = outputBuffer;


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


 memcpy(dst_p, source, sourceSize);

 return RESULT_OK;
}


Match Compressor::getBestMatch(Match* matchCandidates, int matchCandidateCount)
{
 Match bestMatch;
 bestMatch.length = 0;


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

int Compressor::encodeMatch(const Match& match, void* destination)
{
 Debug.Assert(match.length <= MAX_MATCH_LENGTH);
 Debug.Assert(match.length == 0 || match.offset < DICTIONARY_SIZE);

 uint word;
 int size;

 uint lengthCode = static_cast<uint>(match.length - MIN_MATCH_LENGTH);
 uint offsetCode = static_cast<uint>(match.offset);

 if (lengthCode == 0 && offsetCode < 64)
 {
  word = offsetCode << 2;
  size = 1;
 }
 else if (lengthCode == 0 && offsetCode < 16384)
 {
  word = (offsetCode << 2) | 1;
  size = 2;
 }
 else if (lengthCode < 16 && offsetCode < 1024)
 {
  word = (offsetCode << 6) | (lengthCode << 2) | 2;
  size = 2;
 }
 else if (lengthCode < 32 && offsetCode < 65536)
 {
  word = (offsetCode << 8) | (lengthCode << 3) | 3;
  size = 3;
 }
 else
 {
  word = (offsetCode << 11) | (lengthCode << 3) | 7;
  size = 4;
 }

 if (destination != 0)
 {
  { switch (size) { case 4: case 3: *((uint*)(destination)) = ((uint)(word)); break; case 2: *((ushort*)(destination)) = ((ushort)(word)); break; default: *(destination) = ((byte)(word)); } };
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

void Compressor::encodeHeader(const Header& header, ulong maxCompressedSize, void* destination)
{
 Debug.Assert(header.version < 8);

 byte* dst_p = static_cast<byte*>(destination);


 uint attributes = header.version;

 uint sizeCodedSize = getSizeCodedSize(maxCompressedSize);
 attributes |= (sizeCodedSize - 1) << 3;

 if (header.isStored)
 {
  attributes |= 128;
 }

 *dst_p++ = static_cast<byte>(attributes);


 switch (sizeCodedSize)
 {
 case 1:
  *reinterpret_cast<byte*>(dst_p) = static_cast<byte>(header.uncompressedSize);
  *reinterpret_cast<byte*>(dst_p + sizeCodedSize) = static_cast<byte>(header.compressedSize);
  break;

 case 2:
  *reinterpret_cast<uint16_t*>(dst_p) = static_cast<uint16_t>(header.uncompressedSize);
  *reinterpret_cast<uint16_t*>(dst_p + sizeCodedSize) = static_cast<uint16_t>(header.compressedSize);
  break;

 case 4:
  *reinterpret_cast<uint*>(dst_p) = static_cast<uint>(header.uncompressedSize);
  *reinterpret_cast<uint*>(dst_p + sizeCodedSize) = static_cast<uint>(header.compressedSize);
  break;

 case 8:
  *reinterpret_cast<ulong*>(dst_p) = header.uncompressedSize;;
  *reinterpret_cast<ulong*>(dst_p + sizeCodedSize) = header.compressedSize;
  break;
 }
}

ulong Compressor::getMaxCompressedSize(ulong size)
{

 return getHeaderSize(size) + size;
}

}
# 51 "doboz_cs_adapter.h" 2
# 1 "..\\original\\Source\\Doboz\\Dictionary.cpp" 1
# 19 "..\\original\\Source\\Doboz\\Dictionary.cpp"
# 1 "includes/algorithm" 1
# 1 "includes/empty.h" 1
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

 hashTable_ = new int[HASH_TABLE_SIZE];



 children_ = new int[CHILD_COUNT];
}

void Dictionary::setBuffer(const byte* buffer, int bufferLength)
{

 buffer_ = buffer;
 bufferLength_ = bufferLength;
 absolutePosition_ = 0;


 if (bufferLength_ > TAIL_LENGTH + MIN_MATCH_LENGTH)
 {
  matchableBufferLength_ = bufferLength_ - (TAIL_LENGTH + MIN_MATCH_LENGTH);
 }
 else
 {
  matchableBufferLength_ = 0;
 }





 bufferBase_ = buffer_;


 if (hashTable_ == 0)
 {
  initialize();
 }


 for (int i = 0; i < HASH_TABLE_SIZE; ++i)
 {
  hashTable_[i] = INVALID_POSITION;
 }
}





int Dictionary::findMatches(Match* matchCandidates)
{
 Debug.Assert(hashTable_ != 0 && "No buffer is set.");


 if (absolutePosition_ >= matchableBufferLength_)
 {

  ++absolutePosition_;

  return 0;
 }


 int maxMatchLength = static_cast<int>(std::min(bufferLength_ - TAIL_LENGTH - absolutePosition_, static_cast<int>(MAX_MATCH_LENGTH)));




 int position = computeRelativePosition();


 int minMatchPosition = (position < DICTIONARY_SIZE) ? 0 : (position - DICTIONARY_SIZE + 1);


 int hashValue = hash(bufferBase_ + position) % HASH_TABLE_SIZE;


 int matchPosition = hashTable_[hashValue];


 hashTable_[hashValue] = position;


 int cyclicInputPosition = position % DICTIONARY_SIZE;


 int leftSubtreeLeaf = cyclicInputPosition * 2;
 int rightSubtreeLeaf = cyclicInputPosition * 2 + 1;



 int lowMatchLength = 0;
 int highMatchLength = 0;


 int longestMatchLength = 0;






 int matchCount = 0;


 int matchCandidateCount = 0;

 for (; ;)
 {

  if (matchPosition < minMatchPosition || matchCount == MAX_MATCH_CANDIDATE_COUNT)
  {

   children_[leftSubtreeLeaf] = INVALID_POSITION;
   children_[rightSubtreeLeaf] = INVALID_POSITION;
   break;
  }

  ++matchCount;


  int cyclicMatchPosition = matchPosition % DICTIONARY_SIZE;


  int matchLength = std::min(lowMatchLength, highMatchLength);


  while (matchLength < maxMatchLength && bufferBase_[position + matchLength] == bufferBase_[matchPosition + matchLength])
  {
   ++matchLength;
  }


  int matchOffset = position - matchPosition;

  if (matchLength > longestMatchLength && matchLength >= MIN_MATCH_LENGTH)
  {
   longestMatchLength = matchLength;


   if (matchCandidates != 0)
   {
    matchCandidates[matchCandidateCount].length = matchLength;
    matchCandidates[matchCandidateCount].offset = matchOffset;
    ++matchCandidateCount;
   }


   if (matchLength == maxMatchLength)
   {

    children_[leftSubtreeLeaf] = children_[cyclicMatchPosition * 2];
    children_[rightSubtreeLeaf] = children_[cyclicMatchPosition * 2 + 1];
    break;
   }
  }


  if (bufferBase_[position + matchLength] < bufferBase_[matchPosition + matchLength])
  {

   children_[rightSubtreeLeaf] = matchPosition;


   rightSubtreeLeaf = cyclicMatchPosition * 2;
   matchPosition = children_[rightSubtreeLeaf];


   highMatchLength = matchLength;
  }
  else
  {

   children_[leftSubtreeLeaf] = matchPosition;


   leftSubtreeLeaf = cyclicMatchPosition * 2 + 1;
   matchPosition = children_[leftSubtreeLeaf];


   lowMatchLength = matchLength;
  }
 }


 ++absolutePosition_;

 return matchCandidateCount;
}


int Dictionary::computeRelativePosition()
{
 int position = static_cast<int>(absolutePosition_ - (bufferBase_ - buffer_));


 if (position == REBASE_THRESHOLD)
 {

  int rebaseDelta = REBASE_THRESHOLD - DICTIONARY_SIZE;
  Debug.Assert(rebaseDelta % DICTIONARY_SIZE == 0);

  bufferBase_ += rebaseDelta;
  position -= rebaseDelta;


  for (int i = 0; i < HASH_TABLE_SIZE; ++i)
  {
   hashTable_[i] = (hashTable_[i] >= rebaseDelta) ? (hashTable_[i] - rebaseDelta) : INVALID_POSITION;
  }


  for (int i = 0; i < CHILD_COUNT; ++i)
  {
   children_[i] = (children_[i] >= rebaseDelta) ? (children_[i] - rebaseDelta) : INVALID_POSITION;
  }
 }

 return position;
}


void Dictionary::skip()
{
 findMatches(0);
}

uint Dictionary::hash(const byte* data)
{

 const uint prime = 16777619;
 uint result = 2166136261;

 result = (result ^ data[0]) * prime;
 result = (result ^ data[1]) * prime;
 result = (result ^ data[2]) * prime;

 return result;
}

}
}
# 52 "doboz_cs_adapter.h" 2

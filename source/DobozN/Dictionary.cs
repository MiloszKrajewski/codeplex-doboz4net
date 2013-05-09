#if !DOBOZ_UNSAFE
// by default it is always safe (it's... safer?) :-)
#define DOBOZ_SAFE
#endif

using System;
using System.Diagnostics;

#if DOBOZ_SAFE
namespace DobozS
#else
namespace DobozN
#endif
{
#if DOBOZ_SAFE
	internal class Dictionary
#else
	internal unsafe class Dictionary
#endif
	{
		#region consts

		private const int DICTIONARY_SIZE = DobozCodec.DICTIONARY_SIZE;
		private const int TAIL_LENGTH = DobozDecoder.TAIL_LENGTH;
		private const int MIN_MATCH_LENGTH = DobozDecoder.MIN_MATCH_LENGTH;
		private const int MAX_MATCH_LENGTH = DobozCodec.MAX_MATCH_LENGTH;
		private const int MAX_MATCH_CANDIDATE_COUNT = DobozCodec.MAX_MATCH_CANDIDATE_COUNT;

		private const int HASH_TABLE_SIZE = 1 << 20;
		private const int CHILD_COUNT = DobozCodec.DICTIONARY_SIZE * 2;
		private const int INVALID_POSITION = -1;
		private const int REBASE_THRESHOLD = (int.MaxValue - DICTIONARY_SIZE + 1) / DICTIONARY_SIZE * DICTIONARY_SIZE; // must be a multiple of DICTIONARY_SIZE!
		private const int REBASE_DELTA = REBASE_THRESHOLD - DICTIONARY_SIZE;

		#endregion

		#region fields

		// note: this one is public for performance reasons
		public int Position; // position from the beginning of buffer_

		private readonly int _bufferLength;
		private readonly int _matchableBufferLength;

		// Cyclic dictionary
		private readonly int[] _hashTable; // relative match positions to bufferBase_
		private readonly int[] _children; // children of the binary tree nodes (relative match positions to bufferBase_)

		#endregion

		private Dictionary()
		{
			Debug.Assert(INVALID_POSITION < 0);
			Debug.Assert(REBASE_THRESHOLD % DICTIONARY_SIZE == 0);
			Debug.Assert(REBASE_DELTA % DICTIONARY_SIZE == 0);

			// Create the hash table
			_hashTable = new int[HASH_TABLE_SIZE];

			// Create the tree nodes
			// The number of nodes is equal to the size of the dictionary, and every node has two children
			_children = new int[CHILD_COUNT];
		}

		#region fields

#if DOBOZ_SAFE
		private readonly byte[] _data; // pointer to the beginning of the buffer inside which we look for matches
#else
		private readonly byte* _data; // pointer to the beginning of the buffer inside which we look for matches
#endif
		private int _base;

		#endregion

		#region constructor

#if DOBOZ_SAFE
		public Dictionary(byte[] buffer, int bufferOffset, int bufferLength)
#else
		public Dictionary(byte* buffer, int bufferLength)
#endif
			: this()
		{
			// Set the buffer
			_data = buffer;
			_bufferLength = bufferLength;
			Position = 0;

			// Compute the matchable buffer length
			if (_bufferLength > TAIL_LENGTH + MIN_MATCH_LENGTH)
			{
				_matchableBufferLength = _bufferLength - (TAIL_LENGTH + MIN_MATCH_LENGTH);
			}
			else
			{
				_matchableBufferLength = 0;
			}

			// Since we always store 32-bit positions in the dictionary, we need relative positions in order to 
			// support buffers larger then 2 GB
			// This can be possible, because the difference between any two positions stored in the dictionary 
			// never exceeds the size of the dictionary We don't store larger (64-bit) positions, because that 
			// can significantly degrade performance
			// Initialize the relative position base pointer
#if DOBOZ_SAFE
			_base = bufferOffset;
			var ht = _hashTable;
#else
			_base = 0;
			fixed (int* ht = _hashTable)
#endif
			{
				// Clear the hash table
				for (var i = 0; i < HASH_TABLE_SIZE; ++i) ht[i] = INVALID_POSITION;
			}
		}

		#endregion

		#region public interface

		public void Skip()
		{
#if DOBOZ_SAFE
			FindMatches(null, 0);
#else
			FindMatches(null);
#endif
		}

		// Finds match candidates at the current buffer position and slides the matching window to the next character
		// Call findMatches/update with increasing positions
		// The match candidates are stored in the supplied array, ordered by their length (ascending)
		// The return value is the number of match candidates in the array
#if DOBOZ_SAFE
		public int FindMatches(DobozDecoder.Match[] matchCandidates, int index)
#else
		public int FindMatches(DobozDecoder.Match* matchCandidates)
#endif
		{
			Debug.Assert(_hashTable != null, "No buffer is set.");

			var buffer = _data;

#if DOBOZ_SAFE
			var ht = _hashTable;
			var ct = _children;
#else
			fixed (int* ht = _hashTable)
			fixed (int* ct = _children)
#endif
			{
				// Check whether we can find matches at this position
				if (Position >= _matchableBufferLength)
				{
					// Slide the matching window with one character
					++Position;

					return 0;
				}

				// Compute the maximum match length
				var maxMatchLength = Math.Min(_bufferLength - TAIL_LENGTH - Position, MAX_MATCH_LENGTH);

				// Compute the position relative to the beginning of bufferBase_
				// All other positions (including the ones stored in the hash table and the binary trees) are relative too
				// From now on, we can safely ignore this position technique
				var position = ComputeRelativePosition();

				// Compute the minimum match position
				var minMatchPosition = (position < DICTIONARY_SIZE) ? 0 : (position - DICTIONARY_SIZE + 1);

				// Compute the hash value for the current string
				var hashValue = (int)(Hash(buffer, _base + position) % HASH_TABLE_SIZE);

				// Get the position of the first match from the hash table
				var matchPosition = ht[hashValue];

				// Set the current string as the root of the binary tree corresponding to the hash table entry
				ht[hashValue] = position;

				// Compute the current cyclic position in the dictionary
				var cyclicInputPosition = position % DICTIONARY_SIZE;

				// Initialize the references to the leaves of the new root's left and right subtrees
				var leftSubtreeLeaf = cyclicInputPosition * 2;
				var rightSubtreeLeaf = cyclicInputPosition * 2 + 1;

				// Initialize the match lenghts of the lower and upper bounds of the current string (lowMatch < match < highMatch)
				// We use these to avoid unneccesary character comparisons at the beginnings of the strings
				var lowMatchLength = 0;
				var highMatchLength = 0;

				// Initialize the longest match length
				var longestMatchLength = 0;

				// Find matches
				// We look for the current string in the binary search tree and we rebuild the tree at the same time
				// The deeper a node is in the tree, the lower is its position, so the root is the string with the highest position (lowest offset)

				// We count the number of match attempts, and exit if it has reached a certain threshold
				var matchCount = 0;

				// Match candidates are matches which are longer than any previously encountered ones
				var matchCandidateCount = 0;

				while (true)
				{
					// Check whether the current match position is valid
					if (matchPosition < minMatchPosition || matchCount == MAX_MATCH_CANDIDATE_COUNT)
					{
						// We have checked all valid matches, so finish the new tree and exit
						ct[leftSubtreeLeaf] = INVALID_POSITION;
						ct[rightSubtreeLeaf] = INVALID_POSITION;
						break;
					}

					++matchCount;

					// Compute the cyclic position of the current match in the dictionary
					var cyclicMatchPosition = matchPosition % DICTIONARY_SIZE;

					// Use the match lengths of the low and high bounds to determine the number of characters that surely match
					var matchLength = Math.Min(lowMatchLength, highMatchLength);

					// Determine the match length
					while (
						matchLength < maxMatchLength &&
						buffer[_base + position + matchLength] == buffer[_base + matchPosition + matchLength])
					{
						++matchLength;
					}

					// Check whether this match is the longest so far
					var matchOffset = position - matchPosition;

					if (matchLength > longestMatchLength && matchLength >= MIN_MATCH_LENGTH)
					{
						longestMatchLength = matchLength;

						// Add the current best match to the list of good match candidates
						if (matchCandidates != null)
						{
							matchCandidates[matchCandidateCount].length = matchLength;
							matchCandidates[matchCandidateCount].offset = matchOffset;
							++matchCandidateCount;
						}

						// If the match length is the maximum allowed value, the current string is already inserted into the tree: the current node
						if (matchLength == maxMatchLength)
						{
							// Since the current string is also the root of the tree, delete the current node
							ct[leftSubtreeLeaf] = ct[cyclicMatchPosition * 2];
							ct[rightSubtreeLeaf] = ct[cyclicMatchPosition * 2 + 1];
							break;
						}
					}

					// Compare the two strings
					if (buffer[_base + position + matchLength] < buffer[_base + matchPosition + matchLength])
					{
						// Insert the matched string into the right subtree
						ct[rightSubtreeLeaf] = matchPosition;

						// Go left
						rightSubtreeLeaf = cyclicMatchPosition * 2;
						matchPosition = ct[rightSubtreeLeaf];

						// Update the match length of the high bound
						highMatchLength = matchLength;
					}
					else
					{
						// Insert the matched string into the left subtree
						ct[leftSubtreeLeaf] = matchPosition;

						// Go right
						leftSubtreeLeaf = cyclicMatchPosition * 2 + 1;
						matchPosition = ct[leftSubtreeLeaf];

						// Update the match length of the low bound
						lowMatchLength = matchLength;
					}
				}

				// Slide the matching window with one character
				++Position;

				return matchCandidateCount;
			}
		}

		#endregion

		#region private implementation

		// Increments the match window position with one character
		private int ComputeRelativePosition()
		{
#if DOBOZ_SAFE
			var ht = _hashTable;
			var ct = _children;
#else
			fixed (int* ht = _hashTable)
			fixed (int* ct = _children)
#endif
			{
				var position = Position - _base;

				// Check whether the current position has reached the rebase threshold
				if (position == REBASE_THRESHOLD)
				{
					_base += REBASE_DELTA;
					position -= REBASE_DELTA;

					// Rebase the hash entries
					for (var i = 0; i < HASH_TABLE_SIZE; ++i)
					{
						ht[i] = (ht[i] >= REBASE_DELTA) ? (ht[i] - REBASE_DELTA) : INVALID_POSITION;
					}

					// Rebase the binary tree nodes
					for (var i = 0; i < CHILD_COUNT; ++i)
					{
						ct[i] = (ct[i] >= REBASE_DELTA) ? (ct[i] - REBASE_DELTA) : INVALID_POSITION;
					}
				}

				return position;
			}
		}

#if DOBOZ_SAFE
		private static uint Hash(byte[] data, int index)
		{
#else
		private static uint Hash(byte* data, int index)
		{
#endif
			// FNV-1a hash
			const uint prime = 16777619;
			var result = 2166136261;

			result = (result ^ data[index + 0]) * prime;
			result = (result ^ data[index + 1]) * prime;
			result = (result ^ data[index + 2]) * prime;

			return result;
		}

		#endregion
	}
}

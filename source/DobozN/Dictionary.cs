using System.Diagnostics;

namespace DobozN
{
	internal partial class Dictionary
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
	}
}

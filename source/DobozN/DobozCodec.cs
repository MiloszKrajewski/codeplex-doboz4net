namespace DobozN
{
	public partial class DobozCodec: DobozDecoder
	{
		internal const int MAX_MATCH_LENGTH = 255 + MIN_MATCH_LENGTH;
		internal const int MAX_MATCH_CANDIDATE_COUNT = 128;
		internal const int DICTIONARY_SIZE = 1 << 21; // 2 MB, must be a power of 2!
		private const int TRAILING_DUMMY_SIZE = WORD_SIZE; // safety trailing bytes which decrease the number of necessary buffer checks
	}
}

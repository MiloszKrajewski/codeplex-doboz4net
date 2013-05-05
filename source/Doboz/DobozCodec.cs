namespace Doboz
{
	public class DobozCodec
	{
		public int Encode(
			byte[] input, int inputOffset, int inputLength,
			byte[] output, int outputOffset, int outputLength)
		{
			return DobozMM.DobozCodec.Encode(
				input, inputOffset, inputLength, 
				output, outputOffset, outputLength);
		}

		public byte[] Encode(
			byte[] input, int inputOffset, int inputLength)
		{
			return DobozMM.DobozCodec.Encode(
				input, inputOffset, inputLength);
		}
	}
}

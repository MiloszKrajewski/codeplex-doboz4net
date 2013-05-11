namespace Doboz.Services
{
	internal class DobozServiceN: IDobozService
	{
		#region IDobozService Members

		public string CodecName { get { return "Unsafe"; } }

		public int MaximumOutputLength(int size)
		{
			return DobozN.DobozCodec.MaximumOutputLength(size);
		}

		public int UncompressedLength(byte[] buffer, int bufferOffset, int bufferLength)
		{
			return DobozN.DobozCodec.UncompressedLength(buffer, bufferOffset, bufferLength);
		}

		public int Decode(byte[] input, int inputOffset, int inputLength, byte[] output, int outputOffset, int outputLength)
		{
			return DobozN.DobozCodec.Decode(input, inputOffset, inputLength, output, outputOffset, outputLength);
		}

		public int Encode(byte[] input, int inputOffset, int inputLength, byte[] output, int outputOffset, int outputLength)
		{
			return DobozN.DobozCodec.Encode(input, inputOffset, inputLength, output, outputOffset, outputLength);
		}

		public byte[] Decode(byte[] input, int inputOffset, int inputLength)
		{
			return DobozN.DobozCodec.Decode(input, inputOffset, inputLength);
		}

		public byte[] Encode(byte[] input, int inputOffset, int inputLength)
		{
			return DobozN.DobozCodec.Encode(input, inputOffset, inputLength);
		}

		#endregion
	}
}

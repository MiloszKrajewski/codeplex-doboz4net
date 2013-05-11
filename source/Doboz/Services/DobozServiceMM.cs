namespace Doboz.Services
{
	// ReSharper disable InconsistentNaming

	internal class DobozServiceMM: IDobozService
	{
		#region IDobozService Members

		public string CodecName { get { return "MixedMode"; } }

		public int MaximumOutputLength(int size)
		{
			return DobozMM.DobozCodec.MaximumOutputLength(size);
		}

		public int UncompressedLength(byte[] buffer, int bufferOffset, int bufferLength)
		{
			return DobozMM.DobozCodec.UncompressedLength(buffer, bufferOffset, bufferLength);
		}

		public int Decode(byte[] input, int inputOffset, int inputLength, byte[] output, int outputOffset, int outputLength)
		{
			return DobozMM.DobozCodec.Decode(input, inputOffset, inputLength, output, outputOffset, outputLength);
		}

		public int Encode(byte[] input, int inputOffset, int inputLength, byte[] output, int outputOffset, int outputLength)
		{
			return DobozMM.DobozCodec.Encode(input, inputOffset, inputLength, output, outputOffset, outputLength);
		}

		public byte[] Decode(byte[] input, int inputOffset, int inputLength)
		{
			return DobozMM.DobozCodec.Decode(input, inputOffset, inputLength);
		}

		public byte[] Encode(byte[] input, int inputOffset, int inputLength)
		{
			return DobozMM.DobozCodec.Encode(input, inputOffset, inputLength);
		}

		#endregion
	}

	// ReSharper restore InconsistentNaming
}
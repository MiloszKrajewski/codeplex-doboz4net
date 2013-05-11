namespace Doboz.Services
{
	internal class DobozServiceS: IDobozService
	{
		#region IDobozService Members

		public string CodecName { get { return "Safe"; } }

		public int MaximumOutputLength(int size)
		{
			return DobozS.DobozCodec.MaximumOutputLength(size);
		}

		public int UncompressedLength(byte[] buffer, int bufferOffset, int bufferLength)
		{
			return DobozS.DobozCodec.UncompressedLength(buffer, bufferOffset, bufferLength);
		}

		public int Decode(byte[] input, int inputOffset, int inputLength, byte[] output, int outputOffset, int outputLength)
		{
			return DobozS.DobozCodec.Decode(input, inputOffset, inputLength, output, outputOffset, outputLength);
		}

		public int Encode(byte[] input, int inputOffset, int inputLength, byte[] output, int outputOffset, int outputLength)
		{
			return DobozS.DobozCodec.Encode(input, inputOffset, inputLength, output, outputOffset, outputLength);
		}

		public byte[] Decode(byte[] input, int inputOffset, int inputLength)
		{
			return DobozS.DobozCodec.Decode(input, inputOffset, inputLength);
		}

		public byte[] Encode(byte[] input, int inputOffset, int inputLength)
		{
			return DobozS.DobozCodec.Encode(input, inputOffset, inputLength);
		}

		#endregion
	}
}

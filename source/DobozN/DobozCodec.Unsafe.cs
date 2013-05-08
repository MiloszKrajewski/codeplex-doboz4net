using System.Diagnostics;
using System;

namespace DobozN
{
	public partial class DobozCodec
	{
		public unsafe static int Decode(
			byte[] input, int inputOffset, int inputLength,
			byte[] output, int outputOffset, int outputLength)
		{
			/*
			CheckArguments(
				input, inputOffset, inputLength,
				output, outputOffset, outputLength);
			*/

			fixed (byte* src = &input[inputOffset])
			{
				CompressionInfo info = new CompressionInfo();
				if (GetCompressionInfo(src, inputLength, ref info) != Result.RESULT_OK)
					throw new ArgumentException("Corrupted input data");

				if (outputLength < (int)info.uncompressedSize)
					throw new ArgumentException("Output buffer too small");

				outputLength = (int)info.uncompressedSize;

				fixed (byte* dst = &output[outputOffset])
				{

					if (decompress(src, inputLength, dst, outputLength) != Result.RESULT_OK)
						throw new ArgumentException("Corrupted data or out buffer is too small");

					return outputLength;
				}
			}
		}

		public unsafe static byte[] Decode(
			byte[] input, int inputOffset, int inputLength)
		{
			/*
			CheckArguments(
				input, inputOffset, inputLength);
			*/

			fixed (byte* src = &input[inputOffset])
			{

				CompressionInfo info = new CompressionInfo();
				if (GetCompressionInfo(src, inputLength, ref info) != Result.RESULT_OK)
					throw new ArgumentException("Corrupted input data");

				int outputLength = (int)info.uncompressedSize;
				byte[] output = new byte[outputLength];

				fixed (byte* dst = &output[0])
				{

					if (decompress(src, inputLength, dst, outputLength) != Result.RESULT_OK)
						throw new ArgumentException("Corrupted data");

					return output;
				}
			}
		}
	}
}

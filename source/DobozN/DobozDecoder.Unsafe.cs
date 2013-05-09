using System;

namespace DobozN
{
	public partial class DobozDecoder
	{
		public static unsafe int Decode(
			byte[] input, int inputOffset, int inputLength,
			byte[] output, int outputOffset, int outputLength)
		{
			CheckArguments(
				input, inputOffset, ref inputLength,
				output, outputOffset, ref outputLength);

			fixed (byte* src = &input[inputOffset])
			{
				var info = new CompressionInfo();
				if (GetCompressionInfo(src, inputLength, ref info) != Result.RESULT_OK)
					throw new ArgumentException("Corrupted input data");

				if (outputLength < info.uncompressedSize)
					throw new ArgumentException("Output buffer too small");

				outputLength = info.uncompressedSize;

				fixed (byte* dst = &output[outputOffset])
				{
					if (Decompress(src, inputLength, dst, outputLength) != Result.RESULT_OK)
						throw new ArgumentException("Corrupted data or out buffer is too small");

					return outputLength;
				}
			}
		}

		public static unsafe byte[] Decode(
			byte[] input, int inputOffset, int inputLength)
		{
			CheckArguments(
				input, inputOffset, ref inputLength);

			fixed (byte* src = &input[inputOffset])
			{
				var info = new CompressionInfo();
				if (GetCompressionInfo(src, inputLength, ref info) != Result.RESULT_OK)
					throw new ArgumentException("Corrupted input data");

				var outputLength = info.uncompressedSize;
				var output = new byte[outputLength];

				fixed (byte* dst = &output[0])
				{
					if (Decompress(src, inputLength, dst, outputLength) != Result.RESULT_OK)
						throw new ArgumentException("Corrupted data");

					return output;
				}
			}
		}
	}
}

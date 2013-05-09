using System;

// ReSharper disable InconsistentNaming

namespace DobozN
{
	public partial class DobozCodec
	{
		public static unsafe int Encode(
			byte[] input, int inputOffset, int inputLength,
			byte[] output, int outputOffset, int outputLength)
		{
			CheckArguments(
				input, inputOffset, ref inputLength,
				output, outputOffset, ref outputLength);

			fixed (byte* input_p = &input[inputOffset])
			fixed (byte* output_p = &output[outputOffset])
			{
				int length;

				if (Compress(input_p, inputLength, output_p, outputLength, out length) == Result.RESULT_OK)
					return length;

				// buffer is too small
				return -1;
			}
		}

		public static unsafe byte[] Encode(
			byte[] input, int inputOffset, int inputLength)
		{
			CheckArguments(input, inputOffset, ref inputLength);

			var maxOutputSize = MaximumOutputLength(inputLength);

			var bufferLength = maxOutputSize;
			var buffer = new byte[bufferLength];

			fixed (byte* input_p = &input[inputOffset])
			fixed (byte* output_p = buffer)
			{
				int outputLength;

				if (Compress(input_p, inputLength, output_p, bufferLength, out outputLength) != Result.RESULT_OK)
					throw new InvalidOperationException("Compressed data has been corrupted");

				if (outputLength == bufferLength)
					return buffer;

				var output = new byte[outputLength];
				var src = output_p;
				fixed (byte* dst = output)
				{
					BlockCopy(src, dst, outputLength);
				}

				return output;
			}
		}
	}
}

// ReSharper restore InconsistentNaming

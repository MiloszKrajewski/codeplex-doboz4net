namespace Doboz
{
	/// <summary>
	/// Interface to Doboz implemenations.
	/// </summary>
	public interface IDobozService
	{
		/// <summary>Gets the name of the codec.</summary>
		/// <value>The name of the codec.</value>
		string CodecName { get; }

		/// <summary>Gets the maximum length of the output.</summary>
		/// <param name="size">The uncompressed length.</param>
		/// <returns>Maximum compressed length.</returns>
		int MaximumOutputLength(int size);

		/// <summary>Gets the uncompressed length.</summary>
		/// <param name="buffer">The buffer.</param>
		/// <param name="bufferOffset">The buffer offset.</param>
		/// <param name="bufferLength">Length of the buffer.</param>
		/// <returns>Length of uncompressed data.</returns>
		int UncompressedLength(byte[] buffer, int bufferOffset, int bufferLength);

		/// <summary>Decodes the specified input.</summary>
		/// <param name="input">The input.</param>
		/// <param name="inputOffset">The input offset.</param>
		/// <param name="inputLength">Length of the input.</param>
		/// <param name="output">The output.</param>
		/// <param name="outputOffset">The output offset.</param>
		/// <param name="outputLength">Length of the output.</param>
		/// <returns>Number of decoded bytes.</returns>
		int Decode(
			byte[] input, int inputOffset, int inputLength,
			byte[] output, int outputOffset, int outputLength);

		/// <summary>Decodes the specified input.</summary>
		/// <param name="input">The input.</param>
		/// <param name="inputOffset">The input offset.</param>
		/// <param name="inputLength">Length of the input.</param>
		/// <returns>Decoded buffer.</returns>
		byte[] Decode(
			byte[] input, int inputOffset, int inputLength);

		/// <summary>Encodes the specified input.</summary>
		/// <param name="input">The input.</param>
		/// <param name="inputOffset">The input offset.</param>
		/// <param name="inputLength">Length of the input.</param>
		/// <param name="output">The output.</param>
		/// <param name="outputOffset">The output offset.</param>
		/// <param name="outputLength">Length of the output.</param>
		/// <returns>Number of bytes in compressed buffer. Negative value means thet output buffer was too small.</returns>
		int Encode(
			byte[] input, int inputOffset, int inputLength,
			byte[] output, int outputOffset, int outputLength);

		/// <summary>Encodes the specified input.</summary>
		/// <param name="input">The input.</param>
		/// <param name="inputOffset">The input offset.</param>
		/// <param name="inputLength">Length of the input.</param>
		/// <returns>Encoded buffer.</returns>
		byte[] Encode(
			byte[] input, int inputOffset, int inputLength);
	}
}

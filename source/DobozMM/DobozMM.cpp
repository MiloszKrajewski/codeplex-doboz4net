// This is the main DLL file.

#include "stdafx.h"
#include "memory.h"
#include "DobozMM.h"
#include "Compressor.h"
#include "Decompressor.h"

namespace DobozMM 
{
	void DobozCodec::CheckArguments(
		array<Byte>^ input, int inputOffset, int% inputLength)
	{
		if (inputLength < 0) inputLength = input->Length - inputOffset;

		if (input == nullptr) throw gcnew ArgumentNullException("input");
		if (inputOffset < 0 || inputOffset + inputLength > input->Length)
			throw gcnew ArgumentException("inputOffset and inputLength are invalid for given input");
	}

	void DobozCodec::CheckArguments(
		array<Byte>^ input, int inputOffset, int% inputLength,
		array<Byte>^ output, int outputOffset, int% outputLength)
	{
		if (inputLength < 0) inputLength = input->Length - inputOffset;
		if (inputLength == 0)
		{
			outputLength = 0;
			return;
		}

		if (input == nullptr) throw gcnew ArgumentNullException("input");
		if (inputOffset < 0 || inputOffset + inputLength > input->Length)
			throw gcnew ArgumentException("inputOffset and inputLength are invalid for given input");

		if (outputLength < 0) outputLength = output->Length - outputOffset;
		if (output == nullptr) throw gcnew ArgumentNullException("output");
		if (outputOffset < 0 || outputOffset + outputLength > output->Length)
			throw gcnew ArgumentException("outputOffset and outputLength are invalid for given output");
	}

	int DobozCodec::MaximumOutputLength(int inputLength)
	{
		size_t maxOutputSize = (size_t)doboz::Compressor::getMaxCompressedSize(inputLength);
		if (maxOutputSize > INT_MAX)
			throw gcnew ArgumentException("Chunk is too big to be compressed");
		return (int)maxOutputSize;
	}

	int DobozCodec::Encode(
		array<Byte>^ input, int inputOffset, int inputLength,
		array<Byte>^ output, int outputOffset, int outputLength)
	{
		CheckArguments(
			input, inputOffset, inputLength, 
			output, outputOffset, outputLength);

		doboz::Compressor compressor;
		pin_ptr<Byte> input_p = &input[inputOffset];
		pin_ptr<Byte> output_p = &output[outputOffset];
		size_t length; 

		if (compressor.compress(input_p, inputLength, output_p, outputLength, length) == doboz::RESULT_OK)
			return (int)length;

		// buffer is too small
		return -1;
	}

	array<Byte>^ DobozCodec::Encode(
		array<Byte>^ input, int inputOffset, int inputLength)
	{
		CheckArguments(input, inputOffset, inputLength);

		size_t maxOutputSize = (size_t)doboz::Compressor::getMaxCompressedSize(inputLength);
		if (maxOutputSize > INT_MAX)
			throw gcnew ArgumentException("Chunk is too big to be compressed");

		int bufferLength = (int)maxOutputSize;
		array<Byte>^ buffer = gcnew array<Byte>(bufferLength);
		int outputLength;

		doboz::Compressor compressor;
		pin_ptr<Byte> input_p = &input[inputOffset];
		pin_ptr<Byte> output_p = &buffer[0];
		size_t length; 

		if (compressor.compress(input_p, inputLength, output_p, bufferLength, length) != doboz::RESULT_OK)
			throw gcnew InvalidOperationException("Compressed data has been corrupted");

		outputLength = (int)length;

		if (outputLength == bufferLength) 
			return buffer;

		array<Byte>^ output = gcnew array<Byte>(outputLength);
		pin_ptr<Byte> src = &buffer[0];
		pin_ptr<Byte> dst = &output[0];
		memcpy(dst, src, outputLength);
		return output;
	}

	int DobozCodec::Decode(
		array<Byte>^ input, int inputOffset, int inputLength,
		array<Byte>^ output, int outputOffset, int outputLength)
	{
		CheckArguments(
			input, inputOffset, inputLength,
			output, outputOffset, outputLength);

		doboz::Decompressor decompressor;
		pin_ptr<Byte> src = &input[inputOffset];

		doboz::CompressionInfo info;
		if (decompressor.getCompressionInfo(src, inputLength, info) != doboz::RESULT_OK)
			throw gcnew ArgumentException("Corrupted input data");

		if (outputLength < info.uncompressedSize)
			throw gcnew ArgumentException("Output buffer too small");

		outputLength = (int)info.uncompressedSize;

		pin_ptr<Byte> dst = &output[outputOffset];

		if (decompressor.decompress(src, inputLength, dst, outputLength) != doboz::RESULT_OK)
			throw gcnew ArgumentException("Corrupted data or out buffer is too small");

		return outputLength;
	}

	array<Byte>^ DobozCodec::Decode(
		array<Byte>^ input, int inputOffset, int inputLength)
	{
		CheckArguments(
			input, inputOffset, inputLength);

		doboz::Decompressor decompressor;
		pin_ptr<Byte> src = &input[inputOffset];

		doboz::CompressionInfo info;
		if (decompressor.getCompressionInfo(src, inputLength, info) != doboz::RESULT_OK)
			throw gcnew ArgumentException("Corrupted input data");

		int outputLength = (int)info.uncompressedSize;
		array<Byte>^ output = gcnew array<Byte>(outputLength);

		pin_ptr<Byte> dst = &output[0];

		if (decompressor.decompress(src, inputLength, dst, outputLength) != doboz::RESULT_OK)
			throw gcnew ArgumentException("Corrupted data");

		return output;
	}
} // namespace DobozMM
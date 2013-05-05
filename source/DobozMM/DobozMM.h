// DobozMM.h

#pragma once

using namespace System;

namespace DobozMM 
{
	public ref class DobozCodec
	{
	private:
		static void CheckArguments(
			array<Byte>^ input, int inputOffset, int% inputLength);

		static void CheckArguments(
			array<Byte>^ input, int inputOffset, int% inputLength,
			array<Byte>^ output, int outputOffset, int% outputLength);

	public:
		static int MaximumOutputLength(int inputSize);

		static int Encode(
			array<Byte>^ input, int inputOffset, int inputLength,
			array<Byte>^ output, int outputOffset, int outputLength);
		
		static array<Byte>^ Encode(
			array<Byte>^ input, int inputOffset, int inputLength);

		static int Decode(
			array<Byte>^ input, int inputOffset, int inputLength,
			array<Byte>^ output, int outputOffset, int outputLength);

		static array<Byte>^ Decode(
			array<Byte>^ input, int inputOffset, int inputLength);
	};
}

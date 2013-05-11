using System;
using System.Runtime.CompilerServices;
using System.Text;
using Doboz.Services;

namespace Doboz
{
	/// <summary>Doboz codec.</summary>
	public class DobozCodec
	{
		#region fields

		/// <summary>The service</summary>
		private static readonly IDobozService Service;

		/// <summary>The mixed mode service.</summary>
		private static IDobozService _serviceMM;

		/// <summary>The unsafe service.</summary>
		private static IDobozService _serviceN;

		/// <summary>The safe service.</summary>
		private static IDobozService _serviceS;

		#endregion

		#region initialization

		/// <summary>Initializes the <see cref="DobozCodec"/> class.</summary>
		/// <exception cref="System.NotSupportedException">No Doboz compression service found</exception>
		static DobozCodec()
		{
			// NOTE: this method exploits the fact that assemblies are loaded first time they
			// are needed so we can safely try load and handle if not loaded
			// I may change in future versions of .NET

			Try(InitializeDobozMM);
			Try(InitializeDobozN);
			Try(InitializeDobozS);

			Service = _serviceMM ?? _serviceN ?? _serviceS;

			if (Service == null)
			{
				throw new NotSupportedException("No Doboz compression service found");
			}
		}

		/// <summary>Tries to execute specified action. Ignores exception if it failed.</summary>
		/// <param name="method">The method.</param>
		private static void Try(Action method)
		{
			try
			{
				method();
			}
			// ReSharper disable EmptyGeneralCatchClause
			catch
			{
				// ignore exception
			}
			// ReSharper restore EmptyGeneralCatchClause
		}

		/// <summary>Tries to create a specified <seealso cref="IDobozService"/> and tests it.</summary>
		/// <typeparam name="T">Concrete <seealso cref="IDobozService"/> type.</typeparam>
		/// <returns>A service if suceeded or <c>null</c> if it failed.</returns>
		private static IDobozService Try<T>()
			where T: IDobozService, new()
		{
			try
			{
				return AutoTest(new T());
			}
			catch
			{
				return null;
			}
		}

		/// <summary>Perofrms the quick auto-test on given compression service.</summary>
		/// <param name="service">The service.</param>
		/// <returns>A service or <c>null</c> if it failed.</returns>
		private static IDobozService AutoTest(IDobozService service)
		{
			const string loremIpsum =
				"Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut " +
				"labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco " +
				"laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in " +
				"voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat " +
				"non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.";

			// generate some well-known array of bytes
			const string inputText = loremIpsum + loremIpsum + loremIpsum + loremIpsum + loremIpsum;
			var original = Encoding.UTF8.GetBytes(inputText);

			// compress it
			var encoded = new byte[service.MaximumOutputLength(original.Length)];
			var encodedLength = service.Encode(original, 0, original.Length, encoded, 0, encoded.Length);
			if (encodedLength < 0) return null;

			var extractedLength = service.UncompressedLength(encoded, 0, encodedLength);
			if (extractedLength != original.Length) return null;

			// decompress it (knowing original length)
			var decoded = new byte[original.Length];
			var decodedLength1 = service.Decode(encoded, 0, encodedLength, decoded, 0, decoded.Length);
			if (decodedLength1 != original.Length) return null;
			var outputText1 = Encoding.UTF8.GetString(decoded);
			if (outputText1 != inputText) return null;

			return service;
		}

		// ReSharper disable InconsistentNaming

		/// <summary>Initializes codecs from LZ4mm.</summary>
		[MethodImpl(MethodImplOptions.NoInlining)]
		private static void InitializeDobozMM()
		{
			_serviceMM = Try<DobozServiceMM>();
		}


		/// <summary>Initializes codecs from LZ4n.</summary>
		[MethodImpl(MethodImplOptions.NoInlining)]
		private static void InitializeDobozN()
		{
			_serviceN = Try<DobozServiceN>();
		}

		/// <summary>Initializes codecs from LZ4s.</summary>
		[MethodImpl(MethodImplOptions.NoInlining)]
		private static void InitializeDobozS()
		{
			_serviceS = Try<DobozServiceS>();
		}

		// ReSharper restore InconsistentNaming

		#endregion

		/// <summary>Gets the name of the codec.</summary>
		/// <value>The name of the codec.</value>
		public static string CodecName { get { return Service.CodecName; } }

		/// <summary>Gets the maximum length of the output.</summary>
		/// <param name="size">The uncompressed length.</param>
		/// <returns>Maximum compressed length.</returns>
		public int MaximumOutputLength(int size)
		{
			return Service.MaximumOutputLength(size);
		}

		/// <summary>Gets the uncompressed length.</summary>
		/// <param name="buffer">The buffer.</param>
		/// <param name="bufferOffset">The buffer offset.</param>
		/// <param name="bufferLength">Length of the buffer.</param>
		/// <returns>Length of uncompressed data.</returns>
		public int UncompressedLength(byte[] buffer, int bufferOffset, int bufferLength)
		{
			return Service.UncompressedLength(buffer, bufferOffset, bufferLength);
		}

		/// <summary>Decodes the specified input.</summary>
		/// <param name="input">The input.</param>
		/// <param name="inputOffset">The input offset.</param>
		/// <param name="inputLength">Length of the input.</param>
		/// <param name="output">The output.</param>
		/// <param name="outputOffset">The output offset.</param>
		/// <param name="outputLength">Length of the output.</param>
		/// <returns>Number of decoded bytes.</returns>
		public int Decode(byte[] input, int inputOffset, int inputLength, byte[] output, int outputOffset, int outputLength)
		{
			return Service.Decode(input, inputOffset, inputLength, output, outputOffset, outputLength);
		}

		/// <summary>Encodes the specified input.</summary>
		/// <param name="input">The input.</param>
		/// <param name="inputOffset">The input offset.</param>
		/// <param name="inputLength">Length of the input.</param>
		/// <param name="output">The output.</param>
		/// <param name="outputOffset">The output offset.</param>
		/// <param name="outputLength">Length of the output.</param>
		/// <returns>Number of bytes in compressed buffer. Negative value means thet output buffer was too small.</returns>
		public int Encode(byte[] input, int inputOffset, int inputLength, byte[] output, int outputOffset, int outputLength)
		{
			return Service.Encode(input, inputOffset, inputLength, output, outputOffset, outputLength);
		}

		/// <summary>Decodes the specified input.</summary>
		/// <param name="input">The input.</param>
		/// <param name="inputOffset">The input offset.</param>
		/// <param name="inputLength">Length of the input.</param>
		/// <returns>Decoded buffer.</returns>
		public byte[] Decode(byte[] input, int inputOffset, int inputLength)
		{
			return Service.Decode(input, inputOffset, inputLength);
		}

		/// <summary>Encodes the specified input.</summary>
		/// <param name="input">The input.</param>
		/// <param name="inputOffset">The input offset.</param>
		/// <param name="inputLength">Length of the input.</param>
		/// <returns>Encoded buffer.</returns>
		public byte[] Encode(byte[] input, int inputOffset, int inputLength)
		{
			return Service.Encode(input, inputOffset, inputLength);
		}
	}
}

#region License

/*
 * Copyright (c) 2013, Milosz Krajewski
 * 
 * Microsoft Public License (Ms-PL)
 * This license governs use of the accompanying software. 
 * If you use the software, you accept this license. 
 * If you do not accept the license, do not use the software.
 * 
 * 1. Definitions
 * The terms "reproduce," "reproduction," "derivative works," and "distribution" have the same 
 * meaning here as under U.S. copyright law.
 * A "contribution" is the original software, or any additions or changes to the software.
 * A "contributor" is any person that distributes its contribution under this license.
 * "Licensed patents" are a contributor's patent claims that read directly on its contribution.
 * 
 * 2. Grant of Rights
 * (A) Copyright Grant- Subject to the terms of this license, including the license conditions 
 * and limitations in section 3, each contributor grants you a non-exclusive, worldwide, 
 * royalty-free copyright license to reproduce its contribution, prepare derivative works of 
 * its contribution, and distribute its contribution or any derivative works that you create.
 * (B) Patent Grant- Subject to the terms of this license, including the license conditions and 
 * limitations in section 3, each contributor grants you a non-exclusive, worldwide, 
 * royalty-free license under its licensed patents to make, have made, use, sell, offer for sale, 
 * import, and/or otherwise dispose of its contribution in the software or derivative works of 
 * the contribution in the software.
 * 
 * 3. Conditions and Limitations
 * (A) No Trademark License- This license does not grant you rights to use any contributors' name, 
 * logo, or trademarks.
 * (B) If you bring a patent claim against any contributor over patents that you claim are infringed 
 * by the software, your patent license from such contributor to the software ends automatically.
 * (C) If you distribute any portion of the software, you must retain all copyright, patent, trademark, 
 * and attribution notices that are present in the software.
 * (D) If you distribute any portion of the software in source code form, you may do so only under this 
 * license by including a complete copy of this license with your distribution. If you distribute 
 * any portion of the software in compiled or object code form, you may only do so under a license 
 * that complies with this license.
 * (E) The software is licensed "as-is." You bear the risk of using it. The contributors give no express
 * warranties, guarantees or conditions. You may have additional consumer rights under your local 
 * laws which this license cannot change. To the extent permitted under your local laws, the 
 * contributors exclude the implied warranties of merchantability, fitness for a particular 
 * purpose and non-infringement.
 */

#endregion

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
		public static int MaximumOutputLength(int size)
		{
			return Service.MaximumOutputLength(size);
		}

		/// <summary>Gets the uncompressed length.</summary>
		/// <param name="buffer">The buffer.</param>
		/// <param name="bufferOffset">The buffer offset.</param>
		/// <param name="bufferLength">Length of the buffer.</param>
		/// <returns>Length of uncompressed data.</returns>
		public static int UncompressedLength(byte[] buffer, int bufferOffset, int bufferLength)
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
		public static int Decode(byte[] input, int inputOffset, int inputLength, byte[] output, int outputOffset, int outputLength)
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
		public static int Encode(byte[] input, int inputOffset, int inputLength, byte[] output, int outputOffset, int outputLength)
		{
			return Service.Encode(input, inputOffset, inputLength, output, outputOffset, outputLength);
		}

		/// <summary>Decodes the specified input.</summary>
		/// <param name="input">The input.</param>
		/// <param name="inputOffset">The input offset.</param>
		/// <param name="inputLength">Length of the input.</param>
		/// <returns>Decoded buffer.</returns>
		public static byte[] Decode(byte[] input, int inputOffset, int inputLength)
		{
			return Service.Decode(input, inputOffset, inputLength);
		}

		/// <summary>Encodes the specified input.</summary>
		/// <param name="input">The input.</param>
		/// <param name="inputOffset">The input offset.</param>
		/// <param name="inputLength">Length of the input.</param>
		/// <returns>Encoded buffer.</returns>
		public static byte[] Encode(byte[] input, int inputOffset, int inputLength)
		{
			return Service.Encode(input, inputOffset, inputLength);
		}
	}
}

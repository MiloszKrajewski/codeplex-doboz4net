using System;
using System.Collections.Generic;
using NUnit.Framework;
using Doboz.Tests.Utilities;

// ReSharper disable InconsistentNaming

namespace Doboz.Tests
{
	[TestFixture]
	public class ConformanceTests
	{
		private const int MAXIMUM_LENGTH = 1 * 10 * 1024 * 1024; // 10MB

		private void TestConformance(TimedMethod[] compressors, TimedMethod[] decompressors)
		{
			var provider = new BlockDataProvider(Common.TEST_DATA_FOLDER);

			var r = new Random(0);

			Console.WriteLine("Architecture: {0}bit", IntPtr.Size * 8);

			var total = 0;
			const long limit = 1L * 1024 * 1024 * 1024;
			var last_pct = 0;

			while (total < limit)
			{
				var length = Common.RandomLength(r, MAXIMUM_LENGTH);
				var block = provider.GetBytes(length);
				TestData(block, compressors, decompressors);
				total += block.Length;
				var pct = (int)((double)total * 100 / limit);
				if (pct > last_pct)
				{
					Console.WriteLine("{0}%...", pct);
					last_pct = pct;
				}
			}

			/*

			The performance results from this test are completely unreliable
			Too much garbage collection and caching.
			So, no need to mislead anyone.

			Console.WriteLine("Compression:");
			foreach (var compressor in compressors)
			{
				Console.WriteLine("  {0}: {1:0.00}MB/s", compressor.Name, compressor.Speed);
			}

			Console.WriteLine("Decompression:");
			foreach (var decompressor in decompressors)
			{
				Console.WriteLine("  {0}: {1:0.00}MB/s", decompressor.Name, decompressor.Speed);
			}
			*/
		}

		[Test]
		public void TestCompressionConformance()
		{
			var compressors = new[] {
				new TimedMethod("MixedMode", (b, l) => DobozMM.DobozCodec.Encode(b, 0, l)),
				new TimedMethod("Unsafe", (b, l) => DobozN.DobozCodec.Encode(b, 0, l)),
				new TimedMethod("Safe", (b, l) => DobozMM.DobozCodec.Encode(b, 0, l)),
			};

			var decompressors = new[] {
				new TimedMethod("MixedMode", (b, l) => DobozMM.DobozCodec.Decode(b, 0, b.Length)),
				new TimedMethod("Unsafe", (b, l) => DobozN.DobozCodec.Decode(b, 0, b.Length)),
				new TimedMethod("Safe", (b, l) => DobozS.DobozCodec.Decode(b, 0, b.Length)),
			};

			TestConformance(compressors, decompressors);
		}

		private static void TestData(
			byte[] original, 
			IEnumerable<TimedMethod> compressors, 
			IEnumerable<TimedMethod> decompressors)
		{
			int length = original.Length;
			var compressed = new Dictionary<string, byte[]>();
			byte[] compressed0 = null;

			foreach (var compressor in compressors)
			{
				var buffer = compressor.Run(original, length);
				compressed[compressor.Name] = buffer;
				if (compressed0 == null)
				{
					compressed0 = buffer;
				}
				else if (compressor.Identical)
				{
					Common.AssertEqual(compressed0, buffer, compressor.Name);
				}
			}

			foreach (var decompressor in decompressors)
			{
				try
				{
					var temp = decompressor.Run(compressed[decompressor.Name], length);
					Common.AssertEqual(original, temp, decompressor.Name);
				}
				catch
				{
					Console.WriteLine("Failed in: {0}", decompressor.Name);
					throw;
				}
			}
		}
	}
}

// ReSharper restore InconsistentNaming

﻿using System;
using System.IO;
using System.IO.Compression;
using System.Linq;
using System.Threading;
using Doboz.Tests.Utilities;
using NUnit.Framework;

namespace Doboz.Tests
{
	[TestFixture]
	public class PerformanceTests
	{
		[Test]
		public void TestCompressionPerformance()
		{
			var compressors = new[] {
				new TimedMethod("MixedMode", (b, l) => DobozMM.DobozCodec.Encode(b, 0, l)),
				new TimedMethod("Unsafe", (b, l) => DobozN.DobozCodec.Encode(b, 0, l)),
				new TimedMethod("Safe", (b, l) => DobozS.DobozCodec.Encode(b, 0, l)),
				new TimedMethod("Deflate", DeflateEncode),
				new TimedMethod("LZ4n", (b, l) => LZ4n.LZ4Codec.Encode64HC(b, 0, l)), 
				new TimedMethod("LZ4s", (b, l) => LZ4s.LZ4Codec.Encode64HC(b, 0, l)), 
			};

			var decompressors = new[] {
				new TimedMethod("MixedMode", (b, l) => DobozMM.DobozCodec.Decode(b, 0, b.Length)),
				new TimedMethod("Unsafe", (b, l) => DobozN.DobozCodec.Decode(b, 0, b.Length)),
				new TimedMethod("Safe", (b, l) => DobozS.DobozCodec.Decode(b, 0, b.Length)),
				new TimedMethod("Deflate", DeflateDecode), 
				new TimedMethod("LZ4n", (b, l) => LZ4n.LZ4Codec.Decode64(b, 0, b.Length, l)), 
				new TimedMethod("LZ4s", (b, l) => LZ4s.LZ4Codec.Decode64(b, 0, b.Length, l)), 
			};

			TestPerformance(compressors, decompressors);
		}

		private byte[] DeflateEncode(byte[] b, int l)
		{
			using (var istream = new MemoryStream(b))
			using (var ostream = new MemoryStream())
			{
				using (var zstream = new DeflateStream(ostream, CompressionMode.Compress))
				{
					istream.CopyTo(zstream);
					zstream.Flush();
				}
				return ostream.ToArray();
			}
		}

		private byte[] DeflateDecode(byte[] b, int l)
		{
			using (var istream = new MemoryStream(b))
			using (var zstream = new DeflateStream(istream, CompressionMode.Decompress))
			{
				var result = new byte[l];
				zstream.Read(result, 0, l);
				return result;
			}
		}

		private static void TestPerformance(TimedMethod[] compressors, TimedMethod[] decompressors)
		{
			var names = compressors.Select(c => c.Name).ToArray();

			foreach (var name in names)
			{
				var compressor = compressors.First(c => c.Name == name);
				var decompressor = decompressors.First(d => d.Name == name);

				Console.WriteLine("---- {0} ----", name);

				Warmup(compressor, decompressor);

				var provider = new FileDataProvider(Common.TEST_DATA_FOLDER);
				long total = 0;
				const long limit = Common.TEST_DATA_BYTES;
				var last_pct = 0;

				while (total < limit)
				{
					var block = provider.GetBytes();
					TestSpeed(block, compressor, decompressor);
					total += block.Length;
					var pct = (int)((double)total*100/limit);
					if (pct > last_pct)
					{
						Console.WriteLine("{0}%...", pct);
						last_pct = pct;
					}
				}

				GC.Collect(GC.MaxGeneration, GCCollectionMode.Forced);
				Thread.Sleep(1000);
			}

			Console.WriteLine("---- Results ----");

			Console.WriteLine("Architecture: {0}bit", IntPtr.Size*8);
			Console.WriteLine("Compression:");
			foreach (var compressor in compressors)
			{
				Console.WriteLine("  {0}: {1:0.00}MB/s ({2:0.00}%)", compressor.Name, compressor.Speed, compressor.Ratio);
			}

			Console.WriteLine("Decompression:");
			foreach (var decompressor in decompressors)
			{
				Console.WriteLine("  {0}: {1:0.00}MB/s", decompressor.Name, decompressor.Speed);
			}
		}

		private static byte[] Copy(byte[] b, int l)
		{
			var result = new byte[l];
			Buffer.BlockCopy(b, 0, result, 0, l);
			return result;
		}

		private static void Warmup(TimedMethod compressor, TimedMethod decompressor)
		{
			const int length = 1 * 1024 * 1024;
			var data = new byte[length];
			var gen = new Random(0);
			gen.NextBytes(data);

			var compressed = compressor.Warmup(data, length);
			AssertEqual(data, decompressor.Warmup(compressed, length), compressor.Name);
		}

		private static void TestSpeed(byte[] original, TimedMethod compressor, TimedMethod decompressor)
		{
			int length = original.Length;
			byte[] compressed = compressor.Run(original, length);
			AssertEqual(original, decompressor.Run(compressed, length), compressor.Name);
		}

		private static void AssertEqual(byte[] expected, byte[] actual, string name)
		{
			Assert.AreEqual(expected.Length, actual.Length, string.Format("Buffers are different length ({0})", name));
			var length = expected.Length;

			for (int i = 0; i < length; i++)
			{
				if (expected[i] != actual[i]) Assert.Fail("Buffer differ @ {0} ({1})", i, name);
			}
		}
	}
}
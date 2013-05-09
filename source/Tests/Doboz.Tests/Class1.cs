using System;
using System.Text;
using NUnit.Framework;

namespace Doboz.Tests
{
	[TestFixture]
	public class Class1
	{
		public const string LoremIpsum =
			"Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut " +
			"labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris " +
			"nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate " +
			"velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, " +
			"sunt in culpa qui officia deserunt mollit anim id est laborum";

		public static byte[] Uncompressible(int size)
		{
			var result = new byte[size];
			new Random(0).NextBytes(result);
			return result;
		}

		public static byte[] Compressible(int size)
		{
			var result = new byte[size];
			var lorem = Encoding.UTF8.GetBytes(LoremIpsum);
			var count = size;
			var offset = 0;
			while (count > 0)
			{
				var chunk = Math.Min(count, lorem.Length);
				Buffer.BlockCopy(lorem, 0, result, offset, chunk);
				offset += chunk;
				count -= chunk;
			}
			return result;
		}

		[Test]
		public void Test1()
		{
			TestCompression(Compressible(0x10000));
			TestCompression(Uncompressible(0x10000));
		}

		private static void TestCompression(byte[] input)
		{
			var length = input.Length;
			var compressed = DobozN.DobozCodec.Encode(input, 0, length);
			var decompressed = DobozS.DobozCodec.Decode(compressed, 0, compressed.Length);
			Assert.AreEqual(length, decompressed.Length);
			for (var i = 0; i < length; i++) Assert.AreEqual(input[i], decompressed[i]);
		}
	}
}

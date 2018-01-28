# codeplex-doboz4net

Project Description
Doboz is asymmetric (high compression ratio, fast decompression) LZ-based compression algorithm.
Copyright
Originally Doboz was written by Attila Áfra. This library in a port (not wrapper) to .NET.
Download
You can find latest release on NuGet.
Why use such algorithm?
Like "Scope, Time, Cost" compression algorithms have their own triangle: "compression ratio, compression speed, decompression speed". If you want to achieve better results in one area you have to sacrifice something else.
Zip is "general purpose algorithms" which just tries to have average score in all aspects. But sometimes we need one aspect. 
Asymmetric compression is used when one of the operation is used more frequently. For example,
Fast compression speed sacrificing decompression speed - useful for backup software. You want to backup frequently, but you don't really care about decompression speed (you actually prefer to never use it!).
Fast compression and decompression sacrificing compression ratio - useful for transmission through "large pipe" (Snappy, LZ4).
Fast decompression sacrificing compression speed - useful for compress-once-decompress-frequently; example being: distributing data. You compress it is once on your build machine (so you don't really care about compression speed) but it will be decompressed many times (LZ4HC);

Doboz falls into third category.
Comparision
On average, Doboz has the same compression as zip but is 3-6 times faster on decompression (and, to be honest, 4-5 times slower on compression).
Tests were done on Intel Core i5 3570K @ 4.2GHz (but only single core was used):
Algorithms	Ratio	Compression speed	Decompression speed
Deflate (.NET 4.51)	50%	30MB/s	200MB/s
Doboz (Unsafe)	50%	3.3MB/s	1200MB/s
Doboz (Safe)	50%	3.1MB/s	650MB/s
LZ4HC (Unsafe)	53.5%	28MB/s	2000MB/s
LZ4HC (safe)	53.5%	18MB/s	700MB/s
1. NOTE: .NET 4.5 Deflate is finally properly implemented. It has the same compression ratio as zlib.

As you can see LZ4HC is faster but Doboz has better compression ratio. Doboz was also easier to port to .NET, so its safe implementation is almost as fast as LZ4's.

Anyway, I'm not overly impressed, I just ported it for myself and now I'm sharing it with you.


Last edited May 12, 2013 at 5:54 PM by Krashan, version 15

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using System.Threading;
using System.Globalization;

namespace ParseResults
{
	using kvp = KeyValuePair<int, double>;
	using dictI2D = Dictionary<int, double>;

	static class Program
	{
		const string strFolder = @"C:\Z\Fun\LookupTables\RawResults";
		const string strOutput = @"C:\Z\Fun\LookupTables\RawResults\Parsed.tsv";
		const string strOutputRel = @"C:\Z\Fun\LookupTables\RawResults\Relative.tsv";

		/// <summary>Test ID</summary>
		enum eTest : byte
		{
			SinCos = 1,
			BitCount = 2,
		}

		/// <summary>Pack test + algorithm into an int</summary>
		static int key( eTest et, int algo )
		{
			return ( (int)et << 8 ) | algo;
		}

		/// <summary>Unpack test from integer key</summary>
		static eTest test( this int k )
		{
			return (eTest)( k >> 8 );
		}

		/// <summary>Unpack algorithm from integer key</summary>
		static int algo( this int k )
		{
			return k & 255;
		}

		/// <summary>Algorithms names for SinCos test</summary>
		static readonly Dictionary<int, string> s_algoSinCos = new Dictionary<int, string>()
		{
			{ 1, "StdLib" },
			{ 2, "Lookup" },
			{ 3, "GTE" },
			{ 4, "DX" },
			{ 5, "DX_Est" },
		};

		/// <summary>Algorithms names for BitCount test</summary>
		static readonly Dictionary<int, string> s_algoCnt = new Dictionary<int, string>()
		{
			{ 1, "Lookup" },
			{ 2, "SSE2" },
			{ 3, "SSSE3" },
			{ 4, "XOP" },
			{ 5, "POPCNT" },
		};

		/// <summary>Read all lines from the file</summary>
		static IEnumerable<string> getLines( string fileName )
		{
			using( StreamReader sr = new StreamReader( fileName, Encoding.ASCII ) )
			{
				while( !sr.EndOfStream )
					yield return sr.ReadLine();
			}
		}

		/// <summary>Case-insensitive string compare</summary>
		static bool isEqual( this string s, string s2 )
		{
			return s.Equals( s2, StringComparison.OrdinalIgnoreCase );
		}

		/// <summary>Parse lines in the result file</summary>
		static IEnumerable<kvp> parse( string fileName )
		{
			eTest? test = null;
			foreach( string line in getLines( fileName ) )
			{
				if( line.isEqual( "SinCos" ) )
				{
					test = eTest.SinCos;
					continue;
				}
				if( line.isEqual( "BitCount" ) )
				{
					test = eTest.BitCount;
					continue;
				}
				if( !test.HasValue )
					continue;
				string[] fields = line.Split( '\t' );
				if( fields.Length < 3 )
					continue;
				int algo;
				if( !int.TryParse( fields[ 0 ], out algo ) )
					continue;
				double measure;
				if( !double.TryParse( fields[ 2 ], out measure ) )
					continue;
				yield return new kvp( key( test.Value, algo ), measure );
			}
		}

		/// <summary>The log file contains 3 tests per algorithm, this function groups them and calculates the average.</summary>
		static IEnumerable<kvp> average( this IEnumerable<kvp> lines )
		{
			Func<IEnumerable<kvp>, kvp> aggregate = ( IEnumerable<kvp> group ) =>
			{
				int c = 0;
				double sum = 0;
				foreach( var t in group )
				{
					c++;
					sum += t.Value;
				}
				var f = group.First();
				return new kvp( f.Key, sum / c );
			};

			return lines.GroupBy( t => t.Key ).Select( aggregate );
		}

		/// <summary>Read all files in the directory, parse, calculate tests average</summary>
		static Dictionary<string, dictI2D> readFiles( string dir )
		{
			string[] files = Directory.GetFiles( strFolder, "*.txt" );
			Dictionary<string, dictI2D> res = new Dictionary<string, dictI2D>( files.Length );

			foreach( string f in files )
			{
				string key = Path.GetFileNameWithoutExtension( f );

				Dictionary<int,double> val = parse( f ).average().ToDictionary( k => k.Key, k => k.Value );
				res[ key ] = val;
			}
			return res;
		}

		/// <summary>Get lines in this dataset.</summary>
		static int[] tableLines( this Dictionary<string, dictI2D> data )
		{
			HashSet<int> hs = new HashSet<int>();
			foreach( var c in data.Values )
				hs.UnionWith( c.Keys );
			return hs.OrderBy( k => k ).ToArray();
		}

		/// <summary></summary>
		static void printFields( this TextWriter tw, IEnumerable<string> fields )
		{
			tw.WriteLine( String.Join( "\t", fields ) );
		}

		/// <summary></summary>
		static void printTable( TextWriter tw, string title, Dictionary<string, dictI2D> data, IEnumerable<int> rows, string[] columns, Dictionary<int, string> algoNames )
		{
			tw.WriteLine( title );

			List<string> fields = new List<string>();
			fields.Add( "" );
			fields.AddRange( columns );
			tw.printFields( fields );

			foreach( int r in rows )
			{
				fields.Clear();
				fields.Add( algoNames[ r.algo() ] );
				foreach( string c in columns )
				{
					dictI2D dataColumn = data[ c ];
					double val;
					if( dataColumn.TryGetValue( r, out val ) )
						fields.Add( val.ToString() );
					else
						fields.Add( string.Empty );
				}
				tw.printFields( fields );
			}
		}

		/// <summary></summary>
		static void print( Dictionary<string, dictI2D> data, string dest )
		{
			int[] lines = data.tableLines();
			string[] columns = data.Keys.OrderBy( k => k ).ToArray();

			using( StreamWriter sw = new StreamWriter( dest ) )
			{
				printTable( sw, "SinCos", data, lines.Where( l => l.test() == eTest.SinCos ), columns, s_algoSinCos );
				sw.WriteLine();
				printTable( sw, "BitCount", data, lines.Where( l => l.test() == eTest.BitCount ), columns, s_algoCnt );
			}
		}

		static void calcRel( dictI2D column, eTest test, int algo )
		{
			int[] keys = column.Keys.ToArray();
			double rel = column[ key( test, algo ) ];
			foreach( int k in keys )
			{
				if( k.test() != test )
					continue;
				column[ k ] = 100.0 * column[ k ] / rel;
			}
		}

		/// <summary></summary>
		static void Main( string[] args )
		{
			// Reset culture to en-us, to match the output of C++ code
			CultureInfo english = new CultureInfo( "en-US" );
			CultureInfo.DefaultThreadCurrentCulture = english;
			CultureInfo.DefaultThreadCurrentUICulture = english;
			Thread.CurrentThread.CurrentCulture = english;
			Thread.CurrentThread.CurrentUICulture = english;

			// Read, parse, average
			Dictionary<string, dictI2D> data = readFiles( strFolder );

			// Print raw results
			print( data, strOutput );

			// Calculate relative values
			foreach( var d in data.Values )
			{
				calcRel( d, eTest.BitCount, 1 );
				calcRel( d, eTest.SinCos, 2 );
			}

			// Print relative results
			print( data, strOutputRel );
		}
	}
}
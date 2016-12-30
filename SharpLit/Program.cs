using System;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.IO;
using System.IO.Compression;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using CLitWrapper;
using static CLitWrapper.Util;

namespace SharpLit
{
    class Program
    {


        static void Main(string[] args)
        {
            var litPath = "x:\\clit\\source.lit";
            var epubPath = Path.Combine( "c:\\users\\bogda\\desktop", Path.GetRandomFileName() + ".epub");
            try
            {
                var exploder = new CLitExploder();
                var explodedDirectory = exploder.ExplodeToTempDir(litPath);
                System.Console.Out.WriteLine("Exploded to " + explodedDirectory);
                
                var exporter = new EpubExporter(epubPath);
                exporter.BuildFrom(explodedDirectory);

                System.Console.Out.WriteLine("Exported in " + epubPath);
            }
            catch (Exception e)
            {
                System.Console.Out.Write(e.Message);
            }
        }

    }
}

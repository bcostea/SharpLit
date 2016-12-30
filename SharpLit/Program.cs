using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using CLitWrapper;

namespace SharpLit
{
    class Program
    {
        static void Main(string[] args)
        {
            CLitExploder exploder = new CLitExploder();
            exploder.Explode("x:\\clit\\source.lit", "X:\\clit\\target\\");
            System.Console.Out.WriteLine("Done");
        }
    }
}

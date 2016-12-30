using System;
using System.Runtime.InteropServices;


namespace CLitWrapper
{


    public class CLitExploder
    {
        [DllImport("CLit.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern unsafe UInt32 explode(string litfile, string pathOutput);

        public unsafe void Explode(string litFile, string outputPath)
        {

            CLitExploder.explode(litFile, outputPath);

        }
    }
}

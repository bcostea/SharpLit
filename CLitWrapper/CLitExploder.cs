using System;
using System.IO;
using System.Runtime.InteropServices;


namespace CLitWrapper
{
    public class CLitExploder
    {
        [DllImport("CLit.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern unsafe UInt32 explode(string fileName, string pathOutput);

        public unsafe string Explode(string litFile, string outputPath)
        {
            CLitExploder.explode(litFile, outputPath);
            return outputPath;
        }

        public string ExplodeToTempDir(string litPath)
        {
            return Explode(litPath, Util.CreateTempDir());
        }

    }
}

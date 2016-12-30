using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CLitWrapper
{
    public class Util
    {
        public static string CreateTempDir()
        {
            var tempDirPath = Path.Combine(Path.GetTempPath(), Path.GetRandomFileName() + "\\");
            Directory.CreateDirectory(tempDirPath);
            return tempDirPath;
        }
    }
}

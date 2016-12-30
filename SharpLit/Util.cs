using System.IO;

namespace SharpLit
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

using System.IO;
using System.IO.Compression;

namespace SharpLit
{
    public class EpubExporter
    {
        public const string ContainerContent = @"<?xml version='1.0'?>
<container version = '1.0' xmlns = 'urn:oasis:names:tc:opendocument:xmlns:container'>
    <rootfiles>
        <rootfile full-path='content.opf' media-type='application/oebps-package+xml' />
    </rootfiles>
</container>";

        public string EpubPath { get; }

        public EpubExporter(string epubPath)
        {
            this.EpubPath = epubPath;
        }

        public void BuildFrom(string sourceFolder)
        {
            var epubDirectory = PrepareEpubScaffold();
            CopyContent(sourceFolder, epubDirectory);
            System.Console.Out.WriteLine("Copied to " + epubDirectory);

            var opfFileName = ProcessOpf(epubDirectory);

            System.Console.Out.WriteLine("opf file in " + opfFileName);

            ZipFile.CreateFromDirectory(epubDirectory, EpubPath, CompressionLevel.Optimal, false);

        }

        private string ProcessOpf(string epubDirectory)
        {
            var newName = Path.Combine(epubDirectory, "content.opf");
            string[] files = Directory.GetFiles(epubDirectory, "*.opf");
            if (files.Length >= 1)
            {
                var oldName = Path.Combine(epubDirectory, files[0]);
                File.Move(oldName, newName);
            }
            return newName;
        }

        private void CopyContent(string sourcePath, string targetPath)
        {
            var source = new DirectoryInfo(sourcePath);
            var target = new DirectoryInfo(targetPath);

            foreach (var directory in source.GetDirectories())
            {
                CopyContent(directory.FullName, target.CreateSubdirectory(directory.Name).FullName);
            }
            foreach (var file in source.GetFiles())
            {
                file.CopyTo(Path.Combine(target.FullName, file.Name));
            }
        }

        private string PrepareEpubScaffold()
        {
            var epubDir = Util.CreateTempDir();
            var metaDir = Path.Combine(epubDir, "META-INF\\");
            Directory.CreateDirectory(metaDir);

            var containerFilePath = Path.Combine(metaDir, "container.xml");
            if (!File.Exists(containerFilePath))
            {
                File.Create(containerFilePath).Dispose();
                using (TextWriter writer = new StreamWriter(containerFilePath))
                {
                    writer.Write(ContainerContent);
                    writer.Close();
                }
            }
            return epubDir;
        }
    }
}

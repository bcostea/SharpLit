using System;
using System.IO;
using System.Windows.Forms;
using CLitWrapper;
using SharpLit;

namespace SharpLitEpubExporter
{
    public partial class ExportForm : Form
    {
        public ExportForm()
        {
            InitializeComponent();
        }

        private void startExport_Click(object sender, EventArgs e)
        {
            progress.Value = 0;
            var currentFile = 0;

            var sourceDirectory = sourcePath.Text;
            var destinationDirectory = targetPath.Text;
            string[] files = Directory.GetFiles(sourceDirectory, "*.lit");

            progress.Maximum = files.Length;

            foreach (var litFile in files)
            {
                
                var litPath = Path.Combine(sourceDirectory, litFile);
                var epubPath = Path.Combine(destinationDirectory, Path.GetFileName(litFile) + ".epub");
                try
                {
                    var exploder = new CLitExploder();
                    var explodedDirectory = exploder.ExplodeToTempDir(litPath);
                    System.Console.Out.WriteLine("Exploded to " + explodedDirectory);

                    var exporter = new EpubExporter(epubPath);
                    exporter.BuildFrom(explodedDirectory);

                    System.Console.Out.WriteLine("Exported in " + epubPath);
                }
                catch (Exception ex)
                {
                    System.Console.Out.Write(ex.Message);
                }
                currentFile++;
                progress.Value = currentFile;
            }  
        }

        private void selectSource_Click(object sender, EventArgs e)
        {
            sourceBrowser.ShowDialog();
            if (!string.IsNullOrWhiteSpace(sourceBrowser.SelectedPath))
            {
                sourcePath.Text = sourceBrowser.SelectedPath;
            }

            RefreshStartExportButton();
        }


        private void selectTarget_Click(object sender, EventArgs e)
        {
            targetBrowser.ShowDialog();
            if (!string.IsNullOrWhiteSpace(targetBrowser.SelectedPath))
            {
                targetPath.Text = targetBrowser.SelectedPath;
            }
            RefreshStartExportButton();
        }


        private void RefreshStartExportButton()
        {
            if (!string.IsNullOrWhiteSpace(sourcePath.Text) && 
                !string.IsNullOrWhiteSpace(targetPath.Text))
            {
                startExport.Enabled = true;
            }
            else
            {
                startExport.Enabled = false;
            }
        }
    }
}

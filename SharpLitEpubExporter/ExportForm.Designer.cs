namespace SharpLitEpubExporter
{
    partial class ExportForm
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(ExportForm));
            this.panel1 = new System.Windows.Forms.Panel();
            this.progress = new System.Windows.Forms.ProgressBar();
            this.selectTarget = new System.Windows.Forms.Button();
            this.targetPath = new System.Windows.Forms.TextBox();
            this.label2 = new System.Windows.Forms.Label();
            this.selectSource = new System.Windows.Forms.Button();
            this.sourcePath = new System.Windows.Forms.TextBox();
            this.label1 = new System.Windows.Forms.Label();
            this.startExport = new System.Windows.Forms.Button();
            this.sourceBrowser = new System.Windows.Forms.FolderBrowserDialog();
            this.targetBrowser = new System.Windows.Forms.FolderBrowserDialog();
            this.panel1.SuspendLayout();
            this.SuspendLayout();
            // 
            // panel1
            // 
            this.panel1.Controls.Add(this.progress);
            this.panel1.Controls.Add(this.selectTarget);
            this.panel1.Controls.Add(this.targetPath);
            this.panel1.Controls.Add(this.label2);
            this.panel1.Controls.Add(this.selectSource);
            this.panel1.Controls.Add(this.sourcePath);
            this.panel1.Controls.Add(this.label1);
            this.panel1.Controls.Add(this.startExport);
            this.panel1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.panel1.Location = new System.Drawing.Point(0, 0);
            this.panel1.Name = "panel1";
            this.panel1.Size = new System.Drawing.Size(534, 201);
            this.panel1.TabIndex = 0;
            // 
            // progress
            // 
            this.progress.Location = new System.Drawing.Point(16, 115);
            this.progress.Name = "progress";
            this.progress.Size = new System.Drawing.Size(505, 23);
            this.progress.TabIndex = 7;
            // 
            // selectTarget
            // 
            this.selectTarget.Location = new System.Drawing.Point(399, 77);
            this.selectTarget.Name = "selectTarget";
            this.selectTarget.Size = new System.Drawing.Size(35, 23);
            this.selectTarget.TabIndex = 6;
            this.selectTarget.Text = "...";
            this.selectTarget.UseVisualStyleBackColor = true;
            this.selectTarget.Click += new System.EventHandler(this.selectTarget_Click);
            // 
            // targetPath
            // 
            this.targetPath.Enabled = false;
            this.targetPath.Location = new System.Drawing.Point(16, 79);
            this.targetPath.Name = "targetPath";
            this.targetPath.Size = new System.Drawing.Size(377, 20);
            this.targetPath.TabIndex = 5;
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(13, 63);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(151, 13);
            this.label2.TabIndex = 4;
            this.label2.Text = "Directory used for EPub export";
            // 
            // selectSource
            // 
            this.selectSource.Location = new System.Drawing.Point(399, 27);
            this.selectSource.Name = "selectSource";
            this.selectSource.Size = new System.Drawing.Size(35, 23);
            this.selectSource.TabIndex = 3;
            this.selectSource.Text = "...";
            this.selectSource.UseVisualStyleBackColor = true;
            this.selectSource.Click += new System.EventHandler(this.selectSource_Click);
            // 
            // sourcePath
            // 
            this.sourcePath.Enabled = false;
            this.sourcePath.Location = new System.Drawing.Point(16, 30);
            this.sourcePath.Name = "sourcePath";
            this.sourcePath.Size = new System.Drawing.Size(377, 20);
            this.sourcePath.TabIndex = 2;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(13, 13);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(178, 13);
            this.label1.TabIndex = 1;
            this.label1.Text = "Folder containing LIT format eBooks";
            // 
            // startExport
            // 
            this.startExport.Enabled = false;
            this.startExport.Location = new System.Drawing.Point(440, 26);
            this.startExport.Name = "startExport";
            this.startExport.Size = new System.Drawing.Size(81, 73);
            this.startExport.TabIndex = 0;
            this.startExport.Text = "Start conversion";
            this.startExport.UseVisualStyleBackColor = true;
            this.startExport.Click += new System.EventHandler(this.startExport_Click);
            // 
            // sourceBrowser
            // 
            this.sourceBrowser.ShowNewFolderButton = false;
            // 
            // ExportForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(534, 201);
            this.Controls.Add(this.panel1);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.MaximizeBox = false;
            this.Name = "ExportForm";
            this.Text = "SharpLIT EPub Exporter v0.1";
            this.panel1.ResumeLayout(false);
            this.panel1.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.Panel panel1;
        private System.Windows.Forms.Button startExport;
        private System.Windows.Forms.Button selectTarget;
        private System.Windows.Forms.TextBox targetPath;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Button selectSource;
        private System.Windows.Forms.TextBox sourcePath;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.FolderBrowserDialog sourceBrowser;
        private System.Windows.Forms.FolderBrowserDialog targetBrowser;
        private System.Windows.Forms.ProgressBar progress;
    }
}


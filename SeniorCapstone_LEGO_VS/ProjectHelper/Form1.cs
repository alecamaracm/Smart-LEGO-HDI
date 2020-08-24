using SharpMonoInjector;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace ProjectHelper
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();
        }

        private void button1_Click(object sender, EventArgs e)
        {
            Process.Start(@"C:\Program Files\Studio 2.0\Studio.exe");
        }

        private void button2_Click(object sender, EventArgs e)
        {
            Log("Injecting DLL...");
            var processes = Process.GetProcessesByName("Studio");

            if (processes.Any() == false)
            {
                Log("Studio process now found!");
                return;
            }
            try { 
                Injector injector=new Injector("Studio");
                IntPtr ptr=injector.Inject(File.ReadAllBytes("../../../StudioPlugin/bin/Debug/StudioPlugin.dll"), "StudioPlugin", "PluginLoader", "Load");

                Log("Injected successfully at "+ptr);
            } catch (InjectorException ie) {
                Log("Failed to inject assembly: " + ie);
            } catch (Exception exc) {
                Log("Failed to inject assembly (unknown error): " + exc);
            }

        }

        private void Log(string v)
        {
            labelStatus.Text = v;
            Console.WriteLine(v);
        }

        private void button3_Click(object sender, EventArgs e)
        {
            Process.GetProcessesByName("Studio").FirstOrDefault()?.Kill();
        }
    }
}

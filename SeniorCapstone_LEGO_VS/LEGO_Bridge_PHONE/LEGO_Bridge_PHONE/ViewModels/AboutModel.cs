using Android.Content;
using LEGO_Bridge_PHONE.Services;
using System;
using System.ComponentModel;
using System.Windows.Input;
using Xamarin.Essentials;
using Xamarin.Forms;

namespace LEGO_Bridge_PHONE.ViewModels
{
    public class AboutViewModel : BaseViewModel
    {
        public AboutViewModel()
        {
            OpenWebCommand = new Command(async () => await Browser.OpenAsync("https://github.com/alecamaracm/SeniorCapstone_LEGO"));           
        }

        public ICommand OpenWebCommand { get; }

    }
}
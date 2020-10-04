using Android.Content;
using LEGO_Bridge_PHONE.Services;
using System;
using System.ComponentModel;
using System.Windows.Input;
using Xamarin.Essentials;
using Xamarin.Forms;

namespace LEGO_Bridge_PHONE.ViewModels
{
    public class InformationViewModel : BaseViewModel
    {
        public InformationViewModel()
        {
            Title = "Information";
            OpenWebCommand = new Command(async () => await Browser.OpenAsync("https://github.com/alecamaracm/SeniorCapstone_LEGO"));
            ToggleAction = new Command(async () => await BLEThings.ToggleAction(this));
        }

        public ICommand OpenWebCommand { get; }
        public ICommand ToggleAction { get; }

        public string toggleText="Start work";

        public string ToggleText
        {
            get => toggleText;
            set
            {
                SetProperty(ref toggleText, value);
            }
        }

        public string statusText = "Idle";

        public string StatusText
        {
            get => statusText;
            set
            {
                SetProperty(ref statusText, value);
            }
        }

        public string ip = "192.168.137.1";

        public string IP2Connect
        {
            get => ip;
            set
            {
                SetProperty(ref ip, value);
            }
        }

        

    }
}
using System;
using Xamarin.Forms;
using Xamarin.Forms.Xaml;
using LEGO_Bridge_PHONE.Services;
using LEGO_Bridge_PHONE.Views;

namespace LEGO_Bridge_PHONE
{
    public partial class App : Application
    {

        public App()
        {
            InitializeComponent();

            DependencyService.Register<MockDataStore>();
            MainPage = new AppShell();
        }

        protected override void OnStart()
        {
        }

        protected override void OnSleep()
        {
        }

        protected override void OnResume()
        {
        }
    }
}

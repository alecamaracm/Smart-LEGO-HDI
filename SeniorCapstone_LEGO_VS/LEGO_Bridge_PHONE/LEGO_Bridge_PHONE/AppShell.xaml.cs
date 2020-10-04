using System;
using System.Collections.Generic;
using LEGO_Bridge_PHONE.ViewModels;
using LEGO_Bridge_PHONE.Views;
using Xamarin.Forms;

namespace LEGO_Bridge_PHONE
{
    public partial class AppShell : Xamarin.Forms.Shell
    {
        public AppShell()
        {
            InitializeComponent();
            Routing.RegisterRoute(nameof(ItemDetailPage), typeof(ItemDetailPage));
            Routing.RegisterRoute(nameof(NewItemPage), typeof(NewItemPage));
        }

        private async void OnMenuItemClicked(object sender, EventArgs e)
        {
            await Shell.Current.GoToAsync("//LoginPage");
        }
    }
}

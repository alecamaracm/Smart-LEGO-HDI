using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using LEGO_Bridge_PHONE.ViewModels;
using Xamarin.Forms;
using Xamarin.Forms.Xaml;

namespace LEGO_Bridge_PHONE.Views
{
    [XamlCompilation(XamlCompilationOptions.Compile)]
    public partial class LoginPage : ContentPage
    {
        public LoginPage()
        {
            InitializeComponent();
            this.BindingContext = new LoginViewModel();
        }
    }
}
using System.ComponentModel;
using Xamarin.Forms;
using LEGO_Bridge_PHONE.ViewModels;

namespace LEGO_Bridge_PHONE.Views
{
    public partial class ItemDetailPage : ContentPage
    {
        public ItemDetailPage()
        {
            InitializeComponent();
            BindingContext = new ItemDetailViewModel();
        }
    }
}
using System;
using System.Reflection;
using Xamarin.Forms;

namespace opentheknock
{
    public partial class Welcome : ContentPage
    {
        public Welcome()
        {
            InitializeComponent();

            //            var embeddedImage = new Image { Source = ImageSource.FromResource("opentheknock.opentheknock-logo.png", Assembly.GetExecutingAssembly()) };

            // TODO: Debug

//            Logo.Source = ImageSource.FromFile("logo.png");
        }

        async void OnLoginClicked(object sender, EventArgs e)
        {


            if (UserIdInput.SelectedIndex < 0)
            {
                await DisplayAlert("Error:", "You should select some user!", "OK");
                IsEnabled = true;
                return;
            }

            Loading.IsVisible = true;
            LoginButton.IsVisible = false;
            UserIdInput.IsVisible = false;
            var animateEnd = await Loading.FadeTo(1d);
            try
            {
                await Navigation.PushAsync(new MainPage(UserIdInput.SelectedIndex + 1));
            }
            finally
            {
                UserIdInput.IsVisible = true;
                LoginButton.IsVisible = true;
                Loading.IsVisible = false;

            }

        }

        async void OnIndexChange(object sender, EventArgs e)
        {
//            var picker = (Picker)sender;
//            await Navigation.PushAsync(new MainPage(picker.SelectedIndex + 1));
//            picker.Unfocus();
            LoginButton.Focus();
        }

    }
}



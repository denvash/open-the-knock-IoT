using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using Xamarin.Forms;
using Microsoft.Azure.Devices;

namespace opentheknock
{
    public partial class MainPage : ContentPage
    {
        private readonly UserItem userItem;
        DBManager manager;

        public MainPage(int index)
        {
            InitializeComponent();
            manager = DBManager.DefaultManager;
            userItem = manager.GetUserByUserId(index);
            EmailInput.Text = userItem.Email;
            EmailNotificationsInput.IsToggled = userItem.EmailNotifications;

            var message = "Welcome userID=" + userItem.UserId + "!";
            MainPageContent.Children.Add(new Label { FontAttributes = FontAttributes.Bold, Text = message, TextColor = Color.FromHex("#000000") });
        }
        async void OnSetChallengeClicked(object sender, EventArgs e)
        {
            Loading.IsVisible = true;
            SetChallenge.IsEnabled = false;
            var animateEnd = await Loading.FadeTo(1d);
            try
            {
                await Navigation.PushAsync(new SetChallenge(userItem));
            }
            finally
            {
                SetChallenge.IsEnabled = true;
                Loading.IsVisible = false;

            }
        }

        async void UpdateButtonClicked(object sender, EventArgs e)
        {

            Loading.IsVisible = true;
            UpdateEmail.IsEnabled = false;
            var animateEnd = await Loading.FadeTo(1d);
            try
            {
                await manager.SaveData(new UserItem()
                {
                    UserId = userItem.UserId,
                    Email = EmailInput.Text,
                    EmailNotifications = EmailNotificationsInput.IsToggled,
                    Challenge1 = userItem.Challenge1,
                    Challenge2 = userItem.Challenge2,
                    Challenge3 = userItem.Challenge3
                });
            }
            finally
            {
                DisplayAlert("Message", "Email settings for userID=" + userItem.UserId + " was successfully updated", "OK");
                UpdateEmail.IsEnabled = true;
                Loading.IsVisible = false;

            }


        }

        async void OpenButtonClicked(object sender, EventArgs e)
        {
            Loading.IsVisible = true;
            ToggleLock.IsEnabled = false;
            var animateEnd = await Loading.FadeTo(1d);
            try
            {
                InvokeMethod("openLock", "{ \"id\": " + userItem.UserId + "}").Wait();
            }
            finally
            {
                DisplayAlert("Message", "Lock opened!", "OK");
                ToggleLock.IsEnabled = true;
                Loading.IsVisible = false;

            }
        }

        private static async Task InvokeMethod(string methodName, string payloadJson)
        {
            var s_serviceClient = ServiceClient.CreateFromConnectionString("HostName=otk-iot.azure-devices.net;SharedAccessKeyName=iothubowner;SharedAccessKey=o+wrTuFd7yoFi/DMjJXKkNWXwlh04EiLkblShFTN9sk=");

            Console.WriteLine("InvokeMethod called with payload:");
            Console.WriteLine(payloadJson);
            var methodInvocation = new CloudToDeviceMethod(methodName) { ResponseTimeout = TimeSpan.FromSeconds(30) };
            methodInvocation.SetPayloadJson(payloadJson);

            // Invoke the direct method asynchronously and get the response from the simulated device.
            var response = s_serviceClient.InvokeDeviceMethodAsync("myKnockDevice", methodInvocation);

            Console.WriteLine("Response status: {0}, payload:", response.Status);
//            Console.WriteLine(response.GetPayloadAsJson());
        }
    }
}
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using Xamarin.Forms;

namespace opentheknock
{
    public partial class SetChallenge : ContentPage
    {
        DBManager manager;
        private UserItem userItem;
        public ObservableCollection<SelectableItem<string>> Challenge3Items { get; }

        public SetChallenge(UserItem userItem)
        {
            InitializeComponent();

            manager = DBManager.DefaultManager;

            Challenge3Items = new ObservableCollection<SelectableItem<string>>();

            BindingContext = this;

            this.userItem = userItem;

            var message = "Set / Update challenge for userID=" + userItem.UserId;
            TopMessage.Children.Add(new Label { Text = message, TextColor = Color.FromHex("#000000") });

            Dictionary<int, string> challengesMap = manager.GetUsersChallenges(userItem.UserId).Item3;
            string challenge;
            Challenge1Input.Text = challengesMap.TryGetValue(1, out challenge) ? challenge : "";
            Challenge2Input.Text = challengesMap.TryGetValue(2, out challenge) ? challenge : "";
            if (challenge == null)
            {
                GenerateDynamicGrid("000000000");
            }
            else
            {
                GenerateDynamicGrid(challenge);
            }

            string[] MenuItems =
            {
                "Supreme Nachos", "Potato Skins", "Burning Chips", "Home Salat", "Corona",
                "Heineken", "Castille", "Edelweiss", "Coca Cola", "Sprite", "Fanta"
            };

            Challenge3Input.Text = challengesMap.TryGetValue(3, out challenge) ? challenge : "";
            if (challenge == null || Challenge3Input.Text.IndexOf("|") < 0)
            {
                foreach (var menuItem in MenuItems)
                {
                    Challenge3Items.Add(new SelectableItem<string>(menuItem, false));
                }
            }
            else
            {
                string[] ExistingItems = challenge.Split(new [] { " | "}, StringSplitOptions.None);
                foreach (var menuItem in MenuItems)
                {
                    if (ExistingItems.Contains(menuItem))
                    {
                        Challenge3Items.Add(new SelectableItem<string>(menuItem, true));
                    }
                    else
                    {
                        Challenge3Items.Add(new SelectableItem<string>(menuItem, false));
                    }
                }
            }
        }

        // Called when user clicks "Set/Update challenge"
        async void UpdateButtonClicked(object sender, EventArgs e)
        {
            if (Challenge1Input.Text == "")
            {
                await DisplayAlert("Error: Challenge 1", "You should insert some string!", "OK");
                return;
            }
            if (Challenge2Input.Text == "" || Challenge2Input.Text == "000000000")
            {
                await DisplayAlert("Error: Challenge 2", "You should select at least one cell in the grid!", "OK");
                return;
            }
            if (Challenge3Input.Text == "" || 
                (Challenge3Input.Text).Count(f => f == '|') < 2 || 
                (Challenge3Input.Text).Count(f => f == '|') > 5)
            {
                await DisplayAlert("Error: Challenge 3", "You should select between 2 to 5 menu items!", "OK");
                return;
            }


            Loading.IsVisible = true;
            UpdateButton.IsEnabled = false;
            var animateEnd = await Loading.FadeTo(1d);
            try
            {
                await manager.SaveData(new UserItem()
                {
                    UserId = userItem.UserId,
                    Email = userItem.Email,
                    EmailNotifications = userItem.EmailNotifications,
                    Challenge1 = Challenge1Input.Text,
                    Challenge2 = Challenge2Input.Text,
                    Challenge3 = Challenge3Input.Text
                });

            }
            finally
            {
                DisplayAlert("Message", "Challenges for userID=" + userItem.UserId + " was successfully updated", "OK");
                UpdateButton.IsEnabled = true;
                Loading.IsVisible = false;
            }
        }

        // Generates dynamic grid by string for challenge 2
        private void GenerateDynamicGrid(string gridString)
        {
            var grid = new Grid
            {
                BackgroundColor = Color.Black,
                ColumnSpacing = 1,
                RowSpacing = 1
            };
            for (var i = 0; i < 3; i++)
            {
                grid.RowDefinitions.Add(new RowDefinition {Height = new GridLength(50)});
            }

            for (var i = 0; i < 3; i++)
            {
                grid.ColumnDefinitions.Add(new ColumnDefinition {Width = new GridLength(50)});
            }

            Point[] blackCells =
            {
                new Point(0, 0),
                new Point(0, 1),
                new Point(0, 2),
                new Point(1, 1),
                new Point(2, 1)
            };

            var boxesInputArray = gridString.ToCharArray();


            BoxView[] arrayOfBoxes = new BoxView[9];
            int[] arrayOfBits = new int[9];
            TapGestureRecognizer[] tapRecognizers = new TapGestureRecognizer[9];
            var counter = 0;
            for (var i = 0; i < 3; i++)
            {
                for (var j = 0; j < 3; j++)
                {
                    var counter1 = counter;
                    var color = Color.White;
//                    if (blackCells.Contains(new Point(i, j)))
//                    {
//                        color = Color.Black;
//                        arrayOfBits[counter1] = 1;
//                    }

                    if (boxesInputArray[counter1] == '1')
                    {
                        color = Color.Black;
                        arrayOfBits[counter1] = 1;
                    }


                    arrayOfBoxes[counter] = new BoxView {Color = color};
                    tapRecognizers[counter] = new TapGestureRecognizer();
                    arrayOfBoxes[counter].GestureRecognizers.Add(tapRecognizers[counter]);

                    tapRecognizers[counter].Tapped += (s, e) =>
                    {
                        arrayOfBoxes[counter1].Color = arrayOfBoxes[counter1].Color == Color.Black ? Color.White : Color.Black;
                        arrayOfBits[counter1] = 1 - arrayOfBits[counter1];
                        var result = string.Join("", arrayOfBits.Select(x => x.ToString()).ToArray());
                        Challenge2Input.Text = result;
                    };
                    grid.Children.Add(arrayOfBoxes[counter], j, i);
                    counter++;
                }
            }

            var defaultGrid = string.Join("", arrayOfBits.Select(x => x.ToString()).ToArray());
            Challenge2Input.Text = defaultGrid;
            var frame = new Frame
            {
                Content = grid,
                OutlineColor = Color.Black,
                Padding = new Thickness(0, 0, 0, 0),
                //                CornerRadius = 1
            };

            Challenge2InputMatrix.Children.Add(frame);
        }

        // Called when menu item is selected from challenge 3 menu
        private void OnItemSelected(object sender, SelectedItemChangedEventArgs e)
        {
            var item = e.SelectedItem as SelectableItem;
            if (item != null)
            {
                // Challenge have never been set, or the user unselected all items
                if (Challenge3Input.Text.IndexOf("|") < 0) 
                {
                    Challenge3Input.Text = "";
                }
                if (!item.IsSelected)
                {
                    Challenge3Input.Text += item.Data + " | ";
                }
                else
                {
                    var text = Challenge3Input.Text;
                    Challenge3Input.Text = text.Replace(item.Data + " | ", "");
                }
                // toggle the selection property
                item.IsSelected = !item.IsSelected;
                
            }


            // deselect the item
            ((ListView)sender).SelectedItem = null;
        }

    }
}



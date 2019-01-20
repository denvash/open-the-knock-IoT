using Xamarin.Forms;


namespace opentheknock
{
    public class SelectableItem : BindableObject
    {
        public static readonly BindableProperty DataProperty =
            BindableProperty.Create(
                nameof(Data),
                typeof(object),
                typeof(SelectableItem),
                (object)null);

        public static readonly BindableProperty IsSelectedProperty =
            BindableProperty.Create(
                nameof(IsSelected),
                typeof(bool),
                typeof(SelectableItem),
                false);

        public SelectableItem(object data)
        {
            Data = data;
            IsSelected = false;
        }

        public SelectableItem(object data, bool isSelected)
        {
            Data = data;
            IsSelected = isSelected;
        }

        public object Data
        {
            get { return (object)GetValue(DataProperty); }
            set { SetValue(DataProperty, value); }
        }

        public bool IsSelected
        {
            get { return (bool)GetValue(IsSelectedProperty); }
            set { SetValue(IsSelectedProperty, value); }
        }
    }

    public class SelectableItem<T> : SelectableItem
    {
        public SelectableItem(T data)
            : base(data)
        {
        }

        public SelectableItem(T data, bool isSelected)
            : base(data, isSelected)
        {
        }

        // this is safe as we are just returning the base value
        public new T Data
        {
            get { return (T)base.Data; }
            set { base.Data = value; }
        }
    }
}

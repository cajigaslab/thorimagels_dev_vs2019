using System;
using System.Collections.Generic;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Threading;
using ThorDAQConfigControl.ViewModel;
using ThorSharedTypes;

namespace ThorDAQConfigControl.Controls
{
    /// <summary>
    /// Interaction logic for BOBItem.xaml
    /// </summary>
    public partial class BOBItem : UserControl
    {
        public int LEDEnumValue; // BBoxLEDenum
     //   Byte State; // 0 = off, 1 = on, 2 = blink
        public string BobName;
        public bool IsNormalState;
 //       private DispatcherTimer blinkTimer;
 //       private const int TimerInterval = 400;
//        private bool isBlinking;
        private MainContentViewModel vm;

        public BOBItem(int LEDenum, string name)
        {
            InitializeComponent();
            LEDEnumValue = LEDenum;
            BobName = name;
            nameLabel.Content = name;
            vm = MainContentViewModel.GetInstance();

       //     blinkTimer = new DispatcherTimer();
       //     blinkTimer.Interval = TimeSpan.FromMilliseconds(TimerInterval);
       //     blinkTimer.Tick += OnBlinkTimer;
        }

        private void OnBlinkTimer(object sender, EventArgs e)
        {
//            SetBobState(!IsNormalState);
        }

        public void TurnOn()
        {
          //  if (isBlinking)
                StopBlink();
            UpdateBobLED(1);
        }

        public void TurnOff()
        {
      //      if (isBlinking)
      //          StopBlink();
            UpdateBobLED(0);
        }

        public void StartBlink()
        {
            UpdateBobLED(2);
            //blinkTimer.Start();
            //isBlinking = true;
        }

        public void StopBlink()
        {
            UpdateBobLED(0);
            //  blinkTimer.Stop();
            //  isBlinking = false;
        }

        // update LED by the value from device
        // use BLINK LED state
        private void UpdateBobLED(Byte state)
        {
//            IsNormalState = state;
            if (state == 1)
                statusImage.Source = (ImageSource)Resources["Indicator_NormalDrawingImage"];
            else
                statusImage.Source = (ImageSource)Resources["Indicator_IdleDrawingImage"];

        }

        // Set new state to device
        private void SetBobState(Byte State)
        {
            var ledString = Enum.GetName(typeof(BBoxLEDenum), LEDEnumValue);
            string sState = "off";
            switch( State)
            {
                case 1:
                    sState = "on";
                    break;
                case 2:
                    sState = "blink";
                    break;
                case 0:
                default:                    
                    sState = "off";
                    break;

            }
            var ret = vm.commandProvider.APIbreakOutBoxLEDControl(new List<string> { "APIBreakoutBoxLED", "-" + ledString, sState });
            // Currently can only predict status by the return value
   //         UpdateBobLED(ret);
        }

        private void bobImage_MouseLeftButtonUp(object sender, MouseButtonEventArgs e)
        {
            // stop blinking the LED
            SetBobState(0);//!IsNormalState);
        }
        private void bobImage_MouseLeftButtonDown(object sender, MouseButtonEventArgs e)
        {
            // blink the LED
            SetBobState(2);//!IsNormalState);

            // do not show resource configuration dialog
            //eventAggregator.GetEvent<ShowResourceConfigurationEvent>().Publish(Name);
            /*
            if (isBlinking)
                StopBlink();
            else
                StartBlink();
            */
        }
    }
}

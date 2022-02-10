using System;
using System.Collections.Generic;
using System.Configuration;
using System.Data;
using System.Linq;
using System.Threading.Tasks;
using System.Windows;

using SciChart.Charting.Visuals;

namespace ThorSync
{
    /// <summary>
    /// Interaction logic for App.xaml
    /// </summary>
    public partial class App : Application
    {
        // Ensure SetLicenseKey is called once, before any SciChartSurface instance is created 
        // Check this code into your version-control and it will enable SciChart 
        // for end-users of your application. 
        // 
        // You can test the Runtime Key is installed correctly by Running your application 
        // OUTSIDE Of Visual Studio (no debugger attached). Trial watermarks should be removed. 
        public App()
        {
            // Set this code once in App.xaml.cs or application startup
            // Set this code once in App.xaml.cs or application startup
            SciChartSurface.SetRuntimeLicenseKey("TCqHmTkInF/fDQuv2IRL2jISc44wjQP46+iIvQjEtY21jW+X66HmcupG3FzPOD39A8zSj8i8vKIUgW2r9wgDzzuy3RK/gQsogW5d2SN0QVo0tnTzAd/uEWHLFeS2W17/2hf//FVKxwU4704JENFsCxYbOoPZHbpNwbTJovnl1QjEabIjy1KzBkA2fJMJbWF8wPRTD0ruKUEnrHpXOuvpTOQlr7a6XSmUlJ5o/Vsx7oJRcIYm70L7shDDXu1hHEqICpBtcCb91kpgNMaAZoWJwhYiBmowdHbgszC9lm3o6hlLi35y88379sblqhR1b7rIh80hoc3XwfQUmPydvU6RAwLUyIYT/z28JOl3kx0pReVdlLQd5bfdldNeNrI6J3ajng427j2udkQpNqQxNUEbLH9D/qqr5xeez+F/O4FWIYiYJvs9pgMamA6GYfGnV1sQ2spekHboGxh5PWfNgAWTuqFU/arLx5W1LYhT75WcXUe8pSXX1JD6qGD7/G4l9KpN+CYuZrXh1Zl9ND5KLicMDvfX65W+B8ka0TZbLIFExmsWSwNt+n6osLwE48Q8JsPb1+WCzy+1oCaFnyGXcpK5LlVB0Dcg9VdcDnwmrEQ=");
        }
    }
}
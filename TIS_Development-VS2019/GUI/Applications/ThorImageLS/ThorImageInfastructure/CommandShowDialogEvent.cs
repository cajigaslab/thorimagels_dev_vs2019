namespace ThorImageInfastructure
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Text;

    using Microsoft.Practices.Composite.Wpf.Events;

    public class CommandFinishedDialogEvent : CompositeWpfEvent<Command>
    {
    }

    public class CommandFinishedScriptEvent : CompositeWpfEvent<Command>
    {
    }

    public class CommandReviewModalityEvent : CompositeWpfEvent<Command>
    {
    }

    public class CommandShowDialogEvent : CompositeWpfEvent<Command>
    {
    }

    public class HardwareSettingsChangeEvent : CompositeWpfEvent<ChangeEvent>
    {
    }

    public class IPCModuleChangeEvent : CompositeWpfEvent<ChangeEvent>
    {
    }

    public class MenuModuleChangeEvent : CompositeWpfEvent<ChangeEvent>
    {
    }
}
namespace ThorSharedTypes
{
    using System;
    using System.Reflection;

    /// <summary>
    /// Double property event arguments.
    /// </summary>
    public class DoublePropertyArgs : System.EventArgs
    {
        #region Constructors

        /// <summary>
        /// Constructor.
        /// </summary>
        /// <param name="dValue">The progress message</param>
        public DoublePropertyArgs(double dValue)
        {
            DoubleValue = dValue;
        }

        #endregion Constructors

        #region Properties

        public double DoubleValue
        {
            get; set;
        }

        #endregion Properties
    }

    /// <summary>
    /// Contains extension methods that deal with objects.
    /// </summary>
    public static class DynamicEventHandler
    {
        #region Methods

        /// <summary>
        /// Adds an event handler to an object dynamically.
        /// </summary>
        /// <typeparam name="T1">The target object type</typeparam>
        /// <typeparam name="T2">The handler source object type</typeparam>
        /// <param name="target">The target object</param>
        /// <param name="eventName">The name of the event to subcribe to</param>
        /// <param name="handlerName">The name of the method that will handle the event</param>
        /// <param name="handlerSource">The source object containing the handler method</param>
        /// <returns>A reference to the event handler delegate that was added</returns>
        /// <exception cref="MissingMemberException"></exception>
        /// <exception cref="MissingMethodException"></exception>
        public static Delegate AddDynamicEventHandler<T1, T2>(this T1 target, string eventName, string handlerName, T2 handlerSource)
            where T1 : class
            where T2 : class
        {
            EventInfo eventInfo = target.GetType().GetEvent(eventName);

            if (null != eventInfo && null != eventInfo?.EventHandlerType)
            {
                MethodInfo methodInfo = handlerSource.GetType().GetMethod(handlerName, BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Instance);

                if (null != methodInfo)
                {
                    Delegate handler = Delegate.CreateDelegate(eventInfo?.EventHandlerType, handlerSource, methodInfo);
                    eventInfo.AddEventHandler(target, handler);
                    return handler;
                }
            }
            return null;
        }

        /// <summary>
        /// Removes an event handler from an object dynamically.
        /// </summary>
        /// <typeparam name="T">The target object type</typeparam>
        /// <param name="target">The target object</param>
        /// <param name="eventName">The name of the event to unsubcribe from</param>
        /// <param name="handler">The event handler delegate to remove</param>
        /// <exception cref="MissingMemberException"></exception>
        public static void RemoveDynamicEventHandler<T>(this T target, string eventName, Delegate handler)
            where T : class
        {
            if (null != handler)
            {
                EventInfo eventInfo = target.GetType().GetEvent(eventName);
                eventInfo?.RemoveEventHandler(target, handler);
            }
        }

        #endregion Methods
    }

    /// <summary>
    /// Int property event arguments.
    /// </summary>
    public class IntPropertyArgs : System.EventArgs
    {
        #region Constructors

        /// <summary>
        /// Constructor.
        /// </summary>
        /// <param name="dValue">The progress message</param>
        public IntPropertyArgs(int iValue)
        {
            IntValue = iValue;
        }

        #endregion Constructors

        #region Properties

        public int IntValue
        {
            get; set;
        }

        #endregion Properties
    }
}
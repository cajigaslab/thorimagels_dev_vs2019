namespace VtkVolumeControl
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Runtime.InteropServices;
    using System.Text;
    using System.Threading.Tasks;

    /// <summary>
    /// Class used to load a volume from a raw file
    /// </summary>
    class RawVolume
    {
        #region Constructors

        /// <summary>
        /// Constructs a class with the specified volume loaded into a interop compatible buffer.
        /// </summary>
        /// <param name="filePath"> Path to raw file </param>
        /// <param name="dimensions"> Dimensions of raw file </param>
        /// <param name="selectedTime"> Selected time to load from raw file </param>
        /// <param name="selectedChannel"> Selected channel to load from raw file </param>
        /// <param name="enabledChannelsBitmaks"> Bitmask of enabled channels in raw file </param>
        /// <param name="containsDisabledChannels"> Boolean specifying if the raw file contains
        /// placeholder buffers for disabled channels, or only enabled channels </param>
        public RawVolume(string filePath, RawDimensions dimensions, int selectedTime, int selectedChannel, int enabledChannelsBitmaks, bool containsDisabledChannels)
        {
            FilePath = filePath;
            Dimensions = dimensions;
            SelectedTime = selectedTime;
            SelectedChanel = selectedChannel;
            EnabledChannelsBitmask = enabledChannelsBitmaks;
            ContainsDisabledChannels = containsDisabledChannels;

            LoadVolume();
        }

        #endregion Constructors

        #region Properties

        public bool ContainsDisabledChannels
        {
            get; private set;
        }

        public RawDimensions Dimensions
        {
            get;  private set;
        }

        public int EnabledChannelsBitmask
        {
            get; private set;
        }

        public string FilePath
        {
            get; private set;
        }

        public int SelectedChanel
        {
            get; private set;
        }

        public int SelectedTime
        {
            get; private set;
        }

        public IntPtr VolumeData
        {
            get; private set;
        }

        #endregion Properties

        #region Methods

        /// <summary>
        /// Loads a single slice of a raw file into the specified buffer, at the channel offset in the buffer specified
        /// </summary>
        /// <param name="outputBuffer"> The buffer to load into </param>
        /// <param name="channelInOutputBuffer"> The channel in the outputBuffer that will be loaded into. If the output buffer
        /// is only one channel then a value of 0 should be used to specify loading into channel index 0 </param>
        /// <param name="fileName"> The name and path of the raw file </param>
        /// <param name="width"> The width of images in the raw file </param>
        /// <param name="height"> The height of images in the raw file </param>
        /// <param name="zDepth"> The depth of volumes in the raw file </param>
        /// <param name="channels"> The number of channels in the raw file </param>
        /// <param name="loadChannel"> The channel of the slice to copy out of the raw file </param>
        /// <param name="zSlice"> The depth of the slice to copy out of the channel </param>
        /// <param name="time"> The time of the the image in the file to read </param>
        /// <param name="enabledChannelsBitmask"> Bitmask representing the enabled channels in the raw file </param>
        /// <param name="containsDisabledChannels"> Whether the raw file contains placeholder data for disabled channels </param>
        /// <returns> Returns 1 if there is no error </returns>
        [DllImport(".\\ThorDiskIO.dll", EntryPoint = "ReadChannelImageRawSliceToChannel")]
        private static extern int ReadChannelImageRawSlice(IntPtr outputBuffer, int channelInOutputBuffer, [MarshalAs(UnmanagedType.LPStr)]string fileName, int width, int height,
            int zDepth, int channels, int loadChannel, int zSlice, int time, int enabledChannelsBitmask, bool containsDisabledChannels);

        /// <summary>
        /// Allocates an inter-operable buffer of the required size for an image volume
        /// </summary>
        private void AllocateVolumeBuffer()
        {
            if (VolumeData != null)
            {
                Marshal.FreeHGlobal(VolumeData);
            }
            VolumeData = Marshal.AllocHGlobal(VolumeLength() + SliceLength()); //Add additional slice to end to fix bug where last slice doesn't render
        }

        /// <summary>
        /// Loads the volume from the file
        /// </summary>
        private void LoadVolume()
        {
            AllocateVolumeBuffer();

            for (int slice = 0; slice < Dimensions.ImageDepth; slice++)
            {
                IntPtr slicePtr = VolumeData + SliceLength() * slice;
                ReadChannelImageRawSlice(slicePtr, 0, FilePath, Dimensions.Width, Dimensions.Height, Dimensions.TotalDepth, Dimensions.Channels, SelectedChanel, slice, SelectedTime, EnabledChannelsBitmask, ContainsDisabledChannels);
            }
        }

        /// <summary>
        /// Calculate the length in bytes of a single slice of the volume's data
        /// </summary>
        /// <returns> The length in bytes of a single slice of the volume </returns>
        private int SliceLength()
        {
            return Dimensions.Width * Dimensions.Height * 2;
        }

        /// <summary>
        /// Calculate the length in bytes of the volume's data
        /// </summary>
        /// <returns> Returns the length in bytes of the volume's data </returns>
        private int VolumeLength()
        {
            return SliceLength() * Dimensions.ImageDepth;
        }

        #endregion Methods
    }
}
#pragma once
#include <vector>
#include <algorithm>


/// <summary>
/// A template class containing static utility methods that assist in the manipulation and extraction of image channel information
/// <remarks>
/// Information about enabled channels can be stored in different formats. In the Image and Raw file template classes the 
/// information is stored in vectors that contain an int for every enabled channel. Other uses are vectors of enabled channels and bitmasks.
/// To make the conversion and manipulation of these storage means easier this class contains numerous functions. This class is created as a template
/// so that it can be included with the other templates in the image and raw file set, making it easier to use in other code. </remarks>
template <typename T>
class ChannelManipulator
{
public:

	static int getDisabledChannelsBefore(int requestedChannel, const std::vector<int>& enabledChannels);
	static int getSequentialMemoryChannel(int absoluteChannel,  const std::vector<int>& enabledChannels);
	static std::vector<int> getSequentialMemoryChannels(const std::vector<int>& enabledChannels, const std::vector<int>& absoluteChannels);
	static bool isChannelEnabled(int channel, const std::vector<int>& enabledChannels);
	static std::vector<int> getDisabledChannels(int totalChannels, const std::vector<int>& enabledChannels);
	static std::vector<int> getEnabledChannels(int enabledChannelBitmask);
	static std::vector<int>& removeChannels(std::vector<int>& enabledChannels, std::vector<int> channelsToRemove);


private:
	ChannelManipulator(void);
};


template <typename T> ChannelManipulator<T>::ChannelManipulator(void)
{
}

/// <summary> Get the sequential channel in memory referred to by the input channel num. This applies to raw files that contain only the active channels.
/// for example, if out of 4 channels 0 and 1 are disabled, asking for channel 2 corresponds to the first sequential channel, and channel 3 corresponds
/// to the second sequential channel. If a disabled channel is requested, the sequential channel of the next enabled absolute channel is returned </summary>
/// <param name="absoluteChannel"> The absolute channel ie. the channel index out of all possibly enabled channels. Index starts at 0 </param>
/// <param name="enabledChannels"> Vector containing int's for every enabled channel </param>
/// <returns> The sequential channel based on only enabled channels </returns>
template <typename T> int ChannelManipulator<T>::getSequentialMemoryChannel(int absoluteChannel,  const std::vector<int>& enabledChannels)
{
	return absoluteChannel - getDisabledChannelsBefore(absoluteChannel, enabledChannels);
}


/// <summary> Returns the number of disabled channels before the requested channel, non inclusive </summary>
/// <param name="requestedChannel"> The channel to count up until </param>
/// <param name="enabledChannels"> Vector containing int's for every enabled channel </param>
/// <returns> The number of disabled channels before the requested channel, non inclusive </returns>
template <typename T> int ChannelManipulator<T>::getDisabledChannelsBefore(int requestedChannel, const std::vector<int>& enabledChannels)
{

	int enabledChannelsBefore = 0;
	for(int ch : enabledChannels)
	{
		if(ch < requestedChannel)
			enabledChannelsBefore++;
	}
	enabledChannelsBefore;

	return requestedChannel - enabledChannelsBefore;

}

/// <summary> Flip a vector of enabled channels by returning a vector of disabled channels </summary>
/// <param name="totalChannels"> The total number of channels that are possible to be enabled or disabled </param>
/// <param name="enabledChannels"> Vector containing the enabled channels </param>
/// <returns> Returns a vector containing all disabled channels </returns>
template <typename T> std::vector<int> ChannelManipulator<T>::getDisabledChannels(int totalChannels, const std::vector<int>& enabledChannels)
{

	std::vector<int> disabledChannels;
	
	//=== Find Non Active Channels
	for(int i=0; i<totalChannels; i++)
	{
		if(std::find(enabledChannels.begin(), enabledChannels.end(), i) == enabledChannels.end())
		{
			disabledChannels.push_back(i);
		}
	}

	return disabledChannels;

}

/// <summary> Returns the enabled channels as a vector based on an input bitmask </summary>
/// <param name="enabledChannelBitmask"> Bitmask of enabled channels. Supports up to sizeof(int) * 8 channels. Channels start at 1 </param>
/// <returns> Vector containing the channel numbers of enabled channels </returns>
template <typename T> std::vector<int> ChannelManipulator<T>::getEnabledChannels(int enabledChannelBitmask)
{

	std::vector<int> enabledChannels;
	int firstBit = 1;
	for(int i=0; i<sizeof(enabledChannelBitmask*8); i++)
	{
		if(firstBit & enabledChannelBitmask)
			enabledChannels.push_back(i);
		enabledChannelBitmask = enabledChannelBitmask >> 1;
	}

	return enabledChannels;

}

/// <summary> Returns if this channel is enabled based on the input vector of enabled channels </summary>
/// <param name="channel"> The channel of interest </param>
/// <param name="enabledChannels"> Vector containing all enabled channels </param>
/// <returns> Bool indicating the channel is enabled(true), or disabled(false) </returns>
template <typename T> bool ChannelManipulator<T>::isChannelEnabled(int channel, const std::vector<int>& enabledChannels)
{

	if(std::find(enabledChannels.begin(), enabledChannels.end(), channel) != enabledChannels.end()) {
		return true;
	}

	return false;

}

/// <summary> Remove channels from a vector of enabled channels, renumbering enabled channels to reflect the removed channel </summary>
/// <param name="enabledChannels"> Ascending sorted vector containing all enabled channels </param>
/// <param name="channelsToRemove"> Ascending sorted vector containing all the channels to remove </param>
/// <returns> Reference to the enabledChannels vector, modified to reflect the new state </returns>
template <typename T> std::vector<int>& ChannelManipulator<T>::removeChannels(std::vector<int>& enabledChannels, std::vector<int> channelsToRemove)
{
	auto lastElement = std::set_difference(enabledChannels.begin(), enabledChannels.end(), channelsToRemove.begin(), channelsToRemove.end(),enabledChannels.begin());
	enabledChannels.resize(lastElement-enabledChannels.begin());  
	for(int& enabledChannel : enabledChannels)
	{
		int deletedChannelsBefore = static_cast<int>(std::count_if(channelsToRemove.begin(), channelsToRemove.end(),  [&](int val) { return (val < enabledChannel);} ));
		enabledChannel -= deletedChannelsBefore;
	}

	return enabledChannels;
}


/// <summary> Convert a vector of absolute channels into sequential channels. See getSequentialMemoryChannel()
///           used for a singular channel for a better description of absolute vs sequential 
/// </summary>
/// <param name="enabledChannels"> Vector containing all enabled channels </param>
/// <param name="absoluteChannels"> Vector containing the channels to convert </param>
/// <returns> Vector containing the converted sequential channel values </returns>
template <typename T> std::vector<int> ChannelManipulator<T>::getSequentialMemoryChannels(const std::vector<int>& enabledChannels, const std::vector<int>& absoluteChannels)
{
	std::vector<int> sequentialChannels;
	for(int absoluteChannel : absoluteChannels)
	{
		sequentialChannels.push_back(ChannelManipulator<T>::getSequentialMemoryChannel(absoluteChannel,enabledChannels));
	}
	return sequentialChannels;
}

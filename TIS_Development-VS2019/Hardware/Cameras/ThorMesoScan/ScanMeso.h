#pragma once
#include <vector>
#include "..\..\..\Common\ScanManager\Scan.h"
#include "..\..\..\Common\EnumString.h"

using namespace std;
class Rect_64f
{
public:
	double x;
	double y;
	double width;
	double height;

};
class Rect_32u
{
public:
	unsigned int x;
	unsigned int y;
	unsigned int width;
	unsigned int height;
};
class Point_64f
{
public:
	double X;
	double Y;
};
class Point_32u
{
public:
	unsigned int X;
	unsigned int Y;
};

enum ResUnit
{
	None = 0,
	Inch = 1,
	Centimeter = 2,
	Millimeter = 3,
	Micron = 4,
	Nanometer = 5,
	Picometer = 6
};

Begin_Enum_String( ResUnit )
{
	Enum_String( None );
	Enum_String( Inch );
	Enum_String( Centimeter );
	Enum_String( Millimeter );
	Enum_String( Micron );
	Enum_String( Nanometer );
	Enum_String( Picometer );
}End_Enum_String;

enum PixelType {
	//PixelType_INT8 = 0,
	PixelType_INT16 = 1,
	//PixelType_INT32 = 2,
	PixelType_UINT8 = 3,
	PixelType_UINT16 = 4,
	//PixelType_UINT32 = 5,
	//PixelType_FLOAT = 6,
	//PixelType_DOUBLE = 7,
	//PixelType_COMPLEX = 8,
	//PixelType_DOUBLE_COMPLEX = 9,
	//PixelType_BIT = 10
};

Begin_Enum_String( PixelType )
{
	Enum_String( PixelType_INT16 );
	Enum_String( PixelType_UINT8 );
	Enum_String( PixelType_UINT16 );
}End_Enum_String;

enum ShapeTypes
{
	ROI_Rectangle = 0,
	ROI_Ellipse,
	ROI_PolyLine,
	ROI_Polygon
};


class ROI
{
public:
	ROI() {}
	~ROI() {
		if (0 < Points.size())
		{
			for (size_t i = 0; i < Points.size(); i++)
			{
				delete Points.at(i);
			}
			Points.clear();
		}
	}
	ROI(const ROI& source)
	{
		ROIID = source.ROIID;
		Type = source.Type;
		Bound = source.Bound;
		vector<Point_64f*> ps = source.Points;
		for (vector<Point_64f*>::iterator iter = ps.begin(); iter != ps.end(); iter++)
		{
			Points.push_back(new Point_64f(**iter));
		}
	}
	long CalculateBoundary()
	{
		if (Points.size() < 3)
			return 0;
		vector<Point_64f*>::iterator iter = Points.begin();
		double xMax = (*iter)->X;
		double xMin = (*iter)->X;
		double yMax = (*iter)->Y;
		double yMin = (*iter)->Y;
		while (iter != Points.end())
		{
			xMax = max(xMax, (*iter)->X);
			xMin = min(xMin, (*iter)->X);
			yMax = max(yMax, (*iter)->Y);
			yMin = min(yMin, (*iter)->Y);
			++iter;
		}
		Bound.x = xMin;
		Bound.y = yMin;
		Bound.width = xMax - xMin;
		Bound.height = yMax - yMin;
		return 1;
	}
	unsigned int ROIID;
	ShapeTypes Type;
	vector<Point_64f*> Points;
	Rect_64f Bound;
};

class PowerBox
{
public:
	PowerBox() {}
	~PowerBox()
	{
		if (PowerROI != NULL)
			delete PowerROI;
	}
	PowerBox(const PowerBox& source)
	{
		StartZ = source.StartZ;
		EndZ = source.EndZ;
		PowerPercentage = source.PowerPercentage;
		PowerROI = new ROI(*source.PowerROI);
	}

	double StartZ;
	double EndZ;
	double PowerPercentage;
	ROI* PowerROI;
};
class PowerPoint
{
public:
	double ZPosition;
	double PowerPercentage;
};

class ScanArea
{
public:
	ScanArea() {}
	~ScanArea() {
		if (0 < PowerPoints.size())
		{
			for (size_t i = 0; i < PowerPoints.size(); i++)
			{
				delete PowerPoints.at(i);
			}
			PowerPoints.clear();
		}
		if (0 < PowerBoxs.size())
		{
			for (size_t i = 0; i < PowerBoxs.size(); i++)
			{
				delete PowerBoxs.at(i);
			}
			PowerBoxs.clear();
		}
	}
	ScanArea(const ScanArea& source)
	{
		ScanAreaID = source.ScanAreaID;
		SizeX = source.SizeX;
		SizeY = source.SizeY;
		SizeZ = source.SizeZ;
		SizeT = source.SizeT;
		SizeS = source.SizeS;
		PhysicalSizeX = source.PhysicalSizeX;
		PhysicalSizeY = source.PhysicalSizeY;
		PhysicalSizeZ = source.PhysicalSizeZ;
		PositionX = source.PositionX;
		PositionY = source.PositionY;
		PositionZ = source.PositionZ;
		vector<PowerPoint*> pps = source.PowerPoints;
		for (vector<PowerPoint*>::iterator iter = pps.begin(); iter != pps.end(); iter++)
		{
			PowerPoints.push_back(new PowerPoint(**iter));
		}
		vector<PowerBox*> pbs = source.PowerBoxs;

		for (vector<PowerBox*>::iterator iter = pbs.begin(); iter != pbs.end(); iter++)
		{
			PowerBoxs.push_back(new PowerBox(**iter));
		}
	}

	unsigned short ScanAreaID;
	unsigned int SizeX;
	unsigned int SizeY;
	unsigned short SizeZ;
	unsigned short SizeT;
	unsigned short SizeS;
	double PhysicalSizeX;
	double PhysicalSizeY;
	double PhysicalSizeZ;
	double PositionX;
	double PositionY;
	double PositionZ;
	vector<PowerPoint*> PowerPoints;
	vector<PowerBox*> PowerBoxs;
};
class Channel
{
public:
	unsigned short ChannelID;		//0-based id, no gap allowed, 0:1:channelCount, keep based on number of channels
	unsigned short ChannelRefID;	//reference index, used for actual enabled LSM channels, 0:A, 1:B, 2:C, 3:D
	char Name[_MAX_DIR];
private:

};
class ScanConfigure
{
public:
	ScanMode ScanMode;
	AverageMode AverageMode;
	unsigned short NumberOfAverageFrame;
	unsigned short StripLength;
	unsigned short IsLivingMode;
	unsigned short ChannelCount;
	double PhysicalFieldSize;
	double CurrentPower;
	int RemapShift;
	unsigned short CurrentT;
	bool IsEnableCurveCorrection;
};

class Scan
{
public:
	Scan() 
	{
		ScanID = 1;
		SignificantBits = 14;
		XPixelSize = YPixelSize = ZPixelSize = 0;
		TimeInterval = 0;
		ResUnit = ResUnit::Micron;
		TileWidth = TileHeight = 0;
		IPixelType = PixelType::PixelType_UINT16;
	}

	~Scan()
	{
		ClearChannels();

		if (0 < ScanAreas.size())
		{
			for (size_t i = 0; i < ScanAreas.size(); i++)
			{
				if (NULL != ScanAreas.at(i))
				{
					delete ScanAreas.at(i);
					ScanAreas.at(i) = NULL;
				}
			}
			ScanAreas.clear();
		}
	}

	Scan(const Scan& source)
	{
		ScanID = source.ScanID;
		SignificantBits = source.SignificantBits;
		XPixelSize = source.XPixelSize;
		YPixelSize = source.YPixelSize;
		ZPixelSize = source.ZPixelSize;
		TimeInterval = source.TimeInterval;
		ResUnit = source.ResUnit;
		TileWidth = source.TileWidth;
		TileHeight = source.TileHeight;
		IPixelType = source.IPixelType;
		ScanConfig = source.ScanConfig;
		vector<ScanArea*> sas = source.ScanAreas;
		for (vector<ScanArea*>::iterator iter = sas.begin(); iter != sas.end(); iter++)
		{
			ScanAreas.push_back(new ScanArea(**iter));
		}

		vector<Channel*> chs = source.Channels;
		for (vector<Channel*>::iterator iter = chs.begin(); iter != chs.end(); iter++)
		{
			Channels.push_back(new Channel(**iter));
		}
	}

	unsigned short ScanID;
	unsigned short SignificantBits;
	double XPixelSize;
	double YPixelSize;
	double ZPixelSize;
	double TimeInterval;
	ResUnit ResUnit;
	unsigned short TileWidth;
	unsigned short TileHeight;
	PixelType IPixelType;
	vector<ScanArea*> ScanAreas;
	vector<Channel*> Channels;
	ScanConfigure ScanConfig;
	std::string Name;

	unsigned short GetPixelBytes()
	{
		switch (IPixelType)
		{
		case PixelType_INT16:
			return sizeof(signed short);
		case PixelType_UINT8:
			return sizeof(unsigned char);
		case PixelType_UINT16:
		default:
			return sizeof(unsigned short);
		}
	};

	void ClearChannels()
	{
		if (0 < Channels.size())
		{
			for (size_t i = 0; i < Channels.size(); i++)
			{
				if (NULL != Channels.at(i))
				{
					delete Channels.at(i);
					Channels.at(i) = NULL;
				}
			}
			Channels.clear();
		}
	};

};

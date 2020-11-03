// CPP_Library.h

#include "CImg.h"

#pragma once

using namespace System;
using namespace System::Threading;
using namespace System::Drawing;
using namespace System::Drawing::Imaging;
using namespace cimg_library;

namespace CPP_Library 
{
	public ref class DataClass
	{
	private:

	public:
		short *data;
		int width, height;
		double pixelSize_um,gridSpacing_um;
		double mag, angle;

		DataClass(short *Data, int Width, int Height, double PixelSize_um, double GridSpacing_um)
		{
			data = Data;
			width = Width;
			height = Height;
			pixelSize_um = PixelSize_um;
			gridSpacing_um = GridSpacing_um;
		}
	};

	delegate double AngleMagDelegate(DataClass ^Data);
	public ref class CImg_Wrapper
	{
	private:
		static int GetBytesPerPixel(Bitmap^ BMP)
		{
			int dx = 3;
			if(BMP->PixelFormat == PixelFormat::Format32bppArgb)
				dx = 4;
			else if(BMP->PixelFormat == PixelFormat::Format8bppIndexed)
				dx = 1;

			return dx;
		}

		static void Hough_Angle_Detection(Object ^DataObj)
		{
			DataClass ^Data = (DataClass^)DataObj;
			CImg<short> src(Data->data,Data->width,Data->height);
			//src = src.crop(Data->width/4,Data->height/4,3*Data->width/4,3*Data->height/4);
			src = src.crop(0,Data->height/4,Data->width,3*Data->height/4);
			CImg<> vote(src.width(),src.height(),1,1,0), img = src.get_norm().normalize(0,255);
			double alpha = 1.5;
			double sigma = 0.5;
			double rhomax = std::sqrt((double)(src.width()*src.width()+src.height()*src.height()))/2;
			double thetamax =2*cimg::PI;
			double minTheta = 7.0*cimg::PI/8.0;
			double maxTheta = 9.0*cimg::PI/8.0;
			double thetaRange = maxTheta-minTheta;

			CImgList<> grad = img.get_gradient();
			cimglist_for(grad,l) 
				grad[l].blur((float)alpha);
			vote.fill(0);
			cimg_forXY(img,x,y) 
			{
				const double
				  X = (double)x - img.width()/2,
				  Y = (double)y - img.height()/2,
				  gx = grad[0](x,y),
				  gy = grad[1](x,y);
				double
				  theta = std::atan2(gy,gx),
				  rho   = std::sqrt(X*X+Y*Y)*std::cos(std::atan2(Y,X)-theta);
				if (rho<0) 
				{ 
					rho=-rho; theta+=cimg::PI; 
				}
				theta = cimg::mod(theta,thetamax);

				if(theta > minTheta && theta < maxTheta)
				{
					vote((int)((theta-minTheta)*vote.width()/thetaRange),(int)(rho*vote.height()/rhomax))+=(float)std::sqrt(gx*gx+gy*gy);
				}
			}
			//vote.blur((float)sigma);

			array<float>^ sumCols = gcnew array<float>(vote.width());

			float max = float::MinValue;
			int mindex = -1;
			int width = img.width();
			int height = img.height();
			for(int c = width/2-width/8; c < width/2+width/8; c++)
			{
				sumCols[c] = vote.get_column(c).sum();
				if(sumCols[c] > max)
				{
					max = sumCols[c];
					mindex = c;
				}
			}

			//Refine search in Rows based on center of mass around previous mindex
			double totalMass = 0;
			double massColProd = 0;
			int searchRadius = width/10;
			for(int r = 0; r < height; r++)
			{
				CImg<> row = vote.get_row(r);
				for(int c = mindex-searchRadius; c < mindex+searchRadius; c++)
				{
					double mass = row[c];
					totalMass += mass;
					massColProd += mass*c;
				}
			}

			mindex = (int)(massColProd/totalMass);

			Data->angle = (180.0/cimg::PI)*(thetaRange*(double)(mindex-width/2-2)/(double)width);
		}

		static void Magnification(Object ^DataObj)
		{
			DataClass ^Data = (DataClass^)DataObj;

			CImg<short> src(Data->data,Data->width,Data->height);
			src = src.crop(Data->width/4,Data->height/4,3*Data->width/4,3*Data->height/4);
			int width = src.width();
			int height = src.height();

			//Need to change threshold to dynamic
			CImg<> img = (255*((255-src.get_norm().normalize(0,255)).threshold(128))).distance(0);

			CImg<> rowSum = img.get_row(0);
			for(int r = 1; r < height; r++)
			{
				rowSum += img.get_row(r);
			}

			float max = rowSum.max();

			CImg<> mask = rowSum.get_threshold(max/2);
			System::Collections::Generic::List<float>^ peaks = gcnew System::Collections::Generic::List<float>();

			for(int i = 0; i < width; i++)
			{
				while(mask[i] == 0){i++;}

				max = float::MinValue;
				int index = 0;
				while(mask[i] == 1)
				{
					if(rowSum[i] > max)
					{
						max = rowSum[i];
						index = i;
					}

					i++;
				}

				peaks->Add(index);
			}

			int cnt = 0;
			float sum = 0;
			for(int i = 0; i < peaks->Count-1; i++)
			{
				if(peaks[i+1] != 0)
				{
					sum += peaks[i+1]-peaks[i];
					cnt++;
				}
			}
			sum /= (float)cnt;

			//PixelSize_um, double GridSpacing_um
			double mag = sum*Data->pixelSize_um/Data->gridSpacing_um;

			Data->mag = mag;
		}
	public:
		static array<short>^ Bitmap2UShort(Bitmap ^BMP, int ^%Width, int ^%Height)
		{
			int dx = GetBytesPerPixel(BMP);

			int width = BMP->Width;
			int height = BMP->Height;
			BitmapData^ bmpData = BMP->LockBits(System::Drawing::Rectangle(0,0,width,height),ImageLockMode::ReadOnly,BMP->PixelFormat);
			byte* pData = (byte*)bmpData->Scan0.ToPointer();
			int stride = bmpData->Stride;

			array<short> ^output = gcnew array<short>(width*height);

			for(int r = 0; r < height; r++)
			{
				int offset = r * stride;
				for(int c = 0; c < width; c++)
				{
					output[r*width + c] = *(pData + offset + c * dx + 0) * 32;
				}
			}

			BMP->UnlockBits(bmpData);

			Width = width;
			Height = height;

			return output;
		}

		static array<double>^ AngleMagCalculation(short *Data, int Width, int Height, double PixelSize_um, double GridSpacing_um)
		{
			array<double> ^output = gcnew array<double>(2);

			DataClass ^data = gcnew DataClass(Data,Width,Height,PixelSize_um,GridSpacing_um);
			System::Threading::Thread ^threadAngle = gcnew System::Threading::Thread(gcnew ParameterizedThreadStart(Hough_Angle_Detection));
			System::Threading::Thread ^threadMag = gcnew System::Threading::Thread(gcnew ParameterizedThreadStart(Magnification));

			threadAngle->Start(data);
			threadMag->Start(data);

			threadAngle->Join();
			threadMag->Join();

			/*Hough_Angle_Detection(data);
			Magnification(data);*/

			output[0] = data->angle;
			output[1] = data->mag;

			return output;
		}

		static double MagCalculation(short *Data, int Width, int Height, double PixelSize_um, double GridSpacing_um)
		{
			DataClass ^data = gcnew DataClass(Data,Width,Height,PixelSize_um,GridSpacing_um);
			Magnification(data);

			return data->mag;
		}
	};
}

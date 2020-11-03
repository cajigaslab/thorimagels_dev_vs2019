#include "stdafx.h"
#include "CppUnitTest.h"
#include "..\..\GenericImage.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTest1
{		

	template <typename T> std::vector<T> getIncrementingBuffer(T startValue, int length)
	{
		std::vector<T> incrementingBuffer(length);
		T curVal = startValue;
		for(int i=0; i<length; i++)
		{
			incrementingBuffer[i] = curVal;
			++curVal;
		}
		return incrementingBuffer;
	}

	const unsigned short FILL_VAL = 5555;

	template <typename T> bool verifyCopy(GenericImage<T>& from, GenericImage<T>& to)
	{
		to.copyFrom(from);

		auto fromIt=from.begin();
		for(T& toVal : to)
		{
			if(toVal != *fromIt)
				return false;
			++fromIt;
		}
		return true;

	}

	void verifyCopyFunctions()
	{
		int width,height,depth,channels,m;
		width = height = depth = channels = m = 10;
		int size = width*height*depth*channels*m;
		std::vector<unsigned short> incrementingBuffer = getIncrementingBuffer<unsigned short>(0,size); 

		GenericImage<unsigned short> conSource(width,height,depth,channels,m,GenericImage<unsigned short>::CONTIGUOUS_CHANNEL);
		conSource.setMemoryBuffer(&(incrementingBuffer[0]));

		//=== CON CON ===
		InternallyStoredImage<unsigned short> conconDest(width,height,depth,channels,m,GenericImage<unsigned short>::CONTIGUOUS_CHANNEL);
		Assert::IsTrue(verifyCopy(conSource,conconDest), L"CON to CON copy failed");

		//=== CON INT ===
		InternallyStoredImage<unsigned short> conintDest(width,height,depth,channels,m,GenericImage<unsigned short>::INTERLACED_CHANNEL);
		Assert::IsTrue(verifyCopy(conSource,conintDest), L"CON to INT copy failed");

		InternallyStoredImage<unsigned short> intSource(width,height,depth,channels,m,GenericImage<unsigned short>::INTERLACED_CHANNEL);
		intSource.copyFrom(conSource);

		//=== INT INT ===
		InternallyStoredImage<unsigned short> intintDest(width,height,depth,channels,m,GenericImage<unsigned short>::INTERLACED_CHANNEL);
		Assert::IsTrue(verifyCopy(intSource,intintDest), L"INT to INT copy failed");

		//=== INT CON ===
		InternallyStoredImage<unsigned short> intconDest(width,height,depth,channels,m,GenericImage<unsigned short>::CONTIGUOUS_CHANNEL);
		Assert::IsTrue(verifyCopy(intSource,intconDest), L"INT to CON copy failed");

		
	}


		




	TEST_CLASS(UnitTest1)
	{
	public:

		TEST_METHOD(IterateContiguousContainer)
		{
			int width,height,depth = 10;
			width = height = depth = 10;
			int channels = 4;
			int m = 4;
			InternallyStoredImage<unsigned short> testImage(width,height,depth,channels,m,GenericImage<unsigned short>::CONTIGUOUS_CHANNEL);
			testImage.fillWith(FILL_VAL);

			int count=0;
			for(unsigned short& pixel : testImage)
			{
				count++;
			}
			Assert::AreEqual(count,width*height*depth*channels*m,L"NumPixels does not equal count");
			Assert::AreEqual(width*height*depth*channels*m,testImage.getSizeInPixels(),L"Num Pixels does not equal expected Num");
			Assert::IsTrue(testImage.getSizeInBytes() == sizeof(unsigned short) * testImage.getSizeInPixels(), L"Num Bytes does not equal expected amount");

		}

		TEST_METHOD(IterateInterlacedContainer)
		{
			int width,height,depth = 10;
			width = height = depth = 10;
			int channels = 4;
			int m = 4;
			InternallyStoredImage<unsigned short> testImage(width,height,depth,channels,m,GenericImage<unsigned short>::INTERLACED_CHANNEL);
			testImage.fillWith(FILL_VAL);

			int count=0;
			for(unsigned short& pixel : testImage)
			{
				count++;
			}
			Assert::AreEqual(count,width*height*depth*channels*m,L"NumPixels does not equal count");
			Assert::AreEqual(width*height*depth*channels*m,testImage.getSizeInPixels(),L"Num Pixels does not equal expected Num");
			Assert::IsTrue(testImage.getSizeInBytes() == sizeof(unsigned short) * testImage.getSizeInPixels(), L"Num Bytes does not equal expected amount");

		}

		TEST_METHOD(CopyFromImageToImage)
		{
			verifyCopyFunctions();
		}

		TEST_METHOD(TestOperatorOverloads)
		{
			int width,height,depth,channels,m;
			width = height = depth = channels = m = 10;
			int size = width*height*depth*channels*m;
			std::vector<unsigned short> incrementingBuffer = getIncrementingBuffer<unsigned short>(0,size); 

			GenericImage<unsigned short> conSource(width,height,depth,channels,m,GenericImage<unsigned short>::CONTIGUOUS_CHANNEL);
			conSource.setMemoryBuffer(&(incrementingBuffer[0]));

			Assert::IsTrue(conSource == conSource);
			Assert::IsFalse(conSource != conSource);

		}

		TEST_METHOD(AssignementOperatorsDifferentDimensions)
		{
			int width,height,depth,channels,m;
			width = height = depth = channels = m = 10;
			int size = width*height*depth*channels*m;

			int width2,height2,depth2,channels2,m2;
			width2 = height2 = depth2 = channels2 = m2 = width+1;
			int size2 = width2*height2*depth2*channels2*m2;

			//=== Gen Im ===
			std::vector<unsigned short> incrementingBuffer = getIncrementingBuffer<unsigned short>(0,size); 
			GenericImage<unsigned short> source(width,height,depth,channels,m,GenericImage<unsigned short>::CONTIGUOUS_CHANNEL);
			source.setMemoryBuffer(&(incrementingBuffer[0]));
			GenericImage<unsigned short> dest(width2,height2,depth2,channels2,m2,GenericImage<unsigned short>::CONTIGUOUS_CHANNEL);
			dest = source;
			dest.fillWith(500);


			//=== INT INT ===
			InternallyStoredImage<unsigned short> source2(width,height,depth,channels,m,GenericImage<unsigned short>::CONTIGUOUS_CHANNEL);
			InternallyStoredImage<unsigned short> dest2(width2,height2,depth2,channels2,m,GenericImage<unsigned short>::CONTIGUOUS_CHANNEL);
			dest2 = source2;
			dest2.fillWith(500);

		}

		TEST_METHOD(AssignementOperators)
		{
			int width,height,depth,channels,m;
			width = height = depth = channels = m = 10;
			int size = width*height*depth*channels*m;
			std::vector<unsigned short> incrementingBuffer = getIncrementingBuffer<unsigned short>(0,size); 
			std::vector<unsigned short> incrementingBuffer2 = getIncrementingBuffer<unsigned short>(100,size); 

			GenericImage<unsigned short> conSource(width,height,depth,channels,m,GenericImage<unsigned short>::CONTIGUOUS_CHANNEL);
			GenericImage<unsigned short> intSource(width,height,depth,channels,m,GenericImage<unsigned short>::INTERLACED_CHANNEL);
			conSource.setMemoryBuffer(&(incrementingBuffer[0]));
			intSource.setMemoryBuffer(&(incrementingBuffer[0]));
			InternallyStoredImage<unsigned short> conInSrc(width,height,depth,channels,m,GenericImage<unsigned short>::CONTIGUOUS_CHANNEL);
			InternallyStoredImage<unsigned short> intInSrc(width,height,depth,channels,m,GenericImage<unsigned short>::INTERLACED_CHANNEL);
			conInSrc.copyFrom(conSource);
			intInSrc.copyFrom(conSource);

			//=== CON CON ===
			GenericImage<unsigned short> con = conSource;
			Assert::IsTrue(con == conSource, L"Con first");
			conSource.fillWith(0);
			Assert::IsTrue(con == conSource, L"Con modify original");
			conSource.setMemoryBuffer(&(incrementingBuffer2[0]));
			Assert::IsFalse(con == conSource, L"Con modify original");

			//=== INT INT ===
			GenericImage<unsigned short> intIm = intSource;
			Assert::IsTrue(intIm == intSource, L"Int first");
			intSource.fillWith(0);
			Assert::IsTrue(intIm == intSource, L"Int modify original");
			intSource.setMemoryBuffer(&(incrementingBuffer2[0]));
			Assert::IsFalse(intIm == intSource, L"Int modify original");

			//=== CON CON ===
			InternallyStoredImage<unsigned short> con2 = conInSrc;
			Assert::IsTrue(con2 == conInSrc, L"Internal Con");
			conInSrc.fillWith(0);
			Assert::IsFalse(con2 == conInSrc, L"Internal Con modigy orig");

			//=== INT INT ===
			InternallyStoredImage<unsigned short> int2 = intInSrc;
			Assert::IsTrue(int2 == intInSrc, L"Internal Int");
			intInSrc.fillWith(0);
			Assert::IsFalse(int2 == intInSrc, L"Internal Int modify orig");

		}
		
		TEST_METHOD(FillContiguousAllEnabled)
		{
			InternallyStoredImage<unsigned short> testImage(10,10,10,10,10,GenericImage<unsigned short>::CONTIGUOUS_CHANNEL);
			testImage.fillWith(FILL_VAL);

			for(unsigned short& pixel : testImage)
			{
				Assert::AreEqual(pixel,FILL_VAL, L"Fill Filed:");
			}


		}

		TEST_METHOD(IteratorCopyBug)
		{
			InternallyStoredImage<unsigned short> testImage(10,10,10,10,10,GenericImage<unsigned short>::CONTIGUOUS_CHANNEL);
			testImage.fillWith(FILL_VAL);

			auto minMaxValue = std::minmax_element(testImage.begin(), testImage.end());
			int minValue = *(minMaxValue.first);
			int maxValue = *(minMaxValue.second);
		}

		TEST_METHOD(AverageContiguousMemoryImages)
		{
			InternallyStoredImage<unsigned short> source1(10,10,10,10,10,GenericImage<unsigned short>::CONTIGUOUS_CHANNEL);
			InternallyStoredImage<unsigned short> source2(10,10,10,10,10,GenericImage<unsigned short>::CONTIGUOUS_CHANNEL);
			InternallyStoredImage<unsigned short> source3(10,10,10,10,10,GenericImage<unsigned short>::CONTIGUOUS_CHANNEL);

			source1.fillWith(0);
			source2.fillWith(100);
			source3.fillWith(200);

			std::vector<GenericImage<unsigned short>*> imagesToAverage;
			imagesToAverage.push_back(&source2);
			imagesToAverage.push_back(&source3);

			source1.averageWith(imagesToAverage);

			for(unsigned short& pixel : source1)
			{
				Assert::AreEqual(pixel,(unsigned short)100, L"Average Failed");
			}



			InternallyStoredImage<unsigned short> dest(10,10,10,10,10,GenericImage<unsigned short>::CONTIGUOUS_CHANNEL);
			source1.fillWith(0);
			source1.averageWith(imagesToAverage, dest);

			for(unsigned short& pixel : dest)
			{
				Assert::AreEqual(pixel,(unsigned short)100, L"Average And Store Failed");
			}



		}

		TEST_METHOD(AverageInterlacedMemoryImages)
		{
			InternallyStoredImage<unsigned short> source1(10,10,10,10,10,GenericImage<unsigned short>::INTERLACED_CHANNEL);
			InternallyStoredImage<unsigned short> source2(10,10,10,10,10,GenericImage<unsigned short>::INTERLACED_CHANNEL);
			InternallyStoredImage<unsigned short> source3(10,10,10,10,10,GenericImage<unsigned short>::INTERLACED_CHANNEL);

			source1.fillWith(0);
			source2.fillWith(100);
			source3.fillWith(200);

			std::vector<GenericImage<unsigned short>*> imagesToAverage;
			imagesToAverage.push_back(&source2);
			imagesToAverage.push_back(&source3);

			source1.averageWith(imagesToAverage);

			for(unsigned short& pixel : source1)
			{
				Assert::AreEqual(pixel,(unsigned short)100, L"Average Failed");
			}



			InternallyStoredImage<unsigned short> dest(10,10,10,10,10,GenericImage<unsigned short>::INTERLACED_CHANNEL);
			source1.fillWith(0);
			source1.averageWith(imagesToAverage, dest);

			for(unsigned short& pixel : dest)
			{
				Assert::AreEqual(pixel,(unsigned short)100, L"Average And Store Failed");
			}



		}

		TEST_METHOD(AverageMixedMemoryImages)
		{
			InternallyStoredImage<unsigned short> source1(10,10,10,10,10,GenericImage<unsigned short>::INTERLACED_CHANNEL);
			InternallyStoredImage<unsigned short> source2(10,10,10,10,10,GenericImage<unsigned short>::CONTIGUOUS_CHANNEL);
			InternallyStoredImage<unsigned short> source3(10,10,10,10,10,GenericImage<unsigned short>::INTERLACED_CHANNEL);

			source1.fillWith(0);
			source2.fillWith(100);
			source3.fillWith(200);

			std::vector<GenericImage<unsigned short>*> imagesToAverage;
			imagesToAverage.push_back(&source2);
			imagesToAverage.push_back(&source3);

			source1.averageWith(imagesToAverage);

			for(unsigned short& pixel : source1)
			{
				Assert::AreEqual(pixel,(unsigned short)100, L"Average Failed");
			}



			InternallyStoredImage<unsigned short> dest(10,10,10,10,10,GenericImage<unsigned short>::CONTIGUOUS_CHANNEL);
			source1.fillWith(0);
			source1.averageWith(imagesToAverage, dest);

			for(unsigned short& pixel : dest)
			{
				Assert::AreEqual(pixel,(unsigned short)100, L"Average And Store Failed");
			}



		}



		

		TEST_METHOD(FillInterlacedAllEnabled)
		{
			InternallyStoredImage<unsigned short> testImage(10,10,10,10,10,GenericImage<unsigned short>::INTERLACED_CHANNEL);
			testImage.fillWith(FILL_VAL);

			for(unsigned short& pixel : testImage)
			{
				Assert::AreEqual(pixel,FILL_VAL, L"Fill Filed:");
			}


		}

		TEST_METHOD(SelectivelyEnabledChannels)
		{
			int width,height,depth,channels,m;
			width = height = depth = channels = m = 10;
			int size = width*height*depth*channels*m;

			for(int i=1; i<4; i++)
			{
			for(int j=1; j<4; j++)
			{
				std::vector<int> enabledChannels = ChannelManipulator<unsigned short>::getEnabledChannels(i);
				std::vector<int> enabledSourceChannels = ChannelManipulator<unsigned short>::getEnabledChannels(j);

				std::vector<unsigned short> incrementingBuffer = getIncrementingBuffer<unsigned short>(0,size); 
				GenericImage<unsigned short> conSource(width,height,depth,channels,m,GenericImage<unsigned short>::CONTIGUOUS_CHANNEL,enabledSourceChannels);
				GenericImage<unsigned short> intSource(width,height,depth,channels,m,GenericImage<unsigned short>::INTERLACED_CHANNEL,enabledSourceChannels);
				conSource.setMemoryBuffer(&(incrementingBuffer[0]));
				intSource.setMemoryBuffer(&(incrementingBuffer[0]));

				InternallyStoredImage<unsigned short> testImageCon(width,height,depth,channels,m,GenericImage<unsigned short>::CONTIGUOUS_CHANNEL,enabledChannels);
				InternallyStoredImage<unsigned short> testImageInt(width,height,depth,channels,m,GenericImage<unsigned short>::INTERLACED_CHANNEL,enabledChannels);
				InternallyStoredImage<unsigned short> testImageCon2(width,height,depth,channels,m,GenericImage<unsigned short>::CONTIGUOUS_CHANNEL,enabledChannels);
				InternallyStoredImage<unsigned short> testImageInt2(width,height,depth,channels,m,GenericImage<unsigned short>::INTERLACED_CHANNEL,enabledChannels);

				testImageCon.copyFrom(conSource);
				testImageInt.copyFrom(intSource);
				testImageCon2.copyFrom(intSource);
				testImageInt2.copyFrom(conSource);

				testImageCon.printImage();
				testImageInt.printImage();
				testImageCon2.printImage();
				testImageInt2.printImage();

				for(int x=0; x<width; x++)
				{
					for(int y=0; y<height; y++)
					{
						for(int z=0; z<depth; z++)
						{
							for(int c=0; c<channels; c++)
							{
								for(int nm=0; nm<m; nm++)
								{
									if(testImageCon.isChannelEnabled(c) && conSource.isChannelEnabled(c))
									{
										if(y==1)
											int sdf=13;
										std::wstringstream ss1, ss2, ss3, ss4;
										ss1 << L"ConCon Failed " << x << "," << y << "," << z << "," << c << "," << nm << "  orig: " << (int)conSource.getVal(x,y,z,c,nm) << "  copy: " << (int)testImageCon.getVal(x,y,z,c,nm);
										ss2 << L"IntInt Failed " << x << "," << y << "," << z << "," << c << "," << nm << "  orig: " << (int)intSource.getVal(x,y,z,c,nm) << "  copy: " << (int)testImageInt.getVal(x,y,z,c,nm);
										ss3 << L"ConInt Failed " << x << "," << y << "," << z << "," << c << "," << nm << "  orig: " << (int)intSource.getVal(x,y,z,c,nm) << "  copy: " << (int)testImageCon2.getVal(x,y,z,c,nm);
										ss4 << L"IntCon Failed " << x << "," << y << "," << z << "," << c << "," << nm << "  orig: " << (int)conSource.getVal(x,y,z,c,nm) << "  copy: " << (int)testImageInt2.getVal(x,y,z,c,nm);
										std::wstring s1 = ss1.str();
										std::wstring s2 = ss2.str();
										std::wstring s3 = ss3.str();
										std::wstring s4 = ss4.str();

										Assert::AreEqual(testImageCon.getVal(x,y,z,c,nm), conSource.getVal(x,y,z,c,nm), s1.c_str());
										Assert::AreEqual(testImageInt.getVal(x,y,z,c,nm), intSource.getVal(x,y,z,c,nm), s2.c_str());
										Assert::AreEqual(testImageCon2.getVal(x,y,z,c,nm), intSource.getVal(x,y,z,c,nm), s3.c_str());
										Assert::AreEqual(testImageInt2.getVal(x,y,z,c,nm), conSource.getVal(x,y,z,c,nm), s4.c_str());
									}
								}
							}
						}
					}
				}
			}
			}
		}

	};
}
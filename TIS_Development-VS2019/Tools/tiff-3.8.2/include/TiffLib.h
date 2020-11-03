
//TiffLib.h
#include "..\..\..\Common\PDLL\pdll.h"
//dll wrapper class using the virtual class

typedef	uint32 ttag_t;		/* directory tag */
typedef	uint16 tdir_t;		/* directory index */
typedef	uint16 tsample_t;	/* sample number */
typedef	uint32 tstrip_t;	/* strip number */
typedef uint32 ttile_t;		/* tile number */
typedef	int32 tsize_t;		/* i/o size in bytes */
typedef	void* tdata_t;		/* image data ref */
typedef	uint32 toff_t;		/* file offset */

class TiffLib
{
public:

	virtual TIFF* TIFFOpenW(const wchar_t*, const char*)  = 0;	
	virtual int TIFFSetField(TIFF*, ttag_t, int)  = 0;
	virtual int TIFFVSetField(TIFF*, ttag_t, va_list)  = 0;
	virtual int TIFFGetField(TIFF*, ttag_t, int*)  = 0;
	virtual tsize_t TIFFScanlineSize(TIFF*)  = 0;
	virtual tdata_t _TIFFmalloc(tsize_t)  = 0;
	virtual uint32 TIFFDefaultStripSize(TIFF*, uint32)  = 0;
	virtual int TIFFWriteScanline(TIFF*, tdata_t, uint32, tsample_t)  = 0;
	virtual void TIFFClose(TIFF*)  = 0;
	virtual void _TIFFfree(tdata_t)  = 0;
	virtual int TIFFStripSize(TIFF*) = 0;
	virtual int TIFFNumberOfStrips(TIFF*) = 0;
	virtual int TIFFReadEncodedStrip(TIFF*, tstrip_t, tdata_t, tsize_t)=0;
	virtual tsize_t TIFFReadRawStrip(TIFF*, tstrip_t , tdata_t , tsize_t)=0;
	virtual tsize_t TIFFWriteTile(TIFF*, tdata_t, uint32, uint32, uint32, tsample_t)=0;
	virtual tsize_t TIFFReadTile(TIFF*, tdata_t, uint32, uint32, uint32, tsample_t)=0;
	virtual tsize_t TIFFNumberOfTiles(TIFF*)=0;
	virtual tsize_t TIFFTileSize(TIFF*)=0;
	virtual tsize_t TIFFFlush(TIFF*)=0;
	virtual tsize_t TIFFWriteDirectory(TIFF*)=0;

};

class TiffLibDll : public PDLL, TiffLib
{
#pragma warning(push)
#pragma warning(disable:26495)
#pragma warning(disable:26444)
	//call the macro and pass your class name
	DECLARE_CLASS(TiffLibDll)
#pragma warning(pop)

	DECLARE_FUNCTION2(TIFF*,TIFFOpenW, const wchar_t*, const char*);
	
	DECLARE_FUNCTION3(int,TIFFSetField, TIFF*, ttag_t, int);
	DECLARE_FUNCTION3(int,TIFFVSetField, TIFF*, ttag_t, va_list);
	DECLARE_FUNCTION3(int,TIFFGetField, TIFF*, ttag_t, int*);
	DECLARE_FUNCTION1(tsize_t, TIFFScanlineSize, TIFF*);
	DECLARE_FUNCTION1(tdata_t, _TIFFmalloc, tsize_t);
	DECLARE_FUNCTION2(uint32, TIFFDefaultStripSize, TIFF*, uint32);
	DECLARE_FUNCTION4(int, TIFFWriteScanline, TIFF*, tdata_t, uint32, tsample_t);
	DECLARE_FUNCTION1(void, TIFFClose, TIFF*);
	DECLARE_FUNCTION1(void, _TIFFfree, tdata_t);
	DECLARE_FUNCTION1(int, TIFFStripSize, TIFF*);
	DECLARE_FUNCTION1(int, TIFFNumberOfStrips, TIFF*);
	DECLARE_FUNCTION4(int, TIFFReadEncodedStrip, TIFF*, tstrip_t, tdata_t, tsize_t);
	DECLARE_FUNCTION4(int, TIFFReadRawStrip, TIFF*, tstrip_t, tdata_t, tsize_t);
	DECLARE_FUNCTION6(tsize_t, TIFFWriteTile, TIFF*, tdata_t, uint32, uint32, uint32, tsample_t);
	DECLARE_FUNCTION6(tsize_t, TIFFReadTile, TIFF*, tdata_t, uint32, uint32, uint32, tsample_t);
	DECLARE_FUNCTION1(tsize_t, TIFFNumberOfTiles, TIFF*);
	DECLARE_FUNCTION1(tsize_t, TIFFTileSize, TIFF*);
	DECLARE_FUNCTION1(tsize_t, TIFFFlush, TIFF*);
	DECLARE_FUNCTION1(tsize_t, TIFFWriteDirectory, TIFF*);

};


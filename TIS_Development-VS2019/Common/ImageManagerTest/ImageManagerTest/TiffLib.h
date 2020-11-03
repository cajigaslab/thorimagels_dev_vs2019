
//TiffLib.h
#include "..\..\PDLL\pdll.h"
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
	virtual tsize_t TIFFScanlineSize(TIFF*)  = 0;
	virtual tdata_t _TIFFmalloc(tsize_t)  = 0;
	virtual uint32 TIFFDefaultStripSize(TIFF*, uint32)  = 0;
	virtual int TIFFWriteScanline(TIFF*, tdata_t, uint32, tsample_t)  = 0;
	virtual void TIFFClose(TIFF*)  = 0;
	virtual void _TIFFfree(tdata_t)  = 0;
};

class TiffLibDll : public PDLL, TiffLib
{
	//call the macro and pass your class name
	DECLARE_CLASS(TiffLibDll)
	//use DECLARE_FUNCTION4 since this function has 4 parameters

	DECLARE_FUNCTION2(TIFF*,TIFFOpenW, const wchar_t*, const char*);
	
	DECLARE_FUNCTION3(int,TIFFSetField, TIFF*, ttag_t, int);
	DECLARE_FUNCTION1(tsize_t, TIFFScanlineSize, TIFF*);
	DECLARE_FUNCTION1(tdata_t, _TIFFmalloc, tsize_t);
	DECLARE_FUNCTION2(uint32, TIFFDefaultStripSize, TIFF*, uint32);
	DECLARE_FUNCTION4(int, TIFFWriteScanline, TIFF*, tdata_t, uint32, tsample_t);
	DECLARE_FUNCTION1(void, TIFFClose, TIFF*);
	DECLARE_FUNCTION1(void, _TIFFfree, tdata_t);
	
};


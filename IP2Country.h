// [TPT] - IP Country

#pragma once
#include "libGeoIP/GeoIP.h"
#include "libGeoIP/GeoIPCity.h"
#include "CxImage/xImage.h"

class CIP2Country
{
public:
	CIP2Country();
	~CIP2Country();	
	CString GetCountryFromIP(LPCTSTR fullUserIP);
	CString GetCountryISOFromIP(LPCTSTR fullUserIP);
	int Compare(CString country1, CString country2);
	CxImage GetCountryFlag(LPCTSTR fullUserIP);
private:	
	GeoIP * gi;
	GeoIPDBTypes db_loaded;
	CRBMap<CString, CxImage> country_flags;
	HINSTANCE _hCountryFlagDll;	
	void LoadFromFile();
};
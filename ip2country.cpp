// [TPT] - IP Country

#include "stdafx.h"
#include "emule.h"
#include "IP2Country.h"
#include "otherfunctions.h"
#include "Preferences.h"
#include "log.h"
#include <flag/resource.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// N/A flag is the first Res, so it should at index zero
#define NO_FLAG 0

CIP2Country::CIP2Country()
{
	gi = NULL;
	_hCountryFlagDll = NULL;
	LoadFromFile();
	
	// Load dll flag	
	if(thePrefs.GetWindowsVersion() == _WINVER_XP_)
	{
		_hCountryFlagDll = LoadLibrary(thePrefs.GetConfigDir()+ _T("countryflag32.dll"));
		if (_hCountryFlagDll == NULL) 	
			AddLogLine(false, GetResString(IDS_COUNTRYFLAG32_FAILED));
	}
	// if not countryflag32.dll or not windows xp
	if (_hCountryFlagDll == NULL)
		_hCountryFlagDll = LoadLibrary(thePrefs.GetConfigDir()+ _T("countryflag.dll")); 
	
	if (_hCountryFlagDll == NULL) 	
		AddLogLine(false, GetResString(IDS_COUNTRYFLAG_FAILED));
	else		
		AddLogLine(false, GetResString(IDS_COUNTRYFLAG_LOADED));			
}

CIP2Country::~CIP2Country()
{
	if (gi != NULL)
		GeoIP_delete(gi);

	free(GeoIPDBFileName);

	if (country_flags.IsEmpty() != true)
	{
		POSITION pos = country_flags.GetHeadPosition();
		while(pos != NULL)
		{
			CString version;
			CxImage xImage;
			country_flags.GetNextAssoc(pos, version, xImage);
			xImage.Destroy();
		}
		country_flags.RemoveAll();
	}
	
	// Release dll flag
	if(::FreeLibrary(_hCountryFlagDll) == TRUE)
		_hCountryFlagDll = NULL;
}

void CIP2Country::LoadFromFile()
{	
	USES_CONVERSION;
	char* configDir = nstrdup(CT2A(thePrefs.GetConfigDir()));
	
	// load CITY IP database	
	gi = GeoIP_open_type(GEOIP_CITY_EDITION_REV1, GEOIP_STANDARD, configDir);
	if (gi != NULL) 
	{
		AddLogLine(false, GetResString(IDS_GEOIP_CITY_LOADED));	
		AddLogLine(false, GetResString(IDS_GEOIP_CITY_INFO));	
		db_loaded = GEOIP_CITY_EDITION_REV1;
		delete[] configDir;
		return;
	}

	// load REGION IP database	
	gi = GeoIP_open_type(GEOIP_REGION_EDITION_REV1, GEOIP_STANDARD, configDir);
	if (gi != NULL) 
	{
		AddLogLine(false, GetResString(IDS_GEOIP_REGION_LOADED));
		AddLogLine(false, GetResString(IDS_GEOIP_REGION_INFO));
		db_loaded = GEOIP_REGION_EDITION_REV1;
		delete[] configDir;
		return;
	}

	// load COUNTRY IP database	
	gi = GeoIP_open_type(GEOIP_COUNTRY_EDITION, GEOIP_STANDARD, configDir);
	if (gi != NULL) 
	{
		AddLogLine(false, GetResString(IDS_GEOIP_COUNTRY_LOADED));
		AddLogLine(false, GetResString(IDS_GEOIP_COUNTRY_INFO));
		AddLogLine(false, GetResString(IDS_GEOIP_COUNTRY_INFO_DL));
		db_loaded = GEOIP_COUNTRY_EDITION;
		delete[] configDir;
		return;
	}
	
	AddLogLine(false, GetResString(IDS_GEOIP_COUNTRY_NOT_LOADED));
	AddLogLine(false, GetResString(IDS_GEOIP_COUNTRY_INFO));
	AddLogLine(false, GetResString(IDS_GEOIP_COUNTRY_INFO_DL));
	delete[] configDir;
	return;
}


CString CIP2Country::GetCountryFromIP(LPCTSTR fullUserIP)
{	
	if (gi == NULL)
		return _T("");

	USES_CONVERSION;
	char* userIP = nstrdup(CT2A(fullUserIP));

	if (db_loaded == GEOIP_CITY_EDITION_REV1)
	{
		GeoIPRecord * gir;
		gir = GeoIP_record_by_addr (gi, userIP);
		CString City;
		if (gir != NULL) {	
			CString temp1; temp1 = gir->city;
			CString temp2; temp2 = gir->country_name;
			City.Format(_T("%s [%s]"), temp1, temp2);
			GeoIPRecord_delete(gir);
		}		
		delete[] userIP;
		return City;
	}

	if (db_loaded == GEOIP_REGION_EDITION_REV1)
	{
		GeoIPRegion * gir;
		gir = GeoIP_region_by_name(gi, userIP);
		CString region;
		if (gir != NULL) {			
			CString temp1; temp1 = gir->country_code;
			CString temp2; temp2 = gir->region;
			region.Format(_T("%s, %s"),
				 temp1,
				 (!gir->region[0]) ? _T("N/A") :  temp2);
			GeoIPRegion_delete(gir);
		} 		
		delete[] userIP;
		return region;
	}
	
	CString theCountryName = (CString)GeoIP_country_name_by_addr(gi, userIP);
	delete[] userIP;	
	return theCountryName;
}

CString CIP2Country::GetCountryISOFromIP(LPCTSTR fullUserIP)
{	
	if (gi == NULL)
		return _T("");

	USES_CONVERSION;
	char* userIP = nstrdup(CT2A(fullUserIP));

	if (db_loaded == GEOIP_CITY_EDITION_REV1)
	{
		GeoIPRecord * gir;
		gir = GeoIP_record_by_addr (gi, userIP);
		CString City;
		if (gir != NULL) {				
			City = gir->country_code;
			GeoIPRecord_delete(gir);
		}		
		delete[] userIP;
		return City;
	}

	if (db_loaded == GEOIP_REGION_EDITION_REV1)
	{
		GeoIPRegion * gir;
		gir = GeoIP_region_by_name(gi, userIP);
		CString region;
		if (gir != NULL) {			
			region = gir->country_code;
			GeoIPRegion_delete(gir);
		} 		
		delete[] userIP;
		return region;
	}

	CString theCountry = (CString)GeoIP_country_code_by_addr(gi, userIP);
	delete[] userIP;
	return theCountry;
}

int CIP2Country::Compare(CString countryIP1, CString countryIP2)
{
	CString country1 = GetCountryFromIP(countryIP1);
	CString country2 = GetCountryFromIP(countryIP2);

	if (db_loaded == GEOIP_CITY_EDITION_REV1)
	{
		// Compare countries
		int find1 = country1.Find('[');
		int find2 = country2.Find('[');
		if ((find1 != -1) && (find2 != -1))
		{
			int compare = _tcsicmp(country1.Mid( find1 ), country2.Mid( country2.Find('[') ));
			if ( compare == 0 )
				return _tcsicmp(country1, country2);
			else
				return compare;
		}
		else
			if (find1 != -1)
				return 1;
			else 
				return -1;
	}

	return _tcsicmp(country1, country2);
}

CxImage CIP2Country::GetCountryFlag(LPCTSTR fullUserIP)
{	
	CxImage flag;	
	if (_hCountryFlagDll == NULL || thePrefs.GetEnableShowCountryFlags() == false)
	{
		flag.Enable(false);
		return flag;
	}

	CString countryISO = GetCountryISOFromIP(fullUserIP);
	
	// If is a non defined country
	if (countryISO.IsEmpty())
		countryISO = _T("--");
	
	if (country_flags.Lookup(countryISO, flag))
	{
		flag.Enable(true);
		return flag;
	}
	else
	{
		uint16	resID[] = 
		{
			IDI_COUNTRY_FLAG_NOFLAG,
			IDI_COUNTRY_FLAG_AD, IDI_COUNTRY_FLAG_AE, IDI_COUNTRY_FLAG_AF, IDI_COUNTRY_FLAG_AG, 
			IDI_COUNTRY_FLAG_AI, IDI_COUNTRY_FLAG_AL, IDI_COUNTRY_FLAG_AM, IDI_COUNTRY_FLAG_AN, 
			IDI_COUNTRY_FLAG_AO, IDI_COUNTRY_FLAG_AR, IDI_COUNTRY_FLAG_AS, IDI_COUNTRY_FLAG_AT, 
			IDI_COUNTRY_FLAG_AU, IDI_COUNTRY_FLAG_AW, IDI_COUNTRY_FLAG_AZ, IDI_COUNTRY_FLAG_BA, 
			IDI_COUNTRY_FLAG_BB, IDI_COUNTRY_FLAG_BD, IDI_COUNTRY_FLAG_BE, IDI_COUNTRY_FLAG_BF, 
			IDI_COUNTRY_FLAG_BG, IDI_COUNTRY_FLAG_BH, IDI_COUNTRY_FLAG_BI, IDI_COUNTRY_FLAG_BJ, 
			IDI_COUNTRY_FLAG_BM, IDI_COUNTRY_FLAG_BN, IDI_COUNTRY_FLAG_BO, IDI_COUNTRY_FLAG_BR, 
			IDI_COUNTRY_FLAG_BS, IDI_COUNTRY_FLAG_BT, IDI_COUNTRY_FLAG_BW, IDI_COUNTRY_FLAG_BY, 
			IDI_COUNTRY_FLAG_BZ, IDI_COUNTRY_FLAG_CA, IDI_COUNTRY_FLAG_CC, IDI_COUNTRY_FLAG_CD, 
			IDI_COUNTRY_FLAG_CF, IDI_COUNTRY_FLAG_CG, IDI_COUNTRY_FLAG_CH, IDI_COUNTRY_FLAG_CI, 
			IDI_COUNTRY_FLAG_CK, IDI_COUNTRY_FLAG_CL, IDI_COUNTRY_FLAG_CM, IDI_COUNTRY_FLAG_CN, 
			IDI_COUNTRY_FLAG_CO, IDI_COUNTRY_FLAG_CR, IDI_COUNTRY_FLAG_CU, IDI_COUNTRY_FLAG_CV, 
			IDI_COUNTRY_FLAG_CX, IDI_COUNTRY_FLAG_CY, IDI_COUNTRY_FLAG_CZ, IDI_COUNTRY_FLAG_DE, 
			IDI_COUNTRY_FLAG_DJ, IDI_COUNTRY_FLAG_DK, IDI_COUNTRY_FLAG_DM, IDI_COUNTRY_FLAG_DO, 
			IDI_COUNTRY_FLAG_DZ, IDI_COUNTRY_FLAG_EC, IDI_COUNTRY_FLAG_EE, IDI_COUNTRY_FLAG_EG, 
			IDI_COUNTRY_FLAG_EH, IDI_COUNTRY_FLAG_ER, IDI_COUNTRY_FLAG_ES, IDI_COUNTRY_FLAG_ET, 
			IDI_COUNTRY_FLAG_FI, IDI_COUNTRY_FLAG_FJ, IDI_COUNTRY_FLAG_FK, IDI_COUNTRY_FLAG_FM, 
			IDI_COUNTRY_FLAG_FO, IDI_COUNTRY_FLAG_FR, IDI_COUNTRY_FLAG_GA, IDI_COUNTRY_FLAG_GB, 
			IDI_COUNTRY_FLAG_GD, IDI_COUNTRY_FLAG_GE, IDI_COUNTRY_FLAG_GG, IDI_COUNTRY_FLAG_GH, 
			IDI_COUNTRY_FLAG_GI, IDI_COUNTRY_FLAG_GK, IDI_COUNTRY_FLAG_GL, IDI_COUNTRY_FLAG_GM, 
			IDI_COUNTRY_FLAG_GN, IDI_COUNTRY_FLAG_GP, IDI_COUNTRY_FLAG_GQ, IDI_COUNTRY_FLAG_GR, 
			IDI_COUNTRY_FLAG_GS, IDI_COUNTRY_FLAG_GT, IDI_COUNTRY_FLAG_GU, IDI_COUNTRY_FLAG_GW, 
			IDI_COUNTRY_FLAG_GY, IDI_COUNTRY_FLAG_HK, IDI_COUNTRY_FLAG_HN, IDI_COUNTRY_FLAG_HR, 
			IDI_COUNTRY_FLAG_HT, IDI_COUNTRY_FLAG_HU, IDI_COUNTRY_FLAG_ID, IDI_COUNTRY_FLAG_IE, 
			IDI_COUNTRY_FLAG_IL, IDI_COUNTRY_FLAG_IM, IDI_COUNTRY_FLAG_IN, IDI_COUNTRY_FLAG_IO, 
			IDI_COUNTRY_FLAG_IQ, IDI_COUNTRY_FLAG_IR, IDI_COUNTRY_FLAG_IS, IDI_COUNTRY_FLAG_IT, 
			IDI_COUNTRY_FLAG_JE, IDI_COUNTRY_FLAG_JM, IDI_COUNTRY_FLAG_JO, IDI_COUNTRY_FLAG_JP, 
			IDI_COUNTRY_FLAG_KE, IDI_COUNTRY_FLAG_KG, IDI_COUNTRY_FLAG_KH, IDI_COUNTRY_FLAG_KI, 
			IDI_COUNTRY_FLAG_KM, IDI_COUNTRY_FLAG_KN, IDI_COUNTRY_FLAG_KP, IDI_COUNTRY_FLAG_KR, 
			IDI_COUNTRY_FLAG_KW, IDI_COUNTRY_FLAG_KY, IDI_COUNTRY_FLAG_KZ, IDI_COUNTRY_FLAG_LA, 
			IDI_COUNTRY_FLAG_LB, IDI_COUNTRY_FLAG_LC, IDI_COUNTRY_FLAG_LI, IDI_COUNTRY_FLAG_LK, 
			IDI_COUNTRY_FLAG_LR, IDI_COUNTRY_FLAG_LS, IDI_COUNTRY_FLAG_LT, IDI_COUNTRY_FLAG_LU, 
			IDI_COUNTRY_FLAG_LV, IDI_COUNTRY_FLAG_LY, IDI_COUNTRY_FLAG_MA, IDI_COUNTRY_FLAG_MC, 
			IDI_COUNTRY_FLAG_MD, IDI_COUNTRY_FLAG_MG, IDI_COUNTRY_FLAG_MH, IDI_COUNTRY_FLAG_MK, 
			IDI_COUNTRY_FLAG_ML, IDI_COUNTRY_FLAG_MM, IDI_COUNTRY_FLAG_MN, IDI_COUNTRY_FLAG_MO, 
			IDI_COUNTRY_FLAG_MP, IDI_COUNTRY_FLAG_MQ, IDI_COUNTRY_FLAG_MR, IDI_COUNTRY_FLAG_MS, 
			IDI_COUNTRY_FLAG_MT, IDI_COUNTRY_FLAG_MU, IDI_COUNTRY_FLAG_MV, IDI_COUNTRY_FLAG_MW, 
			IDI_COUNTRY_FLAG_MX, IDI_COUNTRY_FLAG_MY, IDI_COUNTRY_FLAG_MZ, IDI_COUNTRY_FLAG_NA, 
			IDI_COUNTRY_FLAG_NC, IDI_COUNTRY_FLAG_NE, IDI_COUNTRY_FLAG_NF, IDI_COUNTRY_FLAG_NG, 
			IDI_COUNTRY_FLAG_NI, IDI_COUNTRY_FLAG_NL, IDI_COUNTRY_FLAG_NO, IDI_COUNTRY_FLAG_NP, 
			IDI_COUNTRY_FLAG_NR, IDI_COUNTRY_FLAG_NU, IDI_COUNTRY_FLAG_NZ, IDI_COUNTRY_FLAG_OM, 
			IDI_COUNTRY_FLAG_PA, IDI_COUNTRY_FLAG_PC, IDI_COUNTRY_FLAG_PE, IDI_COUNTRY_FLAG_PF, 
			IDI_COUNTRY_FLAG_PG, IDI_COUNTRY_FLAG_PH, IDI_COUNTRY_FLAG_PK, IDI_COUNTRY_FLAG_PL, 
			IDI_COUNTRY_FLAG_PM, IDI_COUNTRY_FLAG_PN, IDI_COUNTRY_FLAG_PR, IDI_COUNTRY_FLAG_PS, 
			IDI_COUNTRY_FLAG_PT, IDI_COUNTRY_FLAG_PW, IDI_COUNTRY_FLAG_PY, IDI_COUNTRY_FLAG_QA, 
			IDI_COUNTRY_FLAG_RO, IDI_COUNTRY_FLAG_RU, IDI_COUNTRY_FLAG_RW, IDI_COUNTRY_FLAG_SA, 
			IDI_COUNTRY_FLAG_SB, IDI_COUNTRY_FLAG_SC, IDI_COUNTRY_FLAG_SD, IDI_COUNTRY_FLAG_SE, 
			IDI_COUNTRY_FLAG_SG, IDI_COUNTRY_FLAG_SH, IDI_COUNTRY_FLAG_SI, IDI_COUNTRY_FLAG_SK, 
			IDI_COUNTRY_FLAG_SL, IDI_COUNTRY_FLAG_SM, IDI_COUNTRY_FLAG_SN, IDI_COUNTRY_FLAG_SO, 
			IDI_COUNTRY_FLAG_SR, IDI_COUNTRY_FLAG_ST, IDI_COUNTRY_FLAG_SU, IDI_COUNTRY_FLAG_SV, 
			IDI_COUNTRY_FLAG_SY, IDI_COUNTRY_FLAG_SZ, IDI_COUNTRY_FLAG_TC, IDI_COUNTRY_FLAG_TD, 
			IDI_COUNTRY_FLAG_TF, IDI_COUNTRY_FLAG_TG, IDI_COUNTRY_FLAG_TH, IDI_COUNTRY_FLAG_TJ, 
			IDI_COUNTRY_FLAG_TK, IDI_COUNTRY_FLAG_TL, IDI_COUNTRY_FLAG_TM, IDI_COUNTRY_FLAG_TN, 
			IDI_COUNTRY_FLAG_TO, IDI_COUNTRY_FLAG_TR, IDI_COUNTRY_FLAG_TT, IDI_COUNTRY_FLAG_TV, 
			IDI_COUNTRY_FLAG_TW, IDI_COUNTRY_FLAG_TZ, IDI_COUNTRY_FLAG_UA, IDI_COUNTRY_FLAG_UG, 
			IDI_COUNTRY_FLAG_UM, IDI_COUNTRY_FLAG_US, IDI_COUNTRY_FLAG_UY, IDI_COUNTRY_FLAG_UZ, 
			IDI_COUNTRY_FLAG_VA, IDI_COUNTRY_FLAG_VC, IDI_COUNTRY_FLAG_VE, IDI_COUNTRY_FLAG_VG, 
			IDI_COUNTRY_FLAG_VI, IDI_COUNTRY_FLAG_VN, IDI_COUNTRY_FLAG_VU, IDI_COUNTRY_FLAG_WF, 
			IDI_COUNTRY_FLAG_WS, IDI_COUNTRY_FLAG_YE, IDI_COUNTRY_FLAG_YU, IDI_COUNTRY_FLAG_ZA, 
			IDI_COUNTRY_FLAG_ZM, IDI_COUNTRY_FLAG_ZW, 
			IDI_COUNTRY_FLAG_UK, //by tharghan
			IDI_COUNTRY_FLAG_CS, //by propaganda
			IDI_COUNTRY_FLAG_TP, //by commander
			65535//the end
		};

		CString countryID[] = 
		{
			_T("--"),
			_T("AD"), _T("AE"), _T("AF"), _T("AG"), _T("AI"), _T("AL"), _T("AM"), _T("AN"), _T("AO"), _T("AR"), _T("AS"), _T("AT"), _T("AU"), _T("AW"), _T("AZ"), 
			_T("BA"), _T("BB"), _T("BD"), _T("BE"), _T("BF"), _T("BG"), _T("BH"), _T("BI"), _T("BJ"), _T("BM"), _T("BN"), _T("BO"), _T("BR"), _T("BS"), _T("BT"), 
			_T("BW"), _T("BY"), _T("BZ"), _T("CA"), _T("CC"), _T("CD"), _T("CF"), _T("CG"), _T("CH"), _T("CI"), _T("CK"), _T("CL"), _T("CM"), _T("CN"), _T("CO"), 
			_T("CR"), _T("CU"), _T("CV"), _T("CX"), _T("CY"), _T("CZ"), _T("DE"), _T("DJ"), _T("DK"), _T("DM"), _T("DO"), _T("DZ"), _T("EC"), _T("EE"), _T("EG"), 
			_T("EH"), _T("ER"), _T("ES"), _T("ET"), _T("FI"), _T("FJ"), _T("FK"), _T("FM"), _T("FO"), _T("FR"), _T("GA"), _T("GB"), _T("GD"), _T("GE"), _T("GG"), 
			_T("GH"), _T("GI"), _T("GK"), _T("GL"), _T("GM"), _T("GN"), _T("GP"), _T("GQ"), _T("GR"), _T("GS"), _T("GT"), _T("GU"), _T("GW"), _T("GY"), _T("HK"), 
			_T("HN"), _T("HR"), _T("HT"), _T("HU"), _T("ID"), _T("IE"), _T("IL"), _T("IM"), _T("IN"), _T("IO"), _T("IQ"), _T("IR"), _T("IS"), _T("IT"), _T("JE"), 
			_T("JM"), _T("JO"), _T("JP"), _T("KE"), _T("KG"), _T("KH"), _T("KI"), _T("KM"), _T("KN"), _T("KP"), _T("KR"), _T("KW"), _T("KY"), _T("KZ"), _T("LA"), 
			_T("LB"), _T("LC"), _T("LI"), _T("LK"), _T("LR"), _T("LS"), _T("LT"), _T("LU"), _T("LV"), _T("LY"), _T("MA"), _T("MC"), _T("MD"), _T("MG"), _T("MH"), 
			_T("MK"), _T("ML"), _T("MM"), _T("MN"), _T("MO"), _T("MP"), _T("MQ"), _T("MR"), _T("MS"), _T("MT"), _T("MU"), _T("MV"), _T("MW"), _T("MX"), _T("MY"), 
			_T("MZ"), _T("NA"), _T("NC"), _T("NE"), _T("NF"), _T("NG"), _T("NI"), _T("NL"), _T("NO"), _T("NP"), _T("NR"), _T("NU"), _T("NZ"), _T("OM"), _T("PA"), 
			_T("PC"), _T("PE"), _T("PF"), _T("PG"), _T("PH"), _T("PK"), _T("PL"), _T("PM"), _T("PN"), _T("PR"), _T("PS"), _T("PT"), _T("PW"), _T("PY"), _T("QA"), 
			_T("RO"), _T("RU"), _T("RW"), _T("SA"), _T("SB"), _T("SC"), _T("SD"), _T("SE"), _T("SG"), _T("SH"), _T("SI"), _T("SK"), _T("SL"), _T("SM"), _T("SN"), 
			_T("SO"), _T("SR"), _T("ST"), _T("SU"), _T("SV"), _T("SY"), _T("SZ"), _T("TC"), _T("TD"), _T("TF"), _T("TG"), _T("TH"), _T("TJ"), _T("TK"), _T("TL"), 
			_T("TM"), _T("TN"), _T("TO"), _T("TR"), _T("TT"), _T("TV"), _T("TW"), _T("TZ"), _T("UA"), _T("UG"), _T("UM"), _T("US"), _T("UY"), _T("UZ"), _T("VA"), 
			_T("VC"), _T("VE"), _T("VG"), _T("VI"), _T("VN"), _T("VU"), _T("WF"), _T("WS"), _T("YE"), _T("YU"), _T("ZA"), _T("ZM"), _T("ZW"), 
			_T("UK"), //by tharghan
			_T("CS"), //by propaganda
			_T("TP") //by commander
		};

			for(int cur_pos = 0; resID[cur_pos] != 65535; cur_pos++)
			{
				if (countryISO.CompareNoCase(countryID[cur_pos]) == 0)
				{				            
					if (flag.CreateFromHICON((HICON)LoadImage(_hCountryFlagDll, MAKEINTRESOURCE(resID[cur_pos]), IMAGE_ICON, 18, 16, LR_SHARED | LR_DEFAULTSIZE)))
					{
						flag.Enable(true);
						country_flags.SetAt(countryISO, flag);				
					}
					else
						flag.Enable(false);
                    
					return flag;
				}
			}
		}
	
	flag.Enable(false);
	return flag;
}
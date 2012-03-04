#pragma once

#define	WEBSVC_GEN_URLS		0x0001
#define	WEBSVC_FILE_URLS	0x0002

class CWebServices
{
public:
	CWebServices();

	CString GetDefaultServicesFile() const;
	int ReadAllServices();
	void RemoveAllServices();

	// [TPT] - New Menu Styles
	int GetFileMenuEntries(CMenuXP* rMenu) { return GetAllMenuEntries(rMenu, WEBSVC_FILE_URLS); }
	int GetGeneralMenuEntries(CMenuXP* rMenu) { return GetAllMenuEntries(rMenu, WEBSVC_GEN_URLS); }
	int GetAllMenuEntries(CMenuXP* rMenu, DWORD dwFlags = WEBSVC_GEN_URLS | WEBSVC_FILE_URLS);
	// [TPT] - New Menu Styles
	bool RunURL(const CAbstractFile* file, UINT uMenuID);
	void Edit();

protected:
	struct SEd2kLinkService
	{
		UINT uMenuID;
		CString strMenuLabel;
		CString strUrl;
		BOOL bFileMacros;
	};
	CArray<SEd2kLinkService> m_aServices;
	time_t m_tDefServicesFileLastModified;
};

extern CWebServices theWebServices;

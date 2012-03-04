// [TPT] - Crashrpt based on Maella

#include "stdafx.h"

// Maella -MiniDump/CrashRpt-
// Remark: should be stored in in other file
class MiniDump{
public:
	MiniDump(){
		m_pCrashRptState = NULL;
		m_hInstCrashRpt = NULL;
		LoadCrashRptLibrary();
	}
	
	~MiniDump(){
		UnloadCrashRptLibrary();
	}

	bool IsMiniDumpActivated() {return m_pCrashRptState != NULL;}

private:
	// Client crash callback
	typedef BOOL (CALLBACK *LPGETLOGFILE) (LPVOID lpvState);
	
	//-----------------------------------------------------------------------------
	// Install
	//    Initializes the library and optionally set the client crash callback and
	//    sets up the email details.
	//
	// Parameters
	//    pfn         Client crash callback
	//    lpTo        Email address to send crash report
	//    lpSubject   Subject line to be used with email
	//
	// Return Values
	//    If the function succeeds, the return value is a pointer to the underlying
	//    crash object created.  This state information is required as the first
	//    parameter to all other crash report functions.
	//
	// Remarks
	//    Passing NULL for lpTo will disable the email feature and cause the crash 
	//    report to be saved to disk.
	//
	typedef LPVOID (*INSTALL) (LPGETLOGFILE pfn, LPCTSTR lpTo, LPCTSTR lpSubject);

	//-----------------------------------------------------------------------------
	// Uninstall
	//    Uninstalls the unhandled exception filter set up in Install().
	//
	// Parameters
	//    lpState     State information returned from Install()
	//
	// Return Values
	//    void
	//
	// Remarks
	//    This call is optional.  The crash report library will automatically 
	//    deinitialize when the library is unloaded.  Call this function to
	//    unhook the exception filter manually.
	//
	typedef void (*UNINSTALL)(LPVOID lpState);

	//-----------------------------------------------------------------------------
	// AddFile
	//    Adds a file to the crash report.
	//
	// Parameters
	//    lpState     State information returned from Install()
	//    lpFile      Fully qualified file name
	//    lpDesc      Description of file, used by details dialog
	//
	// Return Values
	//    void
	//
	// Remarks
	//    This function can be called anytime after Install() to add one or more
	//    files to the generated crash report.
	//
	typedef void (*ADDFILE)(LPVOID lpState, LPCTSTR lpFile, LPCTSTR lpDesc);

	//-----------------------------------------------------------------------------
	// GenerateErrorReport
	//    Generates the crash report.
	//
	// Parameters
	//    lpState     State information returned from Install()
	//    pExInfo     Pointer to an EXCEPTION_POINTERS structure
	//
	// Return Values
	//    void
	//
	// Remarks
	//    Call this function to manually generate a crash report.
	//
	typedef void (*GENERATEERRORREPORT)(LPVOID lpState, PEXCEPTION_POINTERS pExInfo);

	void LoadCrashRptLibrary(){
		m_hInstCrashRpt = ::LoadLibrary(_T("CrashRpt.dll"));
		if(m_hInstCrashRpt != NULL){
			// Load pointer to function
			INSTALL Install = (INSTALL)GetProcAddress(m_hInstCrashRpt, "Install");
			// Start library
			m_pCrashRptState = Install(NULL, _T("thephoenixteam@yahoo.es"), MOD_VERSION);
		}
	}

	void UnloadCrashRptLibrary(){
		if(m_hInstCrashRpt != NULL){
			// Load pointer to function
			UNINSTALL Uninstall = (UNINSTALL)GetProcAddress(m_hInstCrashRpt, "Uninstall");
			Uninstall(m_pCrashRptState);
			m_pCrashRptState = NULL;

			// Release library
			if(::FreeLibrary(m_hInstCrashRpt) == TRUE){
				m_hInstCrashRpt = NULL;
			}
		}
	}

	// Don't allow canonical behavior
	MiniDump(const MiniDump&);
	MiniDump& operator=(const MiniDump&);

private:
	void*		m_pCrashRptState;
	HINSTANCE	m_hInstCrashRpt;

} s_miniDump;
// Maella end

// Maella -Test PasaKche-
// This library makes eMule unstable
// see http://www.internautas.org/article.php?sid=1110
class TestPasaKche{
public:
	TestPasaKche(){
		TestPasaKcheLibrary();
	}
	
	~TestPasaKche(){
	}

private:
	void TestPasaKcheLibrary(){
		// try to load the library
		HINSTANCE m_hInstance = ::LoadLibrary(_T("pasakche.dll"));
		if(m_hInstance != NULL){

			// Release library
			::FreeLibrary(m_hInstance);

			// Warning
			::MessageBox(NULL,
						_T("Warning, PasaKche detected. This library makes eMule unstable.\n\nAttention, PasaKche détecté. Cette librarie peut rendre eMule instable.\n\nAchtung, PasaKche gefunden. Diese Bibliothek ist inkompatibel mit eMule.\n\nAdvirtiendo,  PasaKche encontrado. Esta biblioteca es incompatible con el eMule."),
						_T("Warning/Attention/Achtung/Advirtiendo"), 
						MB_ICONWARNING);
		}
	}

	// Don't allow canonical behavior
	TestPasaKche(const TestPasaKche&);
	TestPasaKche& operator=(const TestPasaKche&);

} s_testPasaKche;
// Maella end
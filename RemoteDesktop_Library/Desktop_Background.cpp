#include "stdafx.h"
#include "Desktop_Background.h"
#include "UserInfo.h"
#include "Wininet.h"
#include "Shlobj.h"
#include <thread>

RemoteDesktop::DesktopBackground::DesktopBackground(){
	DEBUG_MSG("DesktopBackground()");
	Red = Green = Blue = 0;
	HKEY hkLocal;
	DWORD dw;
	//need to get the users SID. THis is because the program can run as SYSTEM or another user account. The background must correspond with the logged in user!
	auto userinfo = GetUserInfo();
	auto fqname = std::wstring(userinfo.domain) + L"\\" + std::wstring(userinfo.name);
	DEBUG_MSG("DesktopBackground() %", ws2s(fqname));

	if (!GetSid_From_Username(fqname, UserSID))	return;//nothing can be done.. !!

	auto regkey = UserSID + L"\\Control Panel\\Colors";
	DEBUG_MSG("DesktopBackground() %", ws2s(regkey));
	if (RegOpenKeyExW(HKEY_USERS, regkey.c_str(), 0, KEY_READ, &hkLocal) != ERROR_SUCCESS) return;
	
	wchar_t data[MAX_PATH];
	DWORD bytes = sizeof(data) * 2;
	if (RegQueryValueEx(hkLocal,
		L"Background",
		0,
		NULL,
		(BYTE*)data,
		&bytes) == ERROR_SUCCESS){
		std::wstring col = data;
		col = trim(col);
		auto splits = split(col, L' ');//split by spaces
		if (splits.size() == 3){
			Red = (unsigned char)std::stoi(splits[0]);
			Green = (unsigned char)std::stoi(splits[1]);
			Blue = (unsigned char)std::stoi(splits[2]);
		}
	}
	
	RegCloseKey(hkLocal);
	hkLocal = nullptr;
	regkey = UserSID + L"\\Control Panel\\Desktop";
	if (RegOpenKeyExW(HKEY_USERS, regkey.c_str(), 0, KEY_READ, &hkLocal) != ERROR_SUCCESS) return;

	bytes = sizeof(data) * 2;
	if (RegQueryValueEx(hkLocal,
		L"Wallpaper",
		0,
		NULL,
		(BYTE*)data,
		&bytes) == ERROR_SUCCESS){
		OldWallpaper = data;
	}
	RegCloseKey(hkLocal);
	DEBUG_MSG("DesktopBackground()");

}
RemoteDesktop::DesktopBackground::~DesktopBackground(){
	Restore();
}

void RemoteDesktop::DesktopBackground::Restore(){
	if (!UserSID.empty()){
		if (OldWallpaper.size() > 2) Set(OldWallpaper);
		else SetColor(Red, Green, Blue);
	}

}
bool RemoteDesktop::DesktopBackground::SetColor(unsigned char red, unsigned char green, unsigned char blue){
	DEBUG_MSG("Set Background Color % % %", red, green, blue);
	HKEY hkLocal;
	if (UserSID.empty()) return false;

	auto regkey = UserSID + L"\\Control Panel\\Colors";
	if (RegOpenKeyExW(HKEY_USERS, regkey.c_str(), 0, KEY_WRITE, &hkLocal) != ERROR_SUCCESS) return false;

	std::wstring color = std::to_wstring((int)red) + L" " + std::to_wstring((int)green) + L" " + std::to_wstring((int)blue);
	DWORD bytes = color.size() * 2;
	auto ret = RegSetValueEx(hkLocal,
		L"Background",
		0,
		REG_SZ,
		(BYTE*)color.c_str(),
		bytes + 2);
	RegCloseKey(hkLocal);
	return true;
}
bool RemoteDesktop::DesktopBackground::SetWallpaper(std::wstring path){
	DEBUG_MSG("SetWallpaper %", ws2s(path));

	if (UserSID.empty()) return false;

	HKEY hkLocal;
	auto regkey = UserSID + L"\\Control Panel\\Desktop";
	auto ret = RegOpenKeyExW(HKEY_USERS, regkey.c_str(), 0, KEY_WRITE, &hkLocal);
	DWORD bytes = path.size() * 2;
	ret = RegSetValueExW(hkLocal,
		L"Wallpaper",
		0,
		REG_SZ,
		(BYTE*)path.c_str(),
		bytes + 2);
	RegCloseKey(hkLocal);

	return true;
}

bool RemoteDesktop::DesktopBackground::Set(unsigned char red, unsigned char green, unsigned char blue){
	if (UserSID.empty()) return false;
	auto ret = SetColor(red, green, blue);
	SetWallpaper(L"");//clear the wallpaper
	SystemParametersInfoW(SPI_SETDESKWALLPAPER, 0, L"", SPIF_UPDATEINIFILE | SPIF_SENDWININICHANGE);
	SetWallpaper(OldWallpaper);
	return ret;
}


bool RemoteDesktop::DesktopBackground::Set(std::wstring path){

	CoInitializeEx(NULL, COINIT_MULTITHREADED);
	IActiveDesktop *pActiveDesktop = nullptr;
	if (SUCCEEDED(CoCreateInstance(CLSID_ActiveDesktop, NULL, CLSCTX_INPROC_SERVER, IID_IActiveDesktop, (void**)&pActiveDesktop))){
		pActiveDesktop->SetWallpaper(path.c_str(), 0);
		pActiveDesktop->ApplyChanges(AD_APPLY_ALL | AD_APPLY_FORCE);
		// Call the Release method 
		pActiveDesktop->Release();
	}

	CoUninitialize();	
	auto ret = SetWallpaper(path); 
	SystemParametersInfoW(SPI_SETDESKWALLPAPER, 0, (void*)path.c_str(), SPIF_UPDATEINIFILE | SPIF_SENDWININICHANGE);
	//std::this_thread::sleep_for(std::chrono::milliseconds(5));
	//SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
	return ret;
}

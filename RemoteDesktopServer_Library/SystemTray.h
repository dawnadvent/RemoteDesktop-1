#ifndef SYSTEMTRAY_123_H
#define SYSTEMTRAY_123_H
#include <thread>
#include "..\RemoteDesktop_Library\Desktop_Monitor.h"

namespace RemoteDesktop{
	class SystemTray{
		HWND Hwnd = nullptr;
		HMENU Hmenu = nullptr;
		HICON _SystemTrayIcon = nullptr;
		bool _TrayIconCreated = false;
		std::thread _BackGroundThread; 
		NOTIFYICONDATA notifyIconData;
		
		void _Cleanup();
		void _Run();
		void _CreateIcon(HWND hWnd);

	public:
		SystemTray();
		~SystemTray();
		void Start();
		void Stop();
		void Popup(const wchar_t* title, const wchar_t* message, unsigned int timeout);
		LRESULT WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	};
}

#endif
#include "Display.h"

// https://github.com/Maximus5/ConEmu/blob/master/src/common/Monitors.cpp
struct _FindPrimaryMonitor
{
  HMONITOR hMon;
  MONITORINFO mi;
};

BOOL CALLBACK FindPrimaryMonitor(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
  MONITORINFO mi = { sizeof(mi) };

  if (GetMonitorInfo(hMonitor, &mi) && (mi.dwFlags & MONITORINFOF_PRIMARY))
  {
    _FindPrimaryMonitor* pMon = (_FindPrimaryMonitor*)dwData;
    pMon->hMon = hMonitor;
    pMon->mi = mi;
    // And stop enumeration
    return FALSE;
  }

  return TRUE;
}

// primary dislpay rect
void priDisplayRect(RECT &rect) 
{
  _FindPrimaryMonitor m = { NULL };

  EnumDisplayMonitors(NULL, NULL, FindPrimaryMonitor, (LPARAM)&m);

  if (!m.hMon)
  {
    //_ASSERTE(FALSE && "FindPrimaryMonitor fails");
    // ≈сли облом с мониторами - берем данные по умолчанию
    m.mi.cbSize = 0;
    SystemParametersInfo(SPI_GETWORKAREA, 0, &m.mi.rcWork, 0);
    m.mi.rcMonitor.left = m.mi.rcMonitor.top = 0;
    m.mi.rcMonitor.right = GetSystemMetrics(SM_CXFULLSCREEN);
    m.mi.rcMonitor.bottom = GetSystemMetrics(SM_CYFULLSCREEN);
    m.mi.dwFlags = 0;
  }

  rect.left = m.mi.rcMonitor.left;
  rect.top = m.mi.rcMonitor.top;
  rect.right = m.mi.rcMonitor.right;
  rect.bottom = m.mi.rcMonitor.bottom;
}

// center rect
void centerRect(RECT &rect, LONG width, LONG height) 
{
  rect.left = (LONG)(rect.right - rect.left - width) / 2;
  rect.top = (LONG)(rect.bottom - rect.top - height) / 2;
  rect.right = rect.left + width;
  rect.bottom = rect.top + height;
}

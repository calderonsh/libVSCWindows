#ifndef __VSC_WEBVIEW2_H___
#define __VSC_WEBVIEW2_H___

#ifndef  DLLEXPORT 
#define DLLEXPORT __declspec(dllimport)
#endif

extern "C" {

class vsc_webview2;

DLLEXPORT vsc_webview2* vsc_webview2_new(const char* title, int width, int height);

DLLEXPORT int vsc_webview2_open(vsc_webview2*);

DLLEXPORT void vsc_webview2_set_html(vsc_webview2*, const char* html);

DLLEXPORT bool vsc_webview2_loop(vsc_webview2*);

};

#endif // __VSC_WEBVIEW2_H___

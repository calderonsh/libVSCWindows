#ifndef __VSC_WEBVIEW2_H___
#define __VSC_WEBVIEW2_H___

#ifndef  DLLEXPORT 
#define DLLEXPORT __declspec(dllimport)
#endif

extern "C" {

class vsc_webview2;

DLLEXPORT vsc_webview2* vsc_webview2_new(const char* title, int width, int height);

DLLEXPORT int vsc_webview2_open(vsc_webview2*);

DLLEXPORT void vsc_webview2_navigate(vsc_webview2* _this, const char* url);
DLLEXPORT void vsc_webview2_set_html(vsc_webview2*, const char* html);

DLLEXPORT void vsc_webview2_eval(vsc_webview2*, const char* javascript);
DLLEXPORT void vsc_webview2_callback(vsc_webview2* _this, void (*callback)(vsc_webview2* _this, const char* message));

DLLEXPORT bool vsc_webview2_loop(vsc_webview2*);

};

#endif // __VSC_WEBVIEW2_H___

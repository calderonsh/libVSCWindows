#include "pch.h"

#include <stdio.h>

#include <functional>
#include <string>
#include <map>

#include <wil/com.h>
#include <wrl.h>

#include "WebView2.h"

#define DLLEXPORT __declspec(dllexport) 
class vsc_webview2;
#include "vsc_webview2.h"

class vsc_webview2
{
	public:
		int width;
		int height;
		std::string title;

		std::string html;
		std::string javascript;

		bool ready = false;

		std::function<void(vsc_webview2*, const char*)> callback = nullptr;

		HWND hWindow = NULL;
		wil::com_ptr<ICoreWebView2Controller> webviewController = nullptr;
		wil::com_ptr<ICoreWebView2> webviewWindow = nullptr;
};

std::map<HWND, vsc_webview2*> vsc_webview2_map;

LRESULT CALLBACK _vsc_webview2_wndproc(HWND hWindow, UINT message, WPARAM wParam, LPARAM lParam)
{
	vsc_webview2* _this = nullptr;

	if (vsc_webview2_map.find(hWindow) != vsc_webview2_map.end()) {
		_this = vsc_webview2_map[hWindow];
	}

	switch (message)
	{
	case WM_SIZE:
		if (_this != nullptr && _this->webviewController != nullptr)
		{
			RECT bounds;
			GetClientRect(hWindow, &bounds);
			_this->webviewController->put_Bounds(bounds);
		};
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWindow, message, wParam, lParam);
		break;
	}

	return 0;
}

vsc_webview2* vsc_webview2_new(const char* title, int width, int height)
{
	vsc_webview2* result = new vsc_webview2;

	result->width = width;
	result->height = height;
	result->title = title;

	return result;
}

int vsc_webview2_open(vsc_webview2* _this)
{
	const wchar_t szWindowClass[] = L"VSCWebview2";
	HINSTANCE hInstance = GetModuleHandle(NULL);

	WNDCLASSEX wndClassEx;
	wndClassEx.cbSize = sizeof(WNDCLASSEX);
	wndClassEx.style = CS_HREDRAW | CS_VREDRAW;
	wndClassEx.lpfnWndProc = _vsc_webview2_wndproc;
	wndClassEx.cbClsExtra = 0;
	wndClassEx.cbWndExtra = 0;
	wndClassEx.hInstance = hInstance;
	wndClassEx.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
	wndClassEx.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClassEx.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wndClassEx.lpszMenuName = NULL;
	wndClassEx.lpszClassName = szWindowClass;
	wndClassEx.hIconSm = LoadIcon(hInstance, IDI_APPLICATION);
	RegisterClassEx(&wndClassEx);

	std::wstring wcsTitle(_this->title.begin(), _this->title.end());
	_this->hWindow = CreateWindow(szWindowClass, wcsTitle.c_str(), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, _this->width, _this->height, NULL, NULL, hInstance, NULL);
	vsc_webview2_map[_this->hWindow] = _this;

	ShowWindow(_this->hWindow, SW_SHOWNORMAL);
	UpdateWindow(_this->hWindow);

	CreateCoreWebView2EnvironmentWithOptions(nullptr, nullptr, nullptr, Microsoft::WRL::Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
		[_this](HRESULT result, ICoreWebView2Environment* env) -> HRESULT
		{
			env->CreateCoreWebView2Controller(_this->hWindow, Microsoft::WRL::Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
				[_this](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT
				{
					if (controller != nullptr)
					{
						_this->webviewController = controller;
						_this->webviewController->get_CoreWebView2(&_this->webviewWindow);
					}

					RECT bounds;
					GetClientRect(_this->hWindow, &bounds);
					_this->webviewController->put_Bounds(bounds);

					if (_this->html.length() > 0) {
						vsc_webview2_set_html(_this, _this->html.c_str());
					}

					_this->ready = true;
					if (_this->javascript.length() > 0) {
						vsc_webview2_eval(_this, _this->javascript.c_str());
					}

					EventRegistrationToken token;
					_this->webviewWindow->add_WebMessageReceived(Microsoft::WRL::Callback<ICoreWebView2WebMessageReceivedEventHandler>(
						[_this](ICoreWebView2* webview, ICoreWebView2WebMessageReceivedEventArgs* args) -> HRESULT
						{
							if (_this->callback != nullptr)
							{
								PWSTR lpszMessage;
								args->TryGetWebMessageAsString(&lpszMessage);

								std::wstring wcsMessage = lpszMessage;
								std::string strMessage(wcsMessage.begin(), wcsMessage.end());

								_this->callback(_this, strMessage.c_str());

								CoTaskMemFree(lpszMessage);
							}

							return S_OK;
						}).Get(), &token);

					return S_OK;
				}).Get());

			return S_OK;
		}).Get());

	return 0;
}

void vsc_webview2_set_html(vsc_webview2* _this, const char* html)
{
	if (_this->webviewWindow == nullptr) {
		_this->html = html;
	}
	else
	{
		std::string strHTML = html;
		std::wstring wcsHTML(strHTML.begin(), strHTML.end());

		_this->webviewWindow->NavigateToString(wcsHTML.c_str());
	}
}

void vsc_webview2_eval(vsc_webview2* _this, const char* javascript)
{
	if (_this->webviewWindow == nullptr) {
		_this->javascript += javascript;
	}
	else
	{
		std::string strJavascript = javascript;
		std::wstring wcsJavascript(strJavascript.begin(), strJavascript.end());

		while (_this->ready == false) {
			Sleep(1);
		}

		_this->ready = false;
		_this->webviewWindow->ExecuteScript(wcsJavascript.c_str(), Microsoft::WRL::Callback<ICoreWebView2ExecuteScriptCompletedHandler>(
		[_this](HRESULT errorCode, LPCWSTR resultObjectAsJson) -> HRESULT
		{
			_this->ready = true;
			return S_OK;
		}).Get());
	}
}

void vsc_webview2_callback(vsc_webview2* _this, void (*callback)(vsc_webview2* _this, const char* message))
{
	_this->callback = callback;
}

bool vsc_webview2_loop(vsc_webview2*)
{
	MSG message;
	bool result = GetMessage(&message, NULL, 0, 0);

	if (result == true)
	{
		TranslateMessage(&message);
		DispatchMessage(&message);
	}

	return result;
}
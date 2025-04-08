#ifndef GRAPHICS_WINDOW_WINDOWS_WINDOW_HEADER
#define GRAPHICS_WINDOW_WINDOWS_WINDOW_HEADER

#include "../Window.hpp"
#include <string>
#include <string_view>

#include <windows.h>

namespace Window
{

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

class Win32Window : public IWindow
{
	using NativeType = HWND;
public:
	static inline LPCWSTR CLASS_NAME = L"WIN32 window impl";
public:
	Win32Window(const std::string& title, int w, int h, WindowFlags flags);
	virtual ~Win32Window() override;

	virtual WindowExtent GetDimensions() const override;
	virtual void GetDimensions(int* w, int* h) const override;

	virtual VkSurfaceKHR CreateVulkanSurface(VkInstance instance) override;

	virtual uint64_t GetWindowID() override;


	virtual void InitImguiForVulkan() override;


	// Inherited via IWindow
	void ShutdownImGuiWindow() override;

	// Inherited via IWindow
	void ProcessEvents(void* event) override;

	// Inherited via IWindow
	void BeginImGuiFrame() override;
private:
	NativeType m_window;
	HINSTANCE m_instance;
};
}
#endif //!GRAPHICS_WINDOW_WINDOWS_WINDOW_HEADER
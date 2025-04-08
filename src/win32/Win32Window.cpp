#include "win32/Win32Window.hpp"


#include <backends/imgui_impl_vulkan.h>
#include <backends/imgui_impl_win32.h>
#include <vulkan/vulkan.h>

#include <bit>
#include <cstdint>
#include <string>
#include <windows.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Window
{

LRESULT WINAPI WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
		return true;

	switch (uMsg)
	{
	case WM_CLOSE:
		DestroyWindow(hWnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

static Event get_equivalent_event_from_windows(MSG* event)
{
	typedef Window::Event E;
	switch (event->message)
	{
	case WM_PAINT:      return E::RESIZED;
	case WM_SIZE:
	{
		if (event->wParam = SIZE_MINIMIZED)
			return E::MINIMIZED;
		if (event->wParam = SIZE_RESTORED)
			return E::RESTORED;
		if (event->wParam = SIZE_MAXIMIZED)
			return E::MAXIMIZED;
		break;
	}
	case WM_CLOSE:      return E::CLOSE;
	case WM_MOVE:       return E::MOVED;
	case WM_ACTIVATE:
	{
		if (event->wParam == WA_CLICKACTIVE)
			return E::FOCUS_GAIN_MOUSE;
		if (event->wParam == WA_INACTIVE)
			return E::FOCUS_LOST_MOUSE;
		break;
	}
	case WM_SETFOCUS:   return E::FOCUS_GAIN_KEYBOARD;
	case WM_KILLFOCUS:  return E::FOCUS_LOST_KEYBOARD;
	}

	return E::NONE;
}

static int get_equivalent_windows_flag(Window::WindowFlags flag)
{
	int window_flags = 0;

	if (flag & Flags::BORDERLESS)
		window_flags |= WS_POPUP;
	else if (flag & Flags::FULLSCREEN)
		window_flags |= 0;
	else
		window_flags |= WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;

	if (flag & Flags::RESIZABLE) window_flags |= WS_SIZEBOX | WS_MAXIMIZEBOX;
	if (flag & Flags::MAXIMIZED) window_flags |= WS_MAXIMIZE;
	if (flag & Flags::MINIMIZED) window_flags |= WS_MINIMIZE;
	//if (flag & Flags::OPENGL)     window_flags |= 0;
	//if (flag & Flags::VULKAN)     window_flags |= 0;


	return window_flags;
}


Win32Window::Win32Window(const std::string& title, int w, int h, WindowFlags flags)
	: m_instance(GetModuleHandle(nullptr))
{
	std::wstring wsTmp(title.begin(), title.end());

	WNDCLASS wnd_class{};
	wnd_class.lpszClassName = CLASS_NAME;
	wnd_class.hInstance = m_instance;
	wnd_class.hIcon = NULL;
	wnd_class.hCursor = NULL;
	wnd_class.lpfnWndProc = WindowProc;

	RegisterClass(&wnd_class);

	DWORD style = get_equivalent_windows_flag(flags);
	RECT rect;
	rect.left = 250;
	rect.top = 250;
	rect.right = rect.left + w;
	rect.bottom = rect.top + h;

	AdjustWindowRect(&rect, style, false);

	m_window = CreateWindowEx(
		0,
		wnd_class.lpszClassName,
		wsTmp.c_str(),
		style,
		rect.left,
		rect.top,
		rect.right - rect.left,
		rect.bottom - rect.top,
		NULL,
		NULL,
		m_instance,
		NULL
	);

	ShowWindow(m_window, SW_SHOW);
}

Win32Window::~Win32Window()
{
	UnregisterClass(CLASS_NAME, m_instance);
	DestroyWindow(m_window);
}

WindowExtent Win32Window::GetDimensions() const
{
	RECT rect;
	GetWindowRect(m_window, &rect);

	return { rect.right - rect.left , rect.bottom - rect.top };
}

void Win32Window::GetDimensions(int* w, int* h) const
{
	auto [_w, _h] = GetDimensions();
	*w = _w;
	*h = _h;
}


VkSurfaceKHR Win32Window::CreateVulkanSurface(VkInstance instance)
{
	VkSurfaceKHR surface;
	VkWin32SurfaceCreateInfoKHR info = { VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR, NULL };
	info.hinstance = m_instance;
	info.hwnd = m_window;
	vkCreateWin32SurfaceKHR(instance, &info, nullptr, &surface);
	return surface;
}


uint64_t Win32Window::GetWindowID()
{
	return GetWindowLong(m_window, GWL_ID);
}

static int create_vk_surface(ImGuiViewport* viewport, ImU64 vk_instance, const void* vk_allocator, ImU64* out_vk_surface)
{
	VkWin32SurfaceCreateInfoKHR info = { VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR, NULL };
	info.hinstance = GetModuleHandle(nullptr);
	info.hwnd = static_cast<HWND>( viewport->PlatformHandleRaw );

	VkResult err = vkCreateWin32SurfaceKHR((VkInstance)vk_instance, &info, nullptr, (VkSurfaceKHR*)out_vk_surface);

	return (int)err;
}
void Win32Window::InitImguiForVulkan()
{
	//ImGui_ImplWin32_InitForVulkan(m_window);
	ImGui_ImplWin32_Init(m_window);
	ImGui::GetPlatformIO().Platform_CreateVkSurface = create_vk_surface;

}
void Win32Window::ShutdownImGuiWindow()
{
	ImGui_ImplWin32_Shutdown();
}
void Win32Window::ProcessEvents(void* event)
{
	MSG* e = static_cast<MSG*>( event );


	Event my_event = get_equivalent_event_from_windows(e);
	if (m_event_fn.contains(my_event))
	{
		m_event_fn.at(my_event)( this, event );
	}

}
void Win32Window::BeginImGuiFrame()
{
	ImGui_ImplWin32_NewFrame();
}


}
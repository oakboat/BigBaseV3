#include "common.hpp"
#include "function_types.hpp"
#include "logger.hpp"
#include "gta/array.hpp"
#include "gta/player.hpp"
#include "gta/script_thread.hpp"
#include "gui.hpp"
#include "hooking.hpp"
#include "memory/module.hpp"
#include "natives.hpp"
#include "pointers.hpp"
#include "renderer.hpp"
#include "script_mgr.hpp"

#include <MinHook.h>
#include "kiero.h"

namespace big
{
	static GtaThread *find_script_thread(rage::joaat_t hash)
	{
		for (auto thread : *g_pointers->m_script_threads)
		{
			if (thread
				&& thread->m_context.m_thread_id
				&& thread->m_handler
				&& thread->m_script_hash == hash)
			{
				return thread;
			}
		}

		return nullptr;
	}

	hooking::hooking() :
		m_set_cursor_pos_hook("SetCursorPos", memory::module("user32.dll").get_export("SetCursorPos").as<void*>(), &hooks::set_cursor_pos),

		m_run_script_threads_hook("Script hook", g_pointers->m_run_script_threads, &hooks::run_script_threads),
		m_convert_thread_to_fiber_hook("ConvertThreadToFiber", memory::module("kernel32.dll").get_export("ConvertThreadToFiber").as<void*>(), &hooks::convert_thread_to_fiber),
		m_excute_command_list_hook("ExcuteCommandList hook", (void*)kiero::getMethodsTable()[54], &hooks::execute_command_lists),
		m_dxgi_present_hook("Present hook", (void*)kiero::getMethodsTable()[140], &hooks::swapchain_present),
		m_resize_buffer_hook("ResizeBuffer hook", (void*)kiero::getMethodsTable()[145], &hooks::swapchain_resizebuffers)
	{
		g_hooking = this;
	}

	hooking::~hooking()
	{
		if (m_enabled)
			disable();

		g_hooking = nullptr;
	}

	void hooking::enable()
	{
		m_excute_command_list_hook.enable();
		m_dxgi_present_hook.enable();
		m_resize_buffer_hook.enable();
		m_og_wndproc = reinterpret_cast<WNDPROC>(SetWindowLongPtrW(g_pointers->m_hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&hooks::wndproc)));
		m_set_cursor_pos_hook.enable();

		m_run_script_threads_hook.enable();
		m_convert_thread_to_fiber_hook.enable();
		
		ensure_dynamic_hooks();
		m_enabled = true;
	}

	void hooking::disable()
	{
		m_enabled = false;

		if (m_main_persistent_hook)
		{
			m_main_persistent_hook->disable();
		}

		m_convert_thread_to_fiber_hook.disable();
		m_run_script_threads_hook.disable();

		m_set_cursor_pos_hook.disable();
		SetWindowLongPtrW(g_pointers->m_hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(m_og_wndproc));
		m_excute_command_list_hook.disable();
		m_dxgi_present_hook.disable();
		m_resize_buffer_hook.disable();
	}

	void hooking::ensure_dynamic_hooks()
	{
		if (!m_main_persistent_hook)
		{
			if (auto main_persistent = find_script_thread(RAGE_JOAAT("main_persistent")))
			{
				m_main_persistent_hook = std::make_unique<vmt_hook>(main_persistent->m_handler, hooks::main_persistent_num_funcs);
				m_main_persistent_hook->hook(hooks::main_persistent_dtor_index, &hooks::main_persistent_dtor);
				m_main_persistent_hook->hook(hooks::main_persistent_is_networked_index, &hooks::main_persistent_is_networked);
				m_main_persistent_hook->enable();
			}
		}
	}

	minhook_keepalive::minhook_keepalive()
	{
		MH_Initialize();
	}

	minhook_keepalive::~minhook_keepalive()
	{
		MH_Uninitialize();
	}

	bool hooks::run_script_threads(std::uint32_t ops_to_execute)
	{
		if (g_running)
		{
			g_script_mgr.tick();
		}

		return g_hooking->m_run_script_threads_hook.get_original<functions::run_script_threads_t>()(ops_to_execute);
	}

	void *hooks::convert_thread_to_fiber(void *param)
	{
		if (IsThreadAFiber())
		{
			return GetCurrentFiber();
		}

		return g_hooking->m_convert_thread_to_fiber_hook.get_original<decltype(&convert_thread_to_fiber)>()(param);
	}

	HRESULT hooks::swapchain_present(IDXGISwapChain3 *this_, UINT sync_interval, UINT flags)
	{
		if (g_running)
		{
			g_renderer->on_present(this_);
		}
		return g_hooking->m_dxgi_present_hook.get_original<decltype(&swapchain_present)>()(this_, sync_interval, flags);
	}

	void hooks::execute_command_lists(ID3D12CommandQueue* queue, UINT NumCommandLists, ID3D12CommandList* ppCommandLists)
	{
		if (g_running)
		{
			g_renderer->on_ExecuteCommandLists(queue);
		}
		return g_hooking->m_excute_command_list_hook.get_original<decltype(&execute_command_lists)>()(queue, NumCommandLists, ppCommandLists);
	}

	HRESULT hooks::swapchain_resizebuffers(IDXGISwapChain3 * this_, UINT buffer_count, UINT width, UINT height, DXGI_FORMAT new_format, UINT swapchain_flags)
	{
		if (g_running)
		{
			g_renderer->pre_reset();

			auto result = g_hooking->m_resize_buffer_hook.get_original<decltype(&swapchain_resizebuffers)>()
				(this_, buffer_count, width, height, new_format, swapchain_flags);

			if (SUCCEEDED(result))
			{
				g_renderer->post_reset();
			}

			return result;
		}

		return g_hooking->m_resize_buffer_hook.get_original<decltype(&swapchain_resizebuffers)>()
			(this_, buffer_count, width, height, new_format, swapchain_flags);
	}

	LRESULT hooks::wndproc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
	{
		if (g_running)
		{
			g_renderer->wndproc(hwnd, msg, wparam, lparam);
		}

		return CallWindowProcW(g_hooking->m_og_wndproc, hwnd, msg, wparam, lparam);
	}

	BOOL hooks::set_cursor_pos(int x, int y)
	{
		if (g_gui.m_opened)
		{
			return true;
		}

		return g_hooking->m_set_cursor_pos_hook.get_original<decltype(&set_cursor_pos)>()(x, y);
	}

	void hooks::main_persistent_dtor(CGameScriptHandler *this_, bool free_memory)
	{
		auto og_func = g_hooking->m_main_persistent_hook->get_original<decltype(&main_persistent_dtor)>(main_persistent_dtor_index);
		g_hooking->m_main_persistent_hook->disable();
		g_hooking->m_main_persistent_hook.reset();
		return og_func(this_, free_memory);
	}

	bool hooks::main_persistent_is_networked(CGameScriptHandler *this_)
	{
		return *g_pointers->m_is_session_started;
	}
}

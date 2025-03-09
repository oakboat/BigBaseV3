#pragma once
#include "common.hpp"
#include "detour_hook.hpp"
#include "gta/fwddec.hpp"
#include "script_hook.hpp"
#include "vmt_hook.hpp"

namespace big
{
	struct hooks
	{
		static bool run_script_threads(std::uint32_t ops_to_execute);
		static void *convert_thread_to_fiber(void *param);

		static HRESULT swapchain_present(IDXGISwapChain3* this_, UINT sync_interval, UINT flags);
		static HRESULT swapchain_resizebuffers(IDXGISwapChain3* this_, UINT buffer_count, UINT width, UINT height, DXGI_FORMAT new_format, UINT swapchain_flags);
		static void execute_command_lists(ID3D12CommandQueue* queue, UINT NumCommandLists, ID3D12CommandList* ppCommandLists);

		static LRESULT wndproc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
		static BOOL set_cursor_pos(int x, int y);

		static constexpr auto main_persistent_num_funcs = 16;
		static constexpr auto main_persistent_dtor_index = 0;
		static constexpr auto main_persistent_is_networked_index = 6;
		static void main_persistent_dtor(CGameScriptHandler *this_, bool free_memory);
		static bool main_persistent_is_networked(CGameScriptHandler *this_);
	};

	struct minhook_keepalive
	{
		minhook_keepalive();
		~minhook_keepalive();
	};

	class hooking
	{
		friend hooks;
	public:
		explicit hooking();
		~hooking();

		void enable();
		void disable();

		void ensure_dynamic_hooks();
	private:
		bool m_enabled{};
		minhook_keepalive m_minhook_keepalive;

		WNDPROC m_og_wndproc;
		detour_hook m_set_cursor_pos_hook;
		detour_hook m_dxgi_present_hook;
		detour_hook m_excute_command_list_hook;
		detour_hook m_resize_buffer_hook;

		detour_hook m_run_script_threads_hook;
		detour_hook m_convert_thread_to_fiber_hook;
		std::unique_ptr<vmt_hook> m_main_persistent_hook;
	};

	inline hooking *g_hooking{};
}

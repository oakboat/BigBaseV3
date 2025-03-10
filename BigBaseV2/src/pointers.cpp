#include "common.hpp"
#include "logger.hpp"
#include "pointers.hpp"
#include "memory/all.hpp"
#include "kiero.h"

namespace big
{
	pointers::pointers()
	{
		memory::pattern_batch main_batch;

		main_batch.add("Game state", "83 3D ? ? ? ? 00 78 15", [this](memory::handle ptr)
		{
			m_game_state = ptr.add(2).rip().as<eGameState*>();
		});

		main_batch.add("Is session started", "80 3D ? ? ? ? ? 66 0F 7E DA 0F 44 C1", [this](memory::handle ptr)
		{
			m_is_session_started = ptr.add(2).rip().as<bool*>();
		});

		main_batch.add("Ped factory", "48 8B 05 ? ? ? ? 48 8B 40 ? B3", [this](memory::handle ptr)
		{
			m_ped_factory = ptr.add(3).rip().as<CPedFactory**>();
		});

		main_batch.add("Network player manager", "48 8B 0D ? ? ? ? E8 ? ? ? ? 48 85 C0 74 C3", [this](memory::handle ptr)
		{
			m_network_player_mgr = ptr.add(3).rip().as<CNetworkPlayerMgr**>();
		});

		main_batch.add("Init native tables", "E8 ? ? ? ? 4C 89 E1 E8 ? ? ? ? 48 8D 35 ? ? ? ?", [this](memory::handle ptr)
		{
				m_init_native_tables = ptr.add(1).rip().as<functions::init_native_tables_t>();
		});

		/*main_batch.add("Fix vectors", "83 79 18 00 48 8B D1 74 4A FF 4A 18 48 63 4A 18 48 8D 41 04 48 8B 4C CA", [this](memory::handle ptr)
		{
			m_fix_vectors = ptr.as<functions::fix_vectors_t>();
		});*/

		main_batch.add("Script threads", "56 57 55 53 48 83 EC 28 85 C9 BE ? ? ? ? 0F 45 F1 0F B7 05 ? ? ? ?", [this](memory::handle ptr)
		{
			m_script_threads = ptr.add(0x25).rip().as<decltype(m_script_threads)>();
			m_run_script_threads = ptr.as<functions::run_script_threads_t>();
		});

		main_batch.add("Script programs", "48 03 15 ? ? ? ? 48 85 CA 74 28", [this](memory::handle ptr)
		{
			m_script_program_table = ptr.add(3).rip().as<decltype(m_script_program_table)>();
		});

		main_batch.add("Script globals", "48 8D 15 ? ? ? ? 49 89 D8 4D 89 F1", [this](memory::handle ptr)
		{
			m_script_globals = ptr.add(3).rip().as<std::int64_t**>();
		});

		/*main_batch.add("CGameScriptHandlerMgr", "48 8B 0D ? ? ? ? 4C 8B CE E8 ? ? ? ? 48 85 C0 74 05 40 32 FF", [this](memory::handle ptr)
		{
			m_script_handler_mgr = ptr.add(3).rip().as<CGameScriptHandlerMgr**>();
		});*/

		/*main_batch.add("Swapchain", "48 8B 0D ? ? ? ? 48 8B 01 44 8D 43 01 33 D2 FF 50 40 8B C8", [this](memory::handle ptr)
		{
			m_swapchain = ptr.add(3).rip().as<IDXGISwapChain**>();
		}); */

		main_batch.run(memory::module(nullptr));

		m_hwnd = FindWindowW(L"sgaWindow", nullptr);
		if (!m_hwnd)
			throw std::runtime_error("Failed to find the game's window.");

		if (kiero::init(kiero::RenderType::D3D12) != kiero::Status::Success)
			throw std::runtime_error("Failed to init kiero.");

 		g_pointers = this;
	}

	pointers::~pointers()
	{
		g_pointers = nullptr;
		kiero::shutdown();
	}
}

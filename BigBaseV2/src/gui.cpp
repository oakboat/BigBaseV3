#include "common.hpp"
#include "fiber_pool.hpp"
#include "gta/player.hpp"
#include "gta_util.hpp"
#include "gui.hpp"
#include "logger.hpp"
#include "memory/module.hpp"
#include "memory/pattern.hpp"
#include "natives.hpp"
#include "pointers.hpp"
#include "renderer.hpp"
#include "script.hpp"
#include "features.hpp"

#include <imgui.h>
#include <StackWalker.h>
#include <script_global.hpp>

namespace big
{
	void gui::dx_init()
	{
		auto& style = ImGui::GetStyle();
		style.WindowPadding = { 10.f, 10.f };
		style.PopupRounding = 0.f;
		style.FramePadding = { 8.f, 4.f };
		style.ItemSpacing = { 10.f, 8.f };
		style.ItemInnerSpacing = { 6.f, 6.f };
		style.TouchExtraPadding = { 0.f, 0.f };
		style.IndentSpacing = 21.f;
		style.ScrollbarSize = 15.f;
		style.GrabMinSize = 8.f;
		style.WindowBorderSize = 1.f;
		style.ChildBorderSize = 0.f;
		style.PopupBorderSize = 1.f;
		style.FrameBorderSize = 0.f;
		style.TabBorderSize = 0.f;
		style.WindowRounding = 0.f;
		style.ChildRounding = 0.f;
		style.FrameRounding = 0.f;
		style.ScrollbarRounding = 0.f;
		style.GrabRounding = 0.f;
		style.TabRounding = 0.f;
		style.WindowTitleAlign = { 0.5f, 0.5f };
		style.ButtonTextAlign = { 0.5f, 0.5f };
		style.DisplaySafeAreaPadding = { 3.f, 3.f };

		auto& colors = style.Colors;
		colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
		colors[ImGuiCol_TextDisabled] = ImVec4(1.00f, 0.90f, 0.19f, 1.00f);
		colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
		colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
		colors[ImGuiCol_Border] = ImVec4(0.30f, 0.30f, 0.30f, 0.50f);
		colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_FrameBg] = ImVec4(0.21f, 0.21f, 0.21f, 0.54f);
		colors[ImGuiCol_FrameBgHovered] = ImVec4(0.21f, 0.21f, 0.21f, 0.78f);
		colors[ImGuiCol_FrameBgActive] = ImVec4(0.28f, 0.27f, 0.27f, 0.54f);
		colors[ImGuiCol_TitleBg] = ImVec4(0.17f, 0.17f, 0.17f, 1.00f);
		colors[ImGuiCol_TitleBgActive] = ImVec4(0.19f, 0.19f, 0.19f, 1.00f);
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
		colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
		colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
		colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
		colors[ImGuiCol_CheckMark] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
		colors[ImGuiCol_SliderGrab] = ImVec4(0.34f, 0.34f, 0.34f, 1.00f);
		colors[ImGuiCol_SliderGrabActive] = ImVec4(0.39f, 0.38f, 0.38f, 1.00f);
		colors[ImGuiCol_Button] = ImVec4(0.41f, 0.41f, 0.41f, 0.74f);
		colors[ImGuiCol_ButtonHovered] = ImVec4(0.41f, 0.41f, 0.41f, 0.78f);
		colors[ImGuiCol_ButtonActive] = ImVec4(0.41f, 0.41f, 0.41f, 0.87f);
		colors[ImGuiCol_Header] = ImVec4(0.37f, 0.37f, 0.37f, 0.31f);
		colors[ImGuiCol_HeaderHovered] = ImVec4(0.38f, 0.38f, 0.38f, 0.37f);
		colors[ImGuiCol_HeaderActive] = ImVec4(0.37f, 0.37f, 0.37f, 0.51f);
		colors[ImGuiCol_Separator] = ImVec4(0.38f, 0.38f, 0.38f, 0.50f);
		colors[ImGuiCol_SeparatorHovered] = ImVec4(0.46f, 0.46f, 0.46f, 0.50f);
		colors[ImGuiCol_SeparatorActive] = ImVec4(0.46f, 0.46f, 0.46f, 0.64f);
		colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.26f, 0.26f, 1.00f);
		colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
		colors[ImGuiCol_ResizeGripActive] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
		colors[ImGuiCol_Tab] = ImVec4(0.21f, 0.21f, 0.21f, 0.86f);
		colors[ImGuiCol_TabHovered] = ImVec4(0.27f, 0.27f, 0.27f, 0.86f);
		colors[ImGuiCol_TabActive] = ImVec4(0.34f, 0.34f, 0.34f, 0.86f);
		colors[ImGuiCol_TabUnfocused] = ImVec4(0.10f, 0.10f, 0.10f, 0.97f);
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
		colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
		colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
		colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
		colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
		colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
		colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
		colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
		colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
		colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
	}

	

	void gui::dx_on_tick()
	{
		if (ImGui::Begin("BigBaseV3"))
		{
			static bool godemode = false;
			if (ImGui::Checkbox("Godmode", &godemode))
			{
				features::set_godmode(godemode);
			}
			ImGui::SameLine();
			if (ImGui::Button("Heal"))
			{
				g_fiber_pool->queue_job([]
					{
						auto ped = PLAYER::PLAYER_PED_ID();
						auto id = PLAYER::PLAYER_ID();
						ENTITY::SET_ENTITY_HEALTH(ped, PED::GET_PED_MAX_HEALTH(ped), 0, 0);
						PED::SET_PED_ARMOUR(ped, PLAYER::GET_PLAYER_MAX_ARMOUR(id));
					});
			}
			ImGui::SameLine();
			if (ImGui::Button("Skip cutscene"))
			{
				g_fiber_pool->queue_job([]
					{
						CUTSCENE::STOP_CUTSCENE_IMMEDIATELY();
					});
			}
			ImGui::SameLine();
			if (ImGui::Button("Clear Wanted"))
			{
				g_fiber_pool->queue_job([]
					{
						auto id = PLAYER::PLAYER_ID();
						PLAYER::SET_PLAYER_WANTED_LEVEL(id, 0, FALSE);
						PLAYER::SET_PLAYER_WANTED_LEVEL_NOW(id, FALSE);
					});
			}

			static int wanted_level = 0;
			if (ImGui::SliderInt("Wanted level", &wanted_level, 0, 5))
			{
				g_fiber_pool->queue_job([]
					{
						auto id = PLAYER::PLAYER_ID();
						PLAYER::SET_PLAYER_WANTED_LEVEL(id, wanted_level, FALSE);
						PLAYER::SET_PLAYER_WANTED_LEVEL_NOW(id, FALSE);
					});
			}
			

			if (ImGui::Button("Spawn Kuruma"))
			{
				g_fiber_pool->queue_job([]
					{
						constexpr auto hash = RAGE_JOAAT("kuruma2");
						while (!STREAMING::HAS_MODEL_LOADED(hash))
						{
							STREAMING::REQUEST_MODEL(hash);
							script::get_current()->yield();
						}

						auto pos = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), true);
						auto vehicle = VEHICLE::CREATE_VEHICLE(hash, pos.x, pos.y, pos.z, 0.f, true, true, false);

						if (*g_pointers->m_is_session_started)
						{
							DECORATOR::DECOR_SET_INT(vehicle, "MPBitset", 0);
							auto networkId = NETWORK::VEH_TO_NET(vehicle);
							if (NETWORK::NETWORK_GET_ENTITY_IS_NETWORKED(vehicle))
								NETWORK::SET_NETWORK_ID_EXISTS_ON_ALL_MACHINES(networkId, TRUE);
							VEHICLE::SET_VEHICLE_IS_STOLEN(vehicle, FALSE);
						}

						STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(hash);
					});
			}
			ImGui::SameLine();
			if (ImGui::Button("Teleport waypoint"))
			{
				g_fiber_pool->queue_job([]
					{
						Blip blip = HUD::GET_CLOSEST_BLIP_INFO_ID(8);
						if (!HUD::DOES_BLIP_EXIST(blip)) return;
						auto location = HUD::GET_BLIP_COORDS(blip);
						constexpr float max_ground_check = 1000.f;
						constexpr int max_attempts = 300;
						float ground_z = location.z;
						int current_attempts = 0;
						bool found_ground;
						float height;
						do
						{
							found_ground = MISC::GET_GROUND_Z_FOR_3D_COORD(location.x, location.y, max_ground_check, &ground_z, FALSE, FALSE);
							STREAMING::REQUEST_COLLISION_AT_COORD(location.x, location.y, location.z);

							if (current_attempts % 10 == 0)
							{
								location.z += 25.f;
							}

							++current_attempts;

							script::get_current()->yield();
						} while (!found_ground && current_attempts < max_attempts);
						if (!found_ground)
						{
							return;
						}
						if (WATER::GET_WATER_HEIGHT(location.x, location.y, location.z, &height))
						{
							location.z = height;
						}
						else
						{
							location.z = ground_z + 1.f;
						}
						PED::SET_PED_COORDS_KEEP_VEHICLE(PLAYER::PLAYER_PED_ID(), location.x, location.y, location.z + 1.f);
					});
			}
			ImGui::SameLine();
			if (ImGui::Button("Teleport objective"))
			{
				g_fiber_pool->queue_job([]
					{
						Blip blip = HUD::GET_CLOSEST_BLIP_INFO_ID(1);
						while (HUD::DOES_BLIP_EXIST(blip))
						{
							int blipColour = HUD::GET_BLIP_COLOUR(blip);
							if (blipColour == 5 || blipColour == 60 || blipColour == 66)
							{
								auto location = HUD::GET_BLIP_COORDS(blip);
								PED::SET_PED_COORDS_KEEP_VEHICLE(PLAYER::PLAYER_PED_ID(), location.x, location.y, location.z + 1.f);
							}
							blip = HUD::GET_NEXT_BLIP_INFO_ID(1);
						}
					});
			}	

			if (ImGui::Button("Complete apartment"))
			{
				g_fiber_pool->queue_job([]
					{
						auto id = 0;
						STATS::STAT_GET_INT(rage::joaat("MPPLY_LAST_MP_CHAR"), &id, -1);
						std::string prefix = "MP" + std::to_string(id) + "_";
						STATS::STAT_SET_INT(rage::joaat(prefix + "HEIST_PLANNING_STAGE"), -1, true);
					});
			}

			static char apartment_cuts[100]{ 0 };
			ImGui::InputText("", apartment_cuts, IM_ARRAYSIZE(apartment_cuts), ImGuiInputTextFlags_CharsDecimal);
			ImGui::SameLine();
			if (ImGui::Button("Apartment cuts"))
			{
				g_fiber_pool->queue_job([]
					{
						int cuts = atoi(apartment_cuts);
						*big::script_global(1929796 + 0).as<int*>() = cuts * -4 + 100;
						*big::script_global(1929796 + 1).as<int*>() = cuts;
						*big::script_global(1929796 + 2).as<int*>() = cuts;
						*big::script_global(1929796 + 3).as<int*>() = cuts;
						PAD::SET_CURSOR_POSITION(0.775, 0.175);
						PAD::SET_CONTROL_VALUE_NEXT_FRAME(0, 237, 1);
						PAD::SET_CONTROL_VALUE_NEXT_FRAME(2, 202, 1);
						script::get_current()->yield(500ms);
						*big::script_global(1934771 + 0).as<int*>() = cuts;
					});
			}

			ImGui::Separator();

			if (ImGui::Button("Apply stats file"))
			{
				features::apply_stats_file();
			}

			static char global[100]{ 0 };
			static char value[100]{ 0 };
			ImGui::InputText("Global index", global, IM_ARRAYSIZE(global), ImGuiInputTextFlags_CharsDecimal);
			ImGui::InputText("Global value", value, IM_ARRAYSIZE(value), ImGuiInputTextFlags_CharsDecimal);
			if (ImGui::Button("Set global"))
			{
				g_fiber_pool->queue_job([]
					{
						*big::script_global(atoi(global)).as<int*>() = atoi(value);
					});
			}


			ImGui::Separator();

			if (ImGui::Button("Unload"))
			{
				g_running = false;
			}
		}
		ImGui::End();
	}

	void gui::script_init()
	{
	}

	void gui::script_on_tick()
	{
		if (g_gui.m_opened)
		{
			PAD::DISABLE_ALL_CONTROL_ACTIONS(0);
		}
	}

	void gui::script_func()
	{
		g_gui.script_init();
		while (true)
		{
			g_gui.script_on_tick();
			script::get_current()->yield();
		}
	}
}

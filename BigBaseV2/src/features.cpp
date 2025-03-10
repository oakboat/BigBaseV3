#include "common.hpp"
#include "features.hpp"
#include "logger.hpp"
#include "natives.hpp"
#include "script.hpp"
#include "fiber_pool.hpp"
#include "gta/joaat.hpp"

namespace big
{
	static int wanted_level;
	static bool godemode = false;
	static bool godemoderest = false;
	static std::unordered_map<std::string, int> intStats;
	static std::unordered_map<std::string, bool> boolStats;

	void features::apply_stats_file()
	{
		// 获取当前玩家的ID
		auto id = 0;
		STATS::STAT_GET_INT(rage::joaat("MPPLY_LAST_MP_CHAR"), &id, -1);

		// 计算前缀
		std::string prefix = "MP" + std::to_string(id) + "_";

		// 获取 AppData 目录
		std::string appData = std::getenv("appdata");
		std::ifstream file(appData + "/stats.txt");

		if (!file) {
			LOG_INFO("Cant open file: stats.txt");
			return;
		}

		std::string line;
		std::string key;

		// 读取文件内容
		while (std::getline(file, line)) {
			// 检查是否以 $MPX_ / $MP0_ / $MP1_ 开头
			if (line.rfind("$MPX_", 0) == 0 || line.rfind("$MP0_", 0) == 0 || line.rfind("$MP1_", 0) == 0) {
				key = line.substr(1);  // 去掉 '$'

				// 替换 "MPX_" 为实际 prefix
				if (key.rfind("MPX_", 0) == 0) {
					key.replace(0, 4, prefix);
				}

				if (std::getline(file, line)) {  // 读取下一行的数值
					try {
						// 处理布尔值 (TRUE 或 FALSE)
						if (line == "TRUE" || line == "FALSE") {
							boolStats[key] = (line == "TRUE");
						}
						else {
							// 处理整数值
							intStats[key] = std::stoi(line);
						}
					}
					catch (...) {
						LOG_INFO("Cant get value: {}", line);
					}
				}
			}
		}

		file.close();
		g_fiber_pool->queue_job([]
			{
				// 设置整数类型的统计数据
				for (const auto& [stat_key, value] : intStats) {
					STATS::STAT_SET_INT(rage::joaat(stat_key), value, true);
					LOG_INFO("Set stat (int): {} = {}", stat_key, value);
					script::get_current()->yield();
				}
				intStats.clear();

				// 设置布尔类型的统计数据
				for (const auto& [stat_key, value] : boolStats) {
					STATS::STAT_SET_BOOL(rage::joaat(stat_key), value, true);
					LOG_INFO("Set stat (bool): {} = {}", stat_key, value);
					script::get_current()->yield();
				}
				boolStats.clear();
			});
	}

	void features::set_godmode(bool enable)
	{
		godemode = enable;
		if (!enable)
		{
			godemoderest = false;
		}
	}

	void features::run_tick()
	{
		
		if (godemode)
		{
			ENTITY::SET_ENTITY_INVINCIBLE(PLAYER::PLAYER_PED_ID(), true, 0);
		}
		else if(!godemoderest)
		{
			ENTITY::SET_ENTITY_INVINCIBLE(PLAYER::PLAYER_PED_ID(), false, 0);
			godemoderest = true;
		}
	}

	void features::script_func()
	{
		while (true)
		{
			run_tick();
			script::get_current()->yield();
		}
	}
}

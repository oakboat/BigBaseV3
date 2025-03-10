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
		// ��ȡ��ǰ��ҵ�ID
		auto id = 0;
		STATS::STAT_GET_INT(rage::joaat("MPPLY_LAST_MP_CHAR"), &id, -1);

		// ����ǰ׺
		std::string prefix = "MP" + std::to_string(id) + "_";

		// ��ȡ AppData Ŀ¼
		std::string appData = std::getenv("appdata");
		std::ifstream file(appData + "/stats.txt");

		if (!file) {
			LOG_INFO("Cant open file: stats.txt");
			return;
		}

		std::string line;
		std::string key;

		// ��ȡ�ļ�����
		while (std::getline(file, line)) {
			// ����Ƿ��� $MPX_ / $MP0_ / $MP1_ ��ͷ
			if (line.rfind("$MPX_", 0) == 0 || line.rfind("$MP0_", 0) == 0 || line.rfind("$MP1_", 0) == 0) {
				key = line.substr(1);  // ȥ�� '$'

				// �滻 "MPX_" Ϊʵ�� prefix
				if (key.rfind("MPX_", 0) == 0) {
					key.replace(0, 4, prefix);
				}

				if (std::getline(file, line)) {  // ��ȡ��һ�е���ֵ
					try {
						// ������ֵ (TRUE �� FALSE)
						if (line == "TRUE" || line == "FALSE") {
							boolStats[key] = (line == "TRUE");
						}
						else {
							// ��������ֵ
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
				// �����������͵�ͳ������
				for (const auto& [stat_key, value] : intStats) {
					STATS::STAT_SET_INT(rage::joaat(stat_key), value, true);
					LOG_INFO("Set stat (int): {} = {}", stat_key, value);
					script::get_current()->yield();
				}
				intStats.clear();

				// ���ò������͵�ͳ������
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

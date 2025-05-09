#pragma once
#include <cstdint>
#include "fwddec.hpp"
#include "joaat.hpp"
#include "tls_context.hpp"
#include "scrValue.hpp"

namespace rage
{
	enum class eThreadState : std::uint32_t
	{
		idle,
		running,
		killed,
		unk_3,
		unk_4,
	};

	class scrThreadContext
	{
	public:
		std::uint32_t m_thread_id;           // 0x08
		char pad0[4];                        // 0x0C
		joaat_t m_script_hash;               // 0x10
		char pad1[4];                        // 0x14
		eThreadState m_state;                // 0x18
		std::uint32_t m_instruction_pointer; // 0x1C
		std::uint32_t m_frame_pointer;       // 0x20
		std::uint32_t m_stack_pointer;       // 0x24
		float m_timer_a;                     // 0x28
		float m_timer_b;                     // 0x2C
		float m_wait_timer;                  // 0x30
		char m_padding1[0x2C];               // 0x34
		std::uint32_t m_stack_size;          // 0x60
		char m_padding2[0x54];               // 0x64
	};

	class scrThread
	{
	public:
		virtual ~scrThread() = default;                                                                 // 0 (0x00)
		virtual void reset(std::uint32_t script_hash, void* args, std::uint32_t arg_count) = 0;         // 1 (0x08)
		virtual eThreadState run() = 0;                                                                 // 2 (0x10)
		virtual eThreadState tick(std::uint32_t ops_to_execute) = 0;                                    // 3 (0x18)
		virtual void kill() = 0;

	public:
		scrThreadContext m_context;                 // 0x08
		scrValue* m_stack;                          // 0xB8
		char m_padding[0x4];                        // 0xC0
		uint32_t m_arg_size;                        // 0xC4
		uint32_t m_arg_loc;                         // 0xC8
		char m_padding2[0x4];                       // 0xCC
		const char* m_exit_message;                 // 0xD0
		char m_pad[0x7C];                           // 0xD8
		char m_name[0x44];                          // 0x154
		scriptHandler* m_handler;                   // 0x198
		scriptHandlerNetComponent* m_net_component; // 0x1A0
	};

	static_assert(sizeof(scrThreadContext) == 0xB0);
	static_assert(sizeof(scrThread) == 0x1A8);
}

class GtaThread : public rage::scrThread
{
public:
	rage::joaat_t m_script_hash;               // 0x1A8
	char m_padding3[0x14];                     // 0x124
	std::int32_t m_instance_id;                // 0x138
	char m_padding4[0x04];                     // 0x13C
	uint8_t m_flag1;                      // 0x140
	bool m_safe_for_network_game;              // 0x141
	char m_padding5[0x02];                     // 0x142
	bool m_is_minigame_script;                 // 0x144
	char m_padding6[0x02];                     // 0x145
	bool m_can_be_paused;                      // 0x147
	bool m_can_remove_blips_from_other_scripts;// 0x148
	char m_padding7[0x0F];                     // 0x149
};

//static_assert(sizeof(GtaThread) == 0x160);

#include "common.hpp"
#include "crossmap.hpp"
#include "invoker.hpp"
#include "logger.hpp"
#include "pointers.hpp"

namespace big
{
	native_call_context::native_call_context()
	{
		m_return_value = &m_return_stack[0];
		m_args = &m_arg_stack[0];
	}

	void native_invoker::cache_handlers()
	{
		struct HashCache
		{
			char pad0[0x2c];
			uint32_t count;
			char pad1[0x10];
			uint64_t * data;
		};
		HashCache hashcache{};
		hashcache.count = sizeof(g_crossmap) / sizeof(rage::scrNativeMapping);
		hashcache.data = new uint64_t[hashcache.count];

		for (size_t i = 0; i < hashcache.count; i++)
		{
			auto& mapping = g_crossmap[i];
			hashcache.data[i] = mapping.second;
		}

		g_pointers->m_init_native_tables(&hashcache);

		for (size_t i = 0; i < hashcache.count; i++)
		{
			auto& mapping = g_crossmap[i];
			m_handler_cache.emplace(mapping.first, reinterpret_cast<rage::scrNativeHandler>(hashcache.data[i]));
			//LOG_INFO("{:X}\t\t{:X}", (uint64_t)mapping.first, (uint64_t)hashcache.data[i]);
		}

		delete[] hashcache.data;
	}

	void native_invoker::begin_call()
	{
		m_call_context.reset();
	}

	int64_t fix_vectors(void* a1) {
		auto base = reinterpret_cast<uintptr_t>(a1);
		int64_t v2;
		int64_t v3;
		int64_t result = 0;

		while (*reinterpret_cast<int32_t*>(base + 24)) {
			--(*reinterpret_cast<int32_t*>(base + 24));

			int index = *reinterpret_cast<int32_t*>(base + 24);
			int64_t ptr = *reinterpret_cast<int64_t*>(base + 8LL * index + 32);

			*reinterpret_cast<int32_t*>(ptr) = *reinterpret_cast<int32_t*>(base + 16 * (index + 4));
			*reinterpret_cast<int32_t*>(ptr + 8) = *reinterpret_cast<int32_t*>(base + 16LL * index + 68);

			v2 = index;
			v3 = *reinterpret_cast<int64_t*>(base + 8 * v2 + 32);
			result = *reinterpret_cast<uint32_t*>(base + 16 * v2 + 72);

			*reinterpret_cast<int32_t*>(v3 + 16) = static_cast<int32_t>(result);
		}

		--(*reinterpret_cast<int32_t*>(base + 24));
		return result;
	}

	void native_invoker::end_call(rage::scrNativeHash hash)
	{
		if (auto it = m_handler_cache.find(hash); it != m_handler_cache.end())
		{
			rage::scrNativeHandler handler = it->second;

			__try
			{
				handler(&m_call_context);
				fix_vectors(&m_call_context);
			}
			__except (EXCEPTION_EXECUTE_HANDLER)
			{
				LOG_ERROR("Exception caught while trying to call 0x{:X} native.", hash);
			}
		}
		else
		{
			LOG_ERROR("Failed to find 0x{:X} native's handler.", hash);
		}
	}
}

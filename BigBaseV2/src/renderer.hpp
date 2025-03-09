#pragma once
#include "common.hpp"
#include <imgui.h>

namespace big
{
	class renderer
	{
	public:
		explicit renderer();
		~renderer();

		void on_present(IDXGISwapChain3* pSwapChain);
		void on_ExecuteCommandLists(ID3D12CommandQueue* queue);
		void pre_reset();
		void post_reset();

		void wndproc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	public:
		ImFont *m_font;
		ImFont *m_monospace_font;
	private:
		ID3D12Device* Device = nullptr;
		ID3D12DescriptorHeap* DescriptorHeapBackBuffers = nullptr;
		ID3D12DescriptorHeap* DescriptorHeapImGuiRender = nullptr;
		ID3D12GraphicsCommandList* CommandList = nullptr;
		ID3D12CommandQueue* CommandQueue = nullptr;

		struct _FrameContext {
			ID3D12CommandAllocator* CommandAllocator;
			ID3D12Resource* Resource;
			D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHandle;
		};

		uint64_t BuffersCounts = -1;
		_FrameContext* FrameContext = nullptr;
		bool ImGui_Initialised = false;
	};

	inline renderer *g_renderer{};
}

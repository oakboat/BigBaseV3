#include "common.hpp"
#include "fonts.hpp"
#include "logger.hpp"
#include "gui.hpp"
#include "pointers.hpp"
#include "renderer.hpp"
#include <imgui.h>
#include <imgui_impl_dx12.h>
#include <imgui_impl_win32.h>

IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace big
{
	renderer::renderer()
	{
		g_renderer = this;
	}

	renderer::~renderer()
	{
		ImGui_ImplWin32_Shutdown();
		ImGui_ImplDX12_Shutdown();
		ImGui::DestroyContext();

		g_renderer = nullptr;
	}

	void renderer::on_present(IDXGISwapChain3* pSwapChain)
	{
		if (!ImGui_Initialised) {
			if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D12Device), (void**)&Device))) {
				ImGui::CreateContext();

				ImGuiIO& io = ImGui::GetIO(); (void)io;

				DXGI_SWAP_CHAIN_DESC Desc;
				pSwapChain->GetDesc(&Desc);
				Desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
				Desc.OutputWindow = g_pointers->m_hwnd;
				Desc.Windowed = ((GetWindowLongPtr(g_pointers->m_hwnd, GWL_STYLE) & WS_POPUP) != 0) ? false : true;

				BuffersCounts = Desc.BufferCount;
				FrameContext = new _FrameContext[BuffersCounts];

				D3D12_DESCRIPTOR_HEAP_DESC DescriptorImGuiRender = {};
				DescriptorImGuiRender.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
				DescriptorImGuiRender.NumDescriptors = BuffersCounts;
				DescriptorImGuiRender.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

				if (Device->CreateDescriptorHeap(&DescriptorImGuiRender, IID_PPV_ARGS(&DescriptorHeapImGuiRender)) != S_OK)
					throw("CreateDescriptorHeap");

				ID3D12CommandAllocator* Allocator;
				if (Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&Allocator)) != S_OK)
					throw("CreateCommandAllocator");

				for (size_t i = 0; i < BuffersCounts; i++) {
					FrameContext[i].CommandAllocator = Allocator;
				}

				if (Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, Allocator, NULL, IID_PPV_ARGS(&CommandList)) != S_OK ||
					CommandList->Close() != S_OK)
					throw("CreateCommandList");

				D3D12_DESCRIPTOR_HEAP_DESC DescriptorBackBuffers;
				DescriptorBackBuffers.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
				DescriptorBackBuffers.NumDescriptors = BuffersCounts;
				DescriptorBackBuffers.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
				DescriptorBackBuffers.NodeMask = 1;

				if (Device->CreateDescriptorHeap(&DescriptorBackBuffers, IID_PPV_ARGS(&DescriptorHeapBackBuffers)) != S_OK)
					throw("CreateDescriptorHeap");

				const auto RTVDescriptorSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
				D3D12_CPU_DESCRIPTOR_HANDLE RTVHandle = DescriptorHeapBackBuffers->GetCPUDescriptorHandleForHeapStart();

				for (size_t i = 0; i < BuffersCounts; i++) {
					ID3D12Resource* pBackBuffer = nullptr;
					FrameContext[i].DescriptorHandle = RTVHandle;
					pSwapChain->GetBuffer(i, IID_PPV_ARGS(&pBackBuffer));
					Device->CreateRenderTargetView(pBackBuffer, nullptr, RTVHandle);
					FrameContext[i].Resource = pBackBuffer;
					RTVHandle.ptr += RTVDescriptorSize;
				}

				ImGui_ImplWin32_Init(g_pointers->m_hwnd);
				ImGui_ImplDX12_Init(Device, BuffersCounts, DXGI_FORMAT_R8G8B8A8_UNORM, DescriptorHeapImGuiRender, DescriptorHeapImGuiRender->GetCPUDescriptorHandleForHeapStart(), DescriptorHeapImGuiRender->GetGPUDescriptorHandleForHeapStart());
				ImGui_ImplDX12_CreateDeviceObjects();
				ImGui::GetMainViewport()->PlatformHandleRaw = g_pointers->m_hwnd;

				ImFontConfig font_cfg{};
				font_cfg.FontDataOwnedByAtlas = false;
				std::strcpy(font_cfg.Name, "Rubik");

				m_font = ImGui::GetIO().Fonts->AddFontFromMemoryTTF(const_cast<std::uint8_t*>(font_rubik), sizeof(font_rubik), 20.f, &font_cfg);
				m_monospace_font = ImGui::GetIO().Fonts->AddFontDefault();

				g_gui.dx_init();
			}
			ImGui_Initialised = true;
		}

		if (CommandQueue == nullptr)
			return;

		if (g_gui.m_opened)
		{
			ImGui::GetIO().MouseDrawCursor = true;
			ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
		}
		else
		{
			ImGui::GetIO().MouseDrawCursor = false;
			ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouse;
		}

		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		if (g_gui.m_opened)
		{
			g_gui.dx_on_tick();
		}

		ImGui::EndFrame();

		_FrameContext& CurrentFrameContext = FrameContext[pSwapChain->GetCurrentBackBufferIndex()];
		CurrentFrameContext.CommandAllocator->Reset();

		D3D12_RESOURCE_BARRIER Barrier;
		Barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		Barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		Barrier.Transition.pResource = CurrentFrameContext.Resource;
		Barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		Barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
		Barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

		CommandList->Reset(CurrentFrameContext.CommandAllocator, nullptr);
		CommandList->ResourceBarrier(1, &Barrier);
		CommandList->OMSetRenderTargets(1, &CurrentFrameContext.DescriptorHandle, FALSE, nullptr);
		CommandList->SetDescriptorHeaps(1, &DescriptorHeapImGuiRender);

		ImGui::Render();
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), CommandList);
		Barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		Barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
		CommandList->ResourceBarrier(1, &Barrier);
		CommandList->Close();
		CommandQueue->ExecuteCommandLists(1, reinterpret_cast<ID3D12CommandList* const*>(&CommandList));
	}

	void renderer::on_ExecuteCommandLists(ID3D12CommandQueue* queue)
	{
		if (!CommandQueue) CommandQueue = queue;
	}

	void renderer::pre_reset()
	{
		ImGui_ImplDX12_InvalidateDeviceObjects();
	}

	void renderer::post_reset()
	{
		ImGui_ImplDX12_CreateDeviceObjects();
	}

	void renderer::wndproc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
	{
		if (msg == WM_KEYUP && wparam == VK_INSERT)
			g_gui.m_opened ^= true;

		if (g_gui.m_opened)
		{
			ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam);
		}
	}
}

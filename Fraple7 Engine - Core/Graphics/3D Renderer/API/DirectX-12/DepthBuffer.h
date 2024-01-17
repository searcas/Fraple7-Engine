#pragma once
#include "Utilities/Common/Common.h"
#include "directx/d3dx12.h"
#include "Command.h"
namespace Fraple7
{
	namespace Core
	{
		class DepthBuffer
		{
		public:
			DepthBuffer(const ComPtr<ID3D12Device2>& device, const class Window& window, std::shared_ptr<CommandMgr> commandMgr);
			~DepthBuffer();
			void Init();
			const CD3DX12_CPU_DESCRIPTOR_HANDLE& GetDSVHandle() { return m_DsvHandle; }
			void ResizeDepthBuffer();
			void SetInitComplete(bool init) { m_InitCompleted = init; }
		private:
			void Create();
			void DescriptorHeap();
			void CreateDepthStencilView();

		private:
			ComPtr<ID3D12Resource> m_DepthBuffer;
			ComPtr<ID3D12DescriptorHeap> m_DsvDescriptorHeap;
			const ComPtr<ID3D12Device2>& m_Device;
			CD3DX12_CPU_DESCRIPTOR_HANDLE m_DsvHandle;
			class WinWindow& m_Window;
			bool m_InitCompleted = false;
			std::shared_ptr<CommandMgr> m_CommandMgr;
		};


	}
}
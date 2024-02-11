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
			DepthBuffer(const std::shared_ptr<Studio::Window>& window, std::shared_ptr<CommandMgr> commandMgr);
			~DepthBuffer();
			void Init();
			const CD3DX12_CPU_DESCRIPTOR_HANDLE& GetDSVHandle() { return m_DsvHandle; }
			void ResizeDepthBuffer();
			void Create();
			void InitDescriptorHeap();
			void CreateDepthStencilView();
		private:

		private:
			ComPtr<ID3D12Resource> m_DepthBuffer;
			ComPtr<ID3D12DescriptorHeap> m_DsvDescriptorHeap;
			CD3DX12_CPU_DESCRIPTOR_HANDLE m_DsvHandle;

			std::shared_ptr<Studio::WinWindow> m_Window;
			std::shared_ptr<CommandMgr> m_CommandMgr;
		};


	}
}
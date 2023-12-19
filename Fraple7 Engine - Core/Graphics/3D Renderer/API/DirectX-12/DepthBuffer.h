#pragma once
#include "Utilities/Common/Common.h"
#include "directx/d3dx12.h"
namespace Fraple7
{
	namespace Core
	{
		class DepthBuffer
		{
		public:
			DepthBuffer(const ComPtr<ID3D12Device2>& device, const class Window& window);

			~DepthBuffer();
			void Init();
			const CD3DX12_CPU_DESCRIPTOR_HANDLE& GetDSVHandle() { return m_DsvHandle; }
		private:
			void Create();
			void DescriptorHeap();
			void CreateDepthStencilView();
		private:
			ComPtr<ID3D12Resource> m_DepthBuffer;
			ComPtr<ID3D12DescriptorHeap> m_DsvDescriptorHeap;
			const ComPtr<ID3D12Device2>& m_Device;
			CD3DX12_CPU_DESCRIPTOR_HANDLE m_DsvHandle;
			const class Window& m_Window;
		};


	}
}
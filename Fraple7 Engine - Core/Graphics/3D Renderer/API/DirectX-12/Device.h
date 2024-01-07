#pragma once
#include "directx/d3dx12.h"
#include "Utilities/Common/Common.h"
#include "dxgi1_6.h"

namespace Fraple7
{
	namespace Core
	{
		class Device
		{
		public:
			Device();
			~Device();
			void Construct();
			void CreateAdapter();
			void Destroy();
			uint32_t Validate();
			D3D_FEATURE_LEVEL CheckFeatureLevel();
		public:
			const ComPtr<ID3D12Device2>& GetDevice() const { return m_Device; }
			const ComPtr<IDXGIFactory4>& GetDXGIFactory() const { return m_DxGiFactory; }
			SIZE_T GetMaxDedicatedVideoMemory() const { return m_MaxDedicatedVideoMemory; }
		private:
			ComPtr<ID3D12Device2> m_Device;
			ComPtr<IDXGIFactory4> m_DxGiFactory;

			ComPtr<IDXGIAdapter1> m_DxgiAdapter1;
			ComPtr<IDXGIAdapter4> m_DxgiAdapter4;
			SIZE_T m_MaxDedicatedVideoMemory = 0;
		};
	}
}
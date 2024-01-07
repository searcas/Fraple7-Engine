#include "pch.h"
#include "Device.h"


namespace Fraple7
{
	namespace Core
	{
#ifdef _DEBUG
		static inline void DebugLayer()
		{
			ComPtr<ID3D12Debug1> debugController;
			D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)) >> statusCode;
			debugController->EnableDebugLayer();
			debugController->SetEnableGPUBasedValidation(true);
		}
#else 
#define DebugLayer();
#endif
		Device::Device()
		{
			DebugLayer();
			Construct();
		}
		Device::~Device()
		{
			//Destroy();
		}
		void Device::Construct()
		{
			CreateAdapter();
			if (SUCCEEDED(Validate()))
				D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_Device)) >> statusCode;
			else std::runtime_error{ "Failed to create Device" };
			
		}
		void Device::Destroy()
		{
			m_Device->Release();
		}
		void Device::CreateAdapter()
		{
			UINT flags = 0;
#ifdef _DEBUG
			flags = DXGI_CREATE_FACTORY_DEBUG;
#endif // _DEBUG
			CreateDXGIFactory2(flags, IID_PPV_ARGS(&m_DxGiFactory)) >> statusCode;
		}
		uint32_t Device::Validate()
		{
			for (size_t i = 0; i < m_DxGiFactory->EnumAdapters1(i, &m_DxgiAdapter1) != DXGI_ERROR_NOT_FOUND; ++i)
			{
				DXGI_ADAPTER_DESC1  dxgiAdapterDesc1;
				m_DxgiAdapter1->GetDesc1(&dxgiAdapterDesc1);

				// Check if adapter can create D3D12 Device
				if ((dxgiAdapterDesc1.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0 &&
					SUCCEEDED(D3D12CreateDevice(m_DxgiAdapter1.Get(), D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr) &&
						dxgiAdapterDesc1.DedicatedVideoMemory > m_MaxDedicatedVideoMemory))
				{
					m_MaxDedicatedVideoMemory = dxgiAdapterDesc1.DedicatedVideoMemory;
					m_DxgiAdapter1.As(&m_DxgiAdapter4);
				}
				else
				{
					return FPL_PIPELINE_DEVICE_ERROR;
				}
			}
			return FPL_SUCCESS;
		}
		D3D_FEATURE_LEVEL Device::CheckFeatureLevel()
		{
			D3D_FEATURE_LEVEL level = {};
			m_Device->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, &level, sizeof(level));
			return level;
		}
	}
}
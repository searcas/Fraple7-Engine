#pragma once
#include "Utilities/Common/Common.h"
#include "directx/d3dx12.h"
namespace Fraple7
{
	namespace Core
	{
		class Resource
		{
		public:
			explicit Resource(const std::wstring& name);
			explicit Resource(const D3D12_RESOURCE_DESC& resourceDesc,
				const D3D12_CLEAR_VALUE* clearValue,
				const std::wstring& name);
			explicit Resource(ComPtr<ID3D12Resource> resource, const std::wstring& name);

			Resource(const Resource& copy);
			Resource(Resource&& move);

			Resource& operator=(const Resource& other);
			Resource& operator=(Resource&& other) noexcept;

			virtual ~Resource();

			bool IsValid()const
			{
				return m_d3d12Resource != nullptr;
			}
			ComPtr<ID3D12Resource>GetD3D12Resource()const
			{
				return m_d3d12Resource;
			}
			D3D12_RESOURCE_DESC GetD3D12ResourceDesc()
			{
				D3D12_RESOURCE_DESC resDesc = {};
				if (m_d3d12Resource)
				{
					resDesc = m_d3d12Resource->GetDesc();
				}
				return resDesc;
			}
			virtual void SetD3D12Resource(ComPtr<ID3D12Resource> d3d12Resource,
				const D3D12_CLEAR_VALUE* clearValue = nullptr);
			virtual D3D12_CPU_DESCRIPTOR_HANDLE GetShaderResourceView(const D3D12_SHADER_RESOURCE_VIEW_DESC* srvDesc = nullptr) const = 0;
			virtual D3D12_CPU_DESCRIPTOR_HANDLE GetUnorderedAccessView(const D3D12_UNORDERED_ACCESS_VIEW_DESC* uavDesc = nullptr) const = 0;
			void SetName(const std::wstring& name);

			virtual void Reset();

			bool CheckFormatSupport(D3D12_FORMAT_SUPPORT1 formatSupport) const;
			bool CheckFormatSupport(D3D12_FORMAT_SUPPORT2 formatSupport) const;
		protected:
			ComPtr<ID3D12Resource> m_d3d12Resource;
			D3D12_FEATURE_DATA_FORMAT_SUPPORT m_FormatSupport;
			std::unique_ptr<D3D12_CLEAR_VALUE> m_d3d12ClearValue;
			std::wstring m_ResourceName;

		private:
			void CheckFeatureSupport();
		};
	}
}
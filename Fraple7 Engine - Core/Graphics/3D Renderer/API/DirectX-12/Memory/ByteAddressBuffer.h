#pragma once
#include "Buffer.h"
#include "DescriptorAllocation.h"
namespace Fraple7
{
	namespace Core
	{
		class ByteAddressBuffer : public Buffer
		{
		public:
			ByteAddressBuffer(const std::wstring& name = L"");
			ByteAddressBuffer(const D3D12_RESOURCE_DESC& resDesc,
				size_t numElements,
				size_t elementSize,
				const std::wstring& name = L"");
			~ByteAddressBuffer();

			virtual void CreateViews(size_t numElements, size_t elementSize) override;
			virtual D3D12_CPU_DESCRIPTOR_HANDLE GetShaderResourceView(const D3D12_SHADER_RESOURCE_VIEW_DESC* srvDesc = nullptr) const override;
			virtual D3D12_CPU_DESCRIPTOR_HANDLE GetUnorderedAccessView(const D3D12_UNORDERED_ACCESS_VIEW_DESC* uavDesc = nullptr) const override;
		private:
			size_t m_BufferSize;

			DescriptorAllocation m_SRV;
			DescriptorAllocation m_UAV;
		};
	}
}

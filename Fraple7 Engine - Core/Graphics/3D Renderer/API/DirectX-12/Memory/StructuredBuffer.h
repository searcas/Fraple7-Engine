#pragma once
#include "ByteAddressBuffer.h"

namespace Fraple7
{
	namespace Core
	{
		class StructuredBuffer : public Buffer
		{
		public:
			StructuredBuffer(const std::wstring& name = L"");
			StructuredBuffer(const D3D12_RESOURCE_DESC& resDesc,
				size_t numElements,
				size_t elementSize,
				const std::wstring& name = L"");
			~StructuredBuffer();
			virtual void CreateViews(size_t numeElements, size_t elementSize) override;
			virtual D3D12_CPU_DESCRIPTOR_HANDLE GetShaderResourceView(const D3D12_SHADER_RESOURCE_VIEW_DESC* srvDesc = nullptr) const override;
			virtual D3D12_CPU_DESCRIPTOR_HANDLE GetUnorderedAccessView(const D3D12_UNORDERED_ACCESS_VIEW_DESC* uavDesc = nullptr) const override;

			virtual size_t GetNumElements() const;
			virtual size_t GetElementSize() const;
			const ByteAddressBuffer& GetCounterBuffer() const;
		private:
			size_t m_NumElements;
			size_t m_ElementSize;

			DescriptorAllocation m_SRV;
			DescriptorAllocation m_UAV;

			ByteAddressBuffer m_CounterBuffer;
		};


	}
}

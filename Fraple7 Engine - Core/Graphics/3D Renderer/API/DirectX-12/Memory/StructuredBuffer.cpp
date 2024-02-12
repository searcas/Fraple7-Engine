#include "pch.h"
#include "StructuredBuffer.h"
#include "AllocationFactory.h"
#include "Graphics/3D Renderer/API/DirectX-12/Device.h"
namespace Fraple7
{
	namespace Core
	{
		StructuredBuffer::StructuredBuffer(const std::wstring& name)
			:	Buffer(name), 
				m_CounterBuffer(CD3DX12_RESOURCE_DESC::Buffer(4, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS), 1, 4, name + L"Counter"),
				m_NumElements(0),
				m_ElementSize(0)

		{
			m_SRV = AllocationFactory::GetInstance().AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			m_UAV = AllocationFactory::GetInstance().AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		}
		StructuredBuffer::StructuredBuffer(const D3D12_RESOURCE_DESC& resDesc, size_t numElements, size_t elementSize, const std::wstring& name)
			: Buffer(resDesc, numElements, elementSize, name),
			m_CounterBuffer(CD3DX12_RESOURCE_DESC::Buffer(4, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS), 1, 4, name + L"Counter"),
			m_NumElements(numElements),
			m_ElementSize(elementSize)
		{
			m_SRV = AllocationFactory::GetInstance().AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			m_UAV = AllocationFactory::GetInstance().AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		}
		StructuredBuffer::~StructuredBuffer()
		{
		}
		void StructuredBuffer::CreateViews(size_t numeElements, size_t elementSize)
		{
			m_NumElements = numeElements;
			m_ElementSize = elementSize;

			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = { };
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
			srvDesc.Format = DXGI_FORMAT_UNKNOWN;
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.Buffer.NumElements = static_cast<UINT>(m_NumElements);
			srvDesc.Buffer.StructureByteStride = static_cast<UINT>(m_ElementSize);
			srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

			Device::GetDevice()->CreateShaderResourceView(m_d3d12Resource.Get(), &srvDesc, m_SRV.GetDescriptorHandle());

			D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = { };

			uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
			uavDesc.Format = DXGI_FORMAT_UNKNOWN;
			uavDesc.Buffer.CounterOffsetInBytes = 0;
			uavDesc.Buffer.NumElements = static_cast<UINT>(m_NumElements);
			uavDesc.Buffer.StructureByteStride = static_cast<UINT>(m_ElementSize);
			uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

			Device::GetDevice()->CreateUnorderedAccessView(m_d3d12Resource.Get(), m_CounterBuffer.GetD3D12Resource().Get(), &uavDesc, m_UAV.GetDescriptorHandle());
		}
		D3D12_CPU_DESCRIPTOR_HANDLE StructuredBuffer::GetShaderResourceView(const D3D12_SHADER_RESOURCE_VIEW_DESC* srvDesc) const
		{
			return m_SRV.GetDescriptorHandle();
		}
		D3D12_CPU_DESCRIPTOR_HANDLE StructuredBuffer::GetUnorderedAccessView(const D3D12_UNORDERED_ACCESS_VIEW_DESC* uavDesc) const
		{
			return m_UAV.GetDescriptorHandle();
		}
		size_t StructuredBuffer::GetNumElements() const
		{
			return m_NumElements;
		}
		size_t StructuredBuffer::GetElementSize() const
		{
			return m_ElementSize;
		}
		const ByteAddressBuffer& StructuredBuffer::GetCounterBuffer() const
		{
			return m_CounterBuffer;
		}
	}
}
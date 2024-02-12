#include "pch.h"
#include "ByteAddressBuffer.h"
#include "AllocationFactory.h"
#include "Graphics/3D Renderer/API/DirectX-12/Device.h"
#include "Utilities/Utilities.hpp"
namespace Fraple7
{
	namespace Core
	{

		ByteAddressBuffer::ByteAddressBuffer(const std::wstring& name)
			:	Buffer(name), m_BufferSize(0)
		{
			m_SRV = AllocationFactory::GetInstance().AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			m_UAV = AllocationFactory::GetInstance().AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		}

		ByteAddressBuffer::ByteAddressBuffer(const D3D12_RESOURCE_DESC& resDesc, size_t numElements, size_t elementSize, const std::wstring& name)
			:	Buffer(resDesc, numElements, elementSize, name), m_BufferSize(0)
		{

		}

		ByteAddressBuffer::~ByteAddressBuffer()
		{
		}
		void ByteAddressBuffer::CreateViews(size_t numElements, size_t elementSize)
		{
			m_BufferSize = ::AlignUp(numElements * elementSize, 4);

			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = { };
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
			srvDesc.Format = DXGI_FORMAT_R32_TYPELESS;
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.Buffer.NumElements = (UINT)m_BufferSize / 4;
			srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;

			Device::GetDevice()->CreateShaderResourceView(m_d3d12Resource.Get(), &srvDesc, m_SRV.GetDescriptorHandle());

			D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = { };

			uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
			uavDesc.Format = DXGI_FORMAT_R32_TYPELESS;
			uavDesc.Buffer.NumElements = (UINT)m_BufferSize / 4;
			uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;

			Device::GetDevice()->CreateUnorderedAccessView(m_d3d12Resource.Get(), nullptr, &uavDesc, m_UAV.GetDescriptorHandle());
		}
		D3D12_CPU_DESCRIPTOR_HANDLE ByteAddressBuffer::GetShaderResourceView(const D3D12_SHADER_RESOURCE_VIEW_DESC* srvDesc) const
		{
			return m_SRV.GetDescriptorHandle();
		}
		D3D12_CPU_DESCRIPTOR_HANDLE ByteAddressBuffer::GetUnorderedAccessView(const D3D12_UNORDERED_ACCESS_VIEW_DESC* uavDesc) const
		{
			return m_UAV.GetDescriptorHandle();
		}
	}
}
#include "pch.h"
#include "DepthBuffer.h"
#include "Studio/Platform/Windows/Window.h"

namespace Fraple7
{
	namespace Core
	{
		DepthBuffer::DepthBuffer(const ComPtr<ID3D12Device2>& device, const Window& window) : m_Device(device), m_Window(window)
		{
			
		}
		void DepthBuffer::Create()
		{
			const CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_DEFAULT);
			const CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, m_Window.GetWidth(), m_Window.GetHeight(),
				1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

			const D3D12_CLEAR_VALUE clearValue = { .Format = DXGI_FORMAT_D32_FLOAT, .DepthStencil = { 1.0f, 0} };

			m_Device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE,
				&desc, D3D12_RESOURCE_STATE_DEPTH_WRITE,
				&clearValue, IID_PPV_ARGS(&m_DepthBuffer)) >> statusCode;
		}
		// dsv heap description
		void DepthBuffer::DescriptorHeap()
		{
			const D3D12_DESCRIPTOR_HEAP_DESC desc = { .Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV, .NumDescriptors = 1 };
			m_Device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_DsvDescriptorHeap)) >> statusCode;
		}
		void DepthBuffer::CreateDepthStencilView()
		{
			m_DsvHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE{ m_DsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart() };
			m_Device->CreateDepthStencilView(m_DepthBuffer.Get(), nullptr, m_DsvHandle);
		}

		DepthBuffer::~DepthBuffer()
		{

		}
		void DepthBuffer::Init()
		{
			Create();
			DescriptorHeap();
			CreateDepthStencilView();
		}
	}
}
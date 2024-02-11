#include "pch.h"
#include "DepthBuffer.h"
#include "Command.h"
#include "Studio/Platform/Windows/Window.h"
#include "Device.h"
namespace Fraple7
{
	namespace Core
	{
		DepthBuffer::DepthBuffer( const std::shared_ptr<Studio::Window>& window, std::shared_ptr<CommandMgr> commandMgr)
			:  m_Window(std::dynamic_pointer_cast<Studio::WinWindow>(window)), m_CommandMgr(commandMgr)
		{
				
		}

		void DepthBuffer::Create()
		{
			const CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_DEFAULT);
			const CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, m_Window->GetWidth(), m_Window->GetHeight(),
				1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

			const D3D12_CLEAR_VALUE clearValue = { .Format = DXGI_FORMAT_D32_FLOAT, .DepthStencil = { 1.0f, 0} };

			Device::GetDevice()->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE,
				&desc, D3D12_RESOURCE_STATE_DEPTH_WRITE,
				&clearValue, IID_PPV_ARGS(&m_DepthBuffer)) >> statusCode;
		}
		// dsv heap description
		void DepthBuffer::InitDescriptorHeap()
		{
			const D3D12_DESCRIPTOR_HEAP_DESC desc = { .Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV, .NumDescriptors = 1 };
			Device::GetDevice()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_DsvDescriptorHeap)) >> statusCode;
		}
		void DepthBuffer::CreateDepthStencilView()
		{
			m_DsvHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE{ m_DsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart() };
			D3D12_DEPTH_STENCIL_VIEW_DESC dsv = {};
			dsv.Format = DXGI_FORMAT_D32_FLOAT;
			dsv.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
			dsv.Texture2D.MipSlice = 0;
			dsv.Flags = D3D12_DSV_FLAG_NONE;

			Device::GetDevice()->CreateDepthStencilView(m_DepthBuffer.Get(), &dsv, m_DsvHandle);
		}

		DepthBuffer::~DepthBuffer()
		{

		}
		void DepthBuffer::Init()
		{
			Create();
			InitDescriptorHeap();
			CreateDepthStencilView();
		}
		void DepthBuffer::ResizeDepthBuffer()
		{
			// We need to make sure nothing being referenced to 
			// Depth buffer
			m_CommandMgr->UnloadAll();
				
			m_Window->SetWidth(std::max(1u, m_Window->GetWidth()));
			m_Window->SetHeight(std::max(1u, m_Window->GetHeight()));
			m_DepthBuffer = nullptr;
			Create();
			CreateDepthStencilView();
		}
	}
}
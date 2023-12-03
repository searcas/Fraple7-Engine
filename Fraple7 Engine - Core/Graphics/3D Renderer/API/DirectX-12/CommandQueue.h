#pragma once
#include <wrl.h>
#include <dxgi1_6.h>
#include "directx/d3d12.h"

namespace Fraple7
{
	namespace Core
	{
		using Microsoft::WRL::ComPtr;

		class CommQueue
		{
		public:
			CommQueue() = default;
			CommQueue(const D3D12_COMMAND_QUEUE_DESC& desc);
			void SetCommandQueueDescriptor(const D3D12_COMMAND_QUEUE_DESC& desc);
			uint32_t Create(ComPtr<ID3D12Device2>& device);
			const ComPtr<ID3D12CommandQueue>& GetCmdQueue() const { return m_CommandQueue; }
		private:
			ComPtr<ID3D12CommandQueue> m_CommandQueue;
			D3D12_COMMAND_QUEUE_DESC m_Desc;
		};

	}
}
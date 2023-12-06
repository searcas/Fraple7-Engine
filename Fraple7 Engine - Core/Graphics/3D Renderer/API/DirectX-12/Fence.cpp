#include "pch.h"
#include "Fence.h"



namespace Fraple7
{
	namespace Core
	{
		FenceDx::FenceDx()
		{

		}

		void FenceDx::Create(const ComPtr< ID3D12Device2>& device)
		{
			device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence)) >> statusCode;

		}
		void FenceDx::Signaling()
		{
			m_FenceEvent = CreateEventW(nullptr, FALSE, FALSE, FALSE);
			if (!m_FenceEvent)
			{
				GetLastError() >> statusCode;
				throw std::runtime_error{ "Failed to create fence event" };
			}
		}
	}
}


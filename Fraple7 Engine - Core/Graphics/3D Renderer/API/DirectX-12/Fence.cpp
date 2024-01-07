#include "pch.h"
#include "Fence.h"



namespace Fraple7
{
	namespace Core
	{
		FenceDx::FenceDx(const Commands::QueueDx& cq, uint32_t bufferCount) : m_CQueue(cq)
		{
			m_FenceValues.resize(bufferCount);
		}

		FenceDx::~FenceDx()
		{
			m_CQueue.GetCmdQueue()->Signal(m_Fence.Get(), ++m_FenceVal) >> statusCode;
			m_Fence->SetEventOnCompletion(m_FenceVal, m_FenceEvent) >> statusCode;

			if(WaitForSingleObject(m_FenceEvent, 2000) == WAIT_FAILED)
			{
				GetLastError() >> statusCode;
			}
		}

		void FenceDx::Create(const ComPtr< ID3D12Device2>& device)
		{
			device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence)) >> statusCode;
		}
		void FenceDx::CreateAnEvent()
		{
			m_FenceEvent = CreateEventW(nullptr, FALSE, FALSE, FALSE);
			if (!m_FenceEvent)
			{
				GetLastError() >> statusCode;
				throw std::runtime_error{ "Failed to create fence event" };
			}
		}
		void FenceDx::Wait(uint64_t time)
		{
			m_Fence->SetEventOnCompletion(m_FenceVal, m_FenceEvent) >> statusCode;
			if (::WaitForSingleObject(m_FenceEvent, time) == WAIT_FAILED)
			{
				GetLastError() >> statusCode;
			}
		}
		void FenceDx::Signal()
		{
			m_CQueue.GetCmdQueue()->Signal(m_Fence.Get(), ++m_FenceVal) >> statusCode;
		}
		void FenceDx::Complete()
		{
			Signal();
			Wait(INFINITE);
		}

		uint64_t FenceDx::CompleteMultiFrame()
		{
			Signal();
			Wait(INFINITE);
			return m_FenceVal;
		}
	}
}


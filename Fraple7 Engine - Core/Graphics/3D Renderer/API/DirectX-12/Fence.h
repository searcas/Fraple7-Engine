#pragma once
#include "directx/d3dx12.h"
#include "Utilities/Common/Common.h"
#include "Graphics/3D Renderer/API/DirectX-12/Command.h"
namespace Fraple7
{
	namespace Core
	{
		class FenceDx
		{
		public:
			FenceDx(const std::shared_ptr<Commands::QueueDx>&, uint32_t bufferCount);
			~FenceDx();
			void Create(const ComPtr< ID3D12Device2>& device);
			void CreateAnEvent();

			void Wait(uint64_t time);
			void Signal();
			void Complete();

			uint64_t CompleteMultiFrame();
		public:
			const ComPtr<ID3D12Fence>& GetFence()const { return m_Fence; }
			uint64_t& GetFenceVal() { return m_FenceVal; }
			const HANDLE& GetFenceEvent() const { return m_FenceEvent; }
			std::vector<uint64_t>& GetFenceValues() { return m_FenceValues; }
		private:
			ComPtr<ID3D12Fence> m_Fence;
			uint64_t m_FenceVal = 0;
			std::vector<uint64_t>m_FenceValues;
			HANDLE m_FenceEvent;
			const std::shared_ptr<Commands::QueueDx>& m_CQueue;
		};
	}
}
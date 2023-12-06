#pragma once
#include "directx/d3dx12.h"
#include "Utilities/Common/Common.h"

namespace Fraple7
{
	namespace Core
	{
		class FenceDx
		{
		public:
			FenceDx();
			~FenceDx() = default;
			void Create(const ComPtr< ID3D12Device2>& device);
			void Signaling();
		public:
			const ComPtr<ID3D12Fence>& GetFence()const { return m_Fence; }
			uint64_t& GetFenceVal() { return m_FenceVal; }
			const HANDLE& GetFenceEvent() const { return m_FenceEvent; }

		private:
			ComPtr<ID3D12Fence> m_Fence;
			uint64_t m_FenceVal = 0;
			HANDLE m_FenceEvent;
		};
	}
}
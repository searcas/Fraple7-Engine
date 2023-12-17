#pragma once
#include "Utilities/Common/Common.h"
#include "directx/d3d12.h"
namespace Fraple7
{
	namespace Core
	{
		class Inputlayout
		{
		public:
			Inputlayout();
			~Inputlayout() = default;
			uint32_t GetSize() { return m_size; };
			const D3D12_INPUT_ELEMENT_DESC* GetLayout() { return m_InputLayout; }
		private:
			const static inline uint32_t m_size = 2;
			D3D12_INPUT_ELEMENT_DESC m_InputLayout[m_size];
		};
	}
}

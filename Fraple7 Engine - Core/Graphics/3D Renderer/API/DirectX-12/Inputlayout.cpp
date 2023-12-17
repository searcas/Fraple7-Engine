#include "pch.h"
#include "Inputlayout.h"
#include "directx/d3d12.h"
namespace Fraple7
{
	namespace Core
	{
		Inputlayout::Inputlayout()
		{
			m_InputLayout[0] = { "Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 };
			m_InputLayout[1] = { "Color", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 };
		}
	}
}
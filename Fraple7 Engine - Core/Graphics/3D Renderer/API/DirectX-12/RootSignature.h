#pragma once
#include "Utilities/Common/Common.h"
#include "directx/d3dx12.h"
namespace Fraple7
{
	namespace Core
	{
		class RootSignature
		{
		public:
			RootSignature();
			~RootSignature();
			
			const ComPtr<ID3D12RootSignature>& GetSignature() { return m_RootSignature; }
			void Init();
			void Create(const ComPtr<ID3D12Device2>&);
		private:
			ComPtr<ID3D12RootSignature> m_RootSignature;
			CD3DX12_ROOT_SIGNATURE_DESC m_SigDesc;

		};

	}
}

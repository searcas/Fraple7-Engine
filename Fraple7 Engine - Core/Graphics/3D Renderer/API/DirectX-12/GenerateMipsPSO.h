#pragma once
#include "DirectXMath.h"
#include "Utilities/Common/Common.h"
#include "directx/d3dx12.h"
#include "Memory/DescriptorAllocation.h"

namespace Fraple7
{
	namespace Core
	{
		class RootSignature;
		struct alignas(16) GenerateMipsCB
		{
			uint32_t SrcMipLevel;
			uint32_t NumMipLevels;
			uint32_t SrcDimension;
			uint32_t IsSRGB;
			DirectX::XMFLOAT2 TexelSize;
		};
	
		class GenerateMipsPSO
		{
		public:
			enum GenerateMips
			{
				_GenerateMipsCB,
				_SrcMip,
				_OutMip,
				_NumRootparameters
			};
			GenerateMipsPSO(std::shared_ptr<RootSignature>rootSignature);
			~GenerateMipsPSO();

			const std::shared_ptr<RootSignature>& GetRootSignature()const;
			ComPtr<ID3D12PipelineState>GetPipelineState()const;
			D3D12_CPU_DESCRIPTOR_HANDLE GetDefaultUAV()const;
		private:
			std::shared_ptr<RootSignature> m_RootSignature;
			ComPtr<ID3D12PipelineState> m_PipelineState;
			DescriptorAllocation m_DefaultUAV;
		};
	}
}
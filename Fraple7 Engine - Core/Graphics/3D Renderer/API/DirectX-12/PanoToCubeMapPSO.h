#pragma once
#include "Utilities/Common/Common.h"
#include "directx/d3d12.h"
#include "Memory/DescriptorAllocation.h"

namespace Fraple7
{
	namespace Core
	{
		struct PanoToCubeMapCB
		{
			uint32_t CubeMapSize;
			uint32_t FirstMip;
			uint32_t NumMips;
		};

		class RootSignature;
		class PanoToCubeMapPSO
		{
		public:
			enum PanoToCubeMapRS
			{
				_PanoToCubeMapCB,
				_SrcTexture,
				_DstMips,
				_NumRootparameters,
			};
			PanoToCubeMapPSO();
			~PanoToCubeMapPSO();
			const std::shared_ptr<RootSignature>& GetRootSignature() const;
			ComPtr<ID3D12PipelineState> GetPipelineState()const;
			D3D12_CPU_DESCRIPTOR_HANDLE GetDefaultUAV()const;
		private:
			std::shared_ptr<RootSignature> m_RootSignature;
			ComPtr<ID3D12PipelineState> m_PipelineState;
			DescriptorAllocation m_DefaultUAV;
		};
	}
}
#pragma once
#include "Utilities/Common/Common.h"
#include "directx/d3dx12.h"
#include "RootSignature.h"
namespace Fraple7
{
	namespace Core
	{
		class PSO
		{
			struct PipelineStateStream
			{
				CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE rootSignature;
				CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT inputLayout;
				CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY primitiveTopology;
				CD3DX12_PIPELINE_STATE_STREAM_VS vertexShader;
				CD3DX12_PIPELINE_STATE_STREAM_PS pixelshader;
				//CD3DX12_PIPELINE_STATE_STREAM_CS ComputeShader;
				//CD3DX12_PIPELINE_STATE_STREAM_GS GeometryShader;
				CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT DSVFormats;
				CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;

			}pipelineStateStream;
		public:
			PSO();
			void Create(const ComPtr<ID3D12Device2>& GetDevice);
			~PSO() = default;
			const ComPtr<ID3D12PipelineState>& GetPLS() { return m_PipeLineState; }
			RootSignature& GetRootSig() { return m_RootSign; }
		private:
			RootSignature m_RootSign;
			ComPtr<ID3D12PipelineState> m_PipeLineState;
		};


	}
}
#include "pch.h"
#include "PipelineStateObject.h"
#include "Graphics/3D Renderer/API/DirectX-12/RootSignature.h"
#include "Graphics/3D Renderer/API/DirectX-12/PipelineStateObject.h"
#include "Graphics/3D Renderer/API/DirectX-12/Inputlayout.h"
#include "Graphics/3D Renderer/API/DirectX-12/ShadersLoader.h"

namespace Fraple7
{
	namespace Core
	{
		PSO::PSO()
		{
			
		}
		void PSO::Create(const ComPtr<ID3D12Device2>& device)
		{
			
			m_RootSign.Create(device);

			Inputlayout inputLayout;
			ShadersLoader shaders;
			shaders.Invoke(ShadersLoader::ShaderType::Vertex);
			shaders.Invoke(ShadersLoader::ShaderType::Pixel);

			pipelineStateStream.rootSignature = m_RootSign.GetSignature().Get();
			pipelineStateStream.inputLayout = { inputLayout.GetLayout(), (UINT)inputLayout.GetSize() };
			pipelineStateStream.primitiveTopology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			pipelineStateStream.vertexShader = CD3DX12_SHADER_BYTECODE(shaders.GetBlob(ShadersLoader::ShaderType::Vertex).Get());
			pipelineStateStream.pixelshader = CD3DX12_SHADER_BYTECODE(shaders.GetBlob(ShadersLoader::ShaderType::Pixel).Get());
			pipelineStateStream.DSVFormats = DXGI_FORMAT_D32_FLOAT;
			pipelineStateStream.RTVFormats = { .RTFormats { DXGI_FORMAT_R8G8B8A8_UNORM}, .NumRenderTargets = 1 };
			const D3D12_PIPELINE_STATE_STREAM_DESC  pipelineStateStreamDesc = { sizeof(PipelineStateStream), &pipelineStateStream };
			 
			device->CreatePipelineState(&pipelineStateStreamDesc, IID_PPV_ARGS(&m_PipeLineState)) >> statusCode;
		}
	}
}
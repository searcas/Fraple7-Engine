#include "pch.h"
#include "GenerateMipsPSO.h"
#include "RootSignature.h"
#include "Graphics/3D Renderer/API/DirectX-12/ShadersLoader.h"
#include "Memory/AllocationFactory.h"
#include "Device.h"
namespace Fraple7
{
	namespace Core
	{

		GenerateMipsPSO::GenerateMipsPSO(std::shared_ptr<RootSignature>rootSignature)
			:	m_RootSignature(rootSignature)
		{
			D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
			featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

			if (FAILED(Device::GetDevice()->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
			{
				featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
			}
			CD3DX12_DESCRIPTOR_RANGE1 srcMip(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE);
			CD3DX12_DESCRIPTOR_RANGE1 outMip(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 4, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE);

			CD3DX12_ROOT_PARAMETER1 rootParameters[GenerateMips::_NumRootparameters];
			rootParameters[GenerateMips::_GenerateMipsCB].InitAsConstants(sizeof(GenerateMipsCB) / 4, 0);
			rootParameters[GenerateMips::_SrcMip].InitAsDescriptorTable(1, &srcMip);
			rootParameters[GenerateMips::_OutMip].InitAsDescriptorTable(1, &outMip);

			CD3DX12_STATIC_SAMPLER_DESC linearClampSampler(0,
				D3D12_FILTER_MIN_MAG_MIP_LINEAR,
				D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
				D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
				D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
			CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc(GenerateMips::_NumRootparameters,
				rootParameters, 1, &linearClampSampler);
			m_RootSignature->SetRootSignatureDesc(rootSignatureDesc.Desc_1_1, featureData.HighestVersion);

			struct PipelineStateStream
			{
				CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE pRootSignature;
				CD3DX12_PIPELINE_STATE_STREAM_CS CS;
			}pipelineStateStream;
			ShadersLoader shaders;
			shaders.Invoke(ShadersLoader::ShaderType::Compute);
			pipelineStateStream.pRootSignature = m_RootSignature->GetRootSignature().Get();
			pipelineStateStream.CS = { CD3DX12_SHADER_BYTECODE(shaders.GetBlob(ShadersLoader::ShaderType::Vertex).Get()) };
			D3D12_PIPELINE_STATE_STREAM_DESC pipelineStateStreamDesc = {
				sizeof(PipelineStateStream), &pipelineStateStream };

			Device::GetDevice()->CreatePipelineState(&pipelineStateStreamDesc, IID_PPV_ARGS(&m_PipelineState)) >> statusCode;

			m_DefaultUAV = AllocationFactory::GetInstance().AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 4);

			for (size_t i = 0; i < 4; i++)
			{
				D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
				uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
				uavDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
				uavDesc.Texture2D.MipSlice = i;
				uavDesc.Texture2D.PlaneSlice = 0;

				Device::GetDevice()->CreateUnorderedAccessView(nullptr, nullptr, &uavDesc, m_DefaultUAV.GetDescriptorHandle(i));
			}
		}
		GenerateMipsPSO::~GenerateMipsPSO()
		{
		}
		const std::shared_ptr<RootSignature>& GenerateMipsPSO::GetRootSignature() const
		{
			return m_RootSignature;
		}

		ComPtr<ID3D12PipelineState> GenerateMipsPSO::GetPipelineState() const
		{
			return m_PipelineState;
		}

		D3D12_CPU_DESCRIPTOR_HANDLE GenerateMipsPSO::GetDefaultUAV() const
		{
			return m_DefaultUAV.GetDescriptorHandle();
		}

	}
}
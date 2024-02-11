#include "pch.h"
#include "PanoToCubeMapPSO.h"
#include "RootSignature.h"
#include "ShadersLoader.h"
#include "Memory/AllocationFactory.h"
#include "Device.h"
namespace Fraple7
{
	namespace Core
	{
		PanoToCubeMapPSO::PanoToCubeMapPSO()
		{
			D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
			featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
			if (FAILED(Device::GetDevice()->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
			{
				featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
			}

			CD3DX12_DESCRIPTOR_RANGE1 srcMip(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE);
			CD3DX12_DESCRIPTOR_RANGE1 outMip(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 5, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE);

			CD3DX12_ROOT_PARAMETER1 rootParameters[PanoToCubeMapRS::_NumRootparameters] = {};
			rootParameters[PanoToCubeMapRS::_PanoToCubeMapCB].InitAsConstants(sizeof(PanoToCubeMapCB) / 4, 0);
			rootParameters[PanoToCubeMapRS::_SrcTexture].InitAsDescriptorTable(1, &srcMip);
			rootParameters[PanoToCubeMapRS::_DstMips].InitAsDescriptorTable(1, &outMip);

			CD3DX12_STATIC_SAMPLER_DESC linearRepeatSampler(0, 
										D3D12_FILTER_MIN_MAG_MIP_LINEAR,
										D3D12_TEXTURE_ADDRESS_MODE_WRAP,
										D3D12_TEXTURE_ADDRESS_MODE_WRAP,
										D3D12_TEXTURE_ADDRESS_MODE_WRAP);

			CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc(PanoToCubeMapRS::_NumRootparameters,
				rootParameters, 1, &linearRepeatSampler);

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
			D3D12_PIPELINE_STATE_STREAM_DESC pipelineStateStreamDesc = { sizeof(PipelineStateStream), &pipelineStateStream };

			Device::GetDevice()->CreatePipelineState(&pipelineStateStreamDesc, IID_PPV_ARGS(&m_PipelineState)) >> statusCode;

			m_DefaultUAV = AllocationFactory::GetInstance().AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 5);
			UINT descriptorHandleIncrementSize = Device::GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

			for (UINT i = 0; i < 5; i++)
			{
				D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
				uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
				uavDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
				uavDesc.Texture2DArray.ArraySize = 6; // CubeMap
				uavDesc.Texture2DArray.FirstArraySlice = 0;
				uavDesc.Texture2DArray.MipSlice = i;
				uavDesc.Texture2DArray.PlaneSlice = 0;

				Device::GetDevice()->CreateUnorderedAccessView(nullptr, nullptr, &uavDesc, m_DefaultUAV.GetDescriptorHandle(i));
			}
		}
		PanoToCubeMapPSO::~PanoToCubeMapPSO()
		{
		}
		const std::shared_ptr<RootSignature>& PanoToCubeMapPSO::GetRootSignature() const
		{
			return m_RootSignature;
		}
		ComPtr<ID3D12PipelineState> PanoToCubeMapPSO::GetPipelineState() const
		{
			return m_PipelineState;
		}
		D3D12_CPU_DESCRIPTOR_HANDLE PanoToCubeMapPSO::GetDefaultUAV() const
		{
			return m_DefaultUAV.GetDescriptorHandle();
		}
	}
}
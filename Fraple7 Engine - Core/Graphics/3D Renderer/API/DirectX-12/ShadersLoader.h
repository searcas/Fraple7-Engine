#pragma once
#include <vector>
#include "Utilities/Common/Common.h"
#include "directx/d3dx12.h"
namespace Fraple7
{
	namespace Core
	{
		class ShadersLoader
		{
		public:
			enum ShaderType : uint16_t
			{
				Vertex = 0,
				Pixel,
				Compute,
				Geometry,
			};
		public:
			ShadersLoader();
			void Invoke(ShaderType type);
			~ShadersLoader() = default;
			const ComPtr<ID3DBlob>& GetBlob(ShaderType type) { return m_Shaders[type];}
			void PixelShader();
			void VertexShader();
			void GeometryShader();
			void ComputeShader();
		private:
			ComPtr<ID3DBlob> m_Shaders[4];
		};
	}
}
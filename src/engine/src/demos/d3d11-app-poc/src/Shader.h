
#ifndef __XE_GRAPHICS_D3D11_TESTAPP_SHADER_HPP__
#define __XE_GRAPHICS_D3D11_TESTAPP_SHADER_HPP__

#include <d3d11.h>
#include <xe/math/Matrix.h>

namespace TestApp {
    using namespace xe;

    class Shader {
    public:
        Shader();

        ~Shader();

        bool Initialize(ID3D11Device *device, HWND hWnd);

        void Shutdown();

        bool Render(ID3D11DeviceContext *context, int indexCount, const Matrix4 &world, const Matrix4 &view, const Matrix4 &projection);

    private:
        bool InitializeShader(ID3D11Device *device, HWND hWnd, LPCWSTR vertexShaderSrc, LPCWSTR pixelShaderSrc);

        void ShutdownShader();

        void DisplayShaderErrorMessage(ID3D10Blob *blob, HWND hWnd, LPCWSTR msg);

        bool SetShaderParameters(ID3D11DeviceContext *context, const Matrix4 &world, const Matrix4 &view, const Matrix4 &projection);

        void RenderShader(ID3D11DeviceContext *context, int indexCount);

    private:
        struct MatrixBufferType {
            Matrix4 world;
            Matrix4 view;
            Matrix4 projection;
        };

        ID3D11VertexShader *vertexShader = nullptr;
        ID3D11PixelShader *pixelShader = nullptr;
        ID3D11InputLayout *layout = nullptr;
        ID3D11Buffer *matrixBuffer = nullptr;
    };
} // namespace TestApp

#endif

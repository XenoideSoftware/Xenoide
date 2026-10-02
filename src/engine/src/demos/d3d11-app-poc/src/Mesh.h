
#ifndef __XE_GRAPHICS_D3D11_TESTAPP_MESH_HPP__
#define __XE_GRAPHICS_D3D11_TESTAPP_MESH_HPP__

#include <d3d11.h>

namespace TestApp {
    class Mesh {
    public:
        Mesh();

        ~Mesh();

        bool Initialize(ID3D11Device *device);

        void Shutdown();

        void Render(ID3D11DeviceContext *context);

        int GetIndexCount() const;

    private:
        bool InitializeBuffers(ID3D11Device *device);

        void ShutdownBuffers();

        void RenderBuffers(ID3D11DeviceContext *context);

    private:
        ID3D11Buffer *vertexBuffer = nullptr;
        ID3D11Buffer *indexBuffer = nullptr;

        int vertexCount = 0;
        int indexCount = 0;
    };
} // namespace TestApp

#endif

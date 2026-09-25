
#include "Mesh.h"
#include <xe/math/Vector.h>

namespace TestApp {
    using namespace xe;

    struct VertexType {
        Vector3 position;
        Vector4 color;
    };

    Mesh::Mesh() {
    }

    Mesh::~Mesh() {
        this->Shutdown();
    }

    bool Mesh::Initialize(ID3D11Device *device) {
        return this->InitializeBuffers(device);
    }

    void Mesh::Shutdown() {
        this->ShutdownBuffers();
    }

    void Mesh::Render(ID3D11DeviceContext *context) {
        this->RenderBuffers(context);
    }

    int Mesh::GetIndexCount() const {
        return indexCount;
    }

    bool Mesh::InitializeBuffers(ID3D11Device *device) {
        VertexType vertices[] = {{{-1.0f, -1.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}}, {{0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}}, {{1.0f, -1.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}}};

        int indices[] = {0, 1, 2};

        // setup vertex data
        D3D11_BUFFER_DESC vertexBufferDesc = {};
        vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
        vertexBufferDesc.ByteWidth = sizeof(VertexType) * 3;
        vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

        D3D11_SUBRESOURCE_DATA vertexData = {};
        vertexData.pSysMem = vertices;
        vertexData.SysMemPitch = 0;
        vertexData.SysMemSlicePitch = 0;

        if (FAILED(device->CreateBuffer(&vertexBufferDesc, &vertexData, &vertexBuffer))) {
            return false;
        }

        // setup index data
        D3D11_BUFFER_DESC indexBufferDesc = {};
        indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
        indexBufferDesc.ByteWidth = sizeof(int) * 3;
        indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

        D3D11_SUBRESOURCE_DATA indexData = {};
        indexData.pSysMem = indices;
        indexData.SysMemPitch = 0;
        indexData.SysMemSlicePitch = 0;

        if (FAILED(device->CreateBuffer(&indexBufferDesc, &indexData, &indexBuffer))) {
            return false;
        }

        return true;
    }

    void Mesh::ShutdownBuffers() {
        if (indexBuffer) {
            indexBuffer->Release();
            indexBuffer = nullptr;
        }

        if (vertexBuffer) {
            vertexBuffer->Release();
            vertexBuffer = nullptr;
        }
    }

    void Mesh::RenderBuffers(ID3D11DeviceContext *context) {
        const unsigned int stride = sizeof(VertexType);
        const unsigned int offset = 0;

        context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
        context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    }
} // namespace TestApp


#include "glcore3-context.h"

namespace xe {

    inline const void *mipLevelDataAt(const TextureDescriptor &desc, size_t mip, size_t face, size_t faceCount) {
        if (desc.mipLevels == nullptr) {
            return nullptr;
        }
        return desc.mipLevels[mip * faceCount + face].data;
    }

    inline int halveDimension(int size) {
        return size > 1 ? size / 2 : 1;
    }

    static constexpr std::array<gl::TextureTarget, 6> cubeMapSides{
        gl::TextureTarget::eTextureCubeMapPositiveX,
        gl::TextureTarget::eTextureCubeMapNegativeX,
        gl::TextureTarget::eTextureCubeMapPositiveY,
        gl::TextureTarget::eTextureCubeMapNegativeY,
        gl::TextureTarget::eTextureCubeMapPositiveZ,
        gl::TextureTarget::eTextureCubeMapNegativeZ,
    };

    gl::TextureTarget toTextureTargetGL(const TextureType type) {
        switch (type) {
        case TextureType::Tex1D:
            return gl::TextureTarget::eTexture1d;
        case TextureType::Tex2D:
            return gl::TextureTarget::eTexture2d;
        case TextureType::Tex3D:
            return gl::TextureTarget::eTexture3d;
        case TextureType::TexCubeMap:
            return gl::TextureTarget::eTextureCubeMap;
        case TextureType::Tex2DArray:
            return gl::TextureTarget::eTexture2dArray;
        }
        assert(false && "toTextureTargetGL: Invalid TextureType");
        return gl::TextureTarget::eTexture2d;
    }

    gl::InternalFormat toInternalFormatGL(const PixelFormat pixelFormat) {
        switch (pixelFormat) {
        case PixelFormat::R8G8B8:
            return gl::InternalFormat::eRgb;
        case PixelFormat::R8G8B8A8:
            return gl::InternalFormat::eRgba;
        default:
            break;
        }
        assert(false && "toInternalFormatGL: Invalid PixelFormat");
        return gl::InternalFormat::eRgba;
    }

    gl::PixelFormat toPixelFormatGL(const PixelFormat pixelFormat) {
        switch (pixelFormat) {
        case PixelFormat::R8G8B8:
            return gl::PixelFormat::eRgb;
        case PixelFormat::R8G8B8A8:
            return gl::PixelFormat::eRgba;
        default:
            break;
        }
        assert(false && "toPixelFormatGL: Invalid PixelFormat");
        return gl::PixelFormat::eRgba;
    }

    gl::PixelType toPixelTypeGL(const PixelDataType dataType) {
        const TypeEncoding te = static_cast<TypeEncoding>(dataType);
        const TypeKind kind = getTypeKind(te);
        const TypeSize size = getElementSize(te);
        if (kind == TypeKind::Float) {
            return size == TypeSize::Byte2 ? gl::PixelType::eHalfFloat : gl::PixelType::eFloat;
        }
        if (kind == TypeKind::UInt) {
            if (size == TypeSize::Byte1)
                return gl::PixelType::eUnsignedByte;
            if (size == TypeSize::Byte2)
                return gl::PixelType::eUnsignedShort;
            return gl::PixelType::eUnsignedInt;
        }
        // Int
        if (size == TypeSize::Byte1)
            return gl::PixelType::eByte;
        if (size == TypeSize::Byte2)
            return gl::PixelType::eShort;
        return gl::PixelType::eInt;
    }

    tl::expected<TextureHandle, BackendError> createTextureGL(RenderDeviceBackendContext *ctx, const TextureDescriptor &desc) {
        auto &textures = glctx(ctx)->textures;
        if (!poolHasCapacity(textures)) {
            return makeBackendError(BackendErrorCode::HandlePoolExhausted, "texture pool exhausted (16-bit index field)");
        }

        size_t const faceCount = (desc.type == TextureType::TexCubeMap) ? 6u : 1u;
        size_t const mipCount = desc.mipLevelCount == 0 ? 1u : (desc.mipLevelCount / faceCount);

        if (mipCount > 1 && desc.generateMipmaps) {
            return makeBackendError(BackendErrorCode::InvalidDescriptor, "TextureDescriptor: generateMipmaps is mutually exclusive with supplying >1 mip level");
        }
        if (desc.mipLevels != nullptr && desc.mipLevelCount != mipCount * faceCount) {
            return makeBackendError(BackendErrorCode::InvalidDescriptor, "TextureDescriptor: mipLevelCount must equal mipCount * faceCount");
        }

        auto texture = glaze::makeUnique<gl::Texture>();

        gl::TextureTarget const target = toTextureTargetGL(desc.type);
        gl::InternalFormat const internalFormat = toInternalFormatGL(desc.format);
        gl::PixelFormat const pixelFormat = toPixelFormatGL(desc.sourceFormat);
        gl::PixelType const pixelType = toPixelTypeGL(desc.sourceDataType);

        gl::bindTexture(target, texture);

        int w = desc.size.x;
        int h = desc.size.y;
        int d = desc.size.z;

        for (size_t mip = 0; mip < mipCount; ++mip) {
            GLint const level = static_cast<GLint>(mip);

            switch (desc.type) {
            case TextureType::Tex1D:
                gl::texImage1D(target, level, internalFormat, w, 0, pixelFormat, pixelType, mipLevelDataAt(desc, mip, 0, faceCount));
                break;

            case TextureType::Tex2D:
                gl::texImage2D(target, level, internalFormat, w, h, 0, pixelFormat, pixelType, mipLevelDataAt(desc, mip, 0, faceCount));
                break;

            case TextureType::Tex3D:
            case TextureType::Tex2DArray:
                gl::texImage3D(target, level, internalFormat, w, h, d, 0, pixelFormat, pixelType, mipLevelDataAt(desc, mip, 0, faceCount));
                break;

            case TextureType::TexCubeMap:
                for (size_t face = 0; face < 6u; ++face) {
                    gl::texImage2D(cubeMapSides[face], level, internalFormat, w, h, 0, pixelFormat, pixelType, mipLevelDataAt(desc, mip, face, faceCount));
                }
                break;

            default:
                return makeBackendError(BackendErrorCode::InternalError, "createTextureGL: unknown TextureType");
            }

            w = halveDimension(w);
            h = halveDimension(h);
            d = halveDimension(d);
        }

        if (desc.generateMipmaps) {
            gl::generateMipmap(target);
        } else {
            gl::texParameteri(target, gl::TextureParameterName::eTextureBaseLevel, 0);
            gl::texParameteri(target, gl::TextureParameterName::eTextureMaxLevel, static_cast<GLint>(mipCount - 1));
        }

        // TODO: Use a Sampler Resource instead for later
        bool const hasMips = desc.generateMipmaps || mipCount > 1;

        GLint const param = hasMips ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR;
        gl::texParameteri(target, gl::TextureParameterName::eTextureMinFilter, param);
        gl::texParameteri(target, gl::TextureParameterName::eTextureMagFilter, GL_LINEAR);
        gl::texParameteri(target, gl::TextureParameterName::eTextureWrapS, GL_CLAMP_TO_EDGE);

        if (h > 0) {
            gl::texParameteri(target, gl::TextureParameterName::eTextureWrapT, GL_CLAMP_TO_EDGE);
        }

        if (d > 0) {
            gl::texParameteri(target, gl::TextureParameterName::eTextureWrapR, GL_CLAMP_TO_EDGE);
        }

        gl::bindTexture(target, {});

        auto const [index, gen] = acquireSlot(textures, TextureGL{std::move(texture)});
        return TextureHandle::make(gen, index, desc.type);
    }

    void destroyTextureGL(RenderDeviceBackendContext *ctx, TextureHandle handle) {
        auto &textures = glctx(ctx)->textures;
        uint32_t const index = handle.index();
        assert(index < textures.size() && "destroyTextureGL: handle index out of range");
        auto &slot = textures[index];
        assert(slot.obj && "destroyTextureGL: slot already free (double destroy)");
        assert(slot.gen == handle.gen() && "destroyTextureGL: stale handle (generation mismatch)");
        slot.obj.reset();
    }

    void updateTextureGL(RenderDeviceBackendContext *ctx, TextureHandle handle, const TextureUpdateDescriptor &desc) {
        auto &textures = glctx(ctx)->textures;
        uint32_t const index = handle.index();
        assert(index < textures.size() && "updateTextureGL: handle index out of range");
        auto &slot = textures[index];
        assert(slot.obj && "updateTextureGL: use of freed handle");
        assert(slot.gen == handle.gen() && "updateTextureGL: stale handle (generation mismatch)");

        TextureType const type = handle.subType();
        gl::TextureTarget const target = toTextureTargetGL(type);
        gl::PixelFormat const pixelFormat = toPixelFormatGL(desc.sourceFormat);
        gl::PixelType const pixelType = toPixelTypeGL(desc.sourceDataType);

        GLint const level = desc.mipLevel;
        GLint const x = desc.offset.x;
        GLint const y = desc.offset.y;
        GLint const z = desc.offset.z;
        GLsizei const w = desc.size.x;
        GLsizei const h = desc.size.y;
        GLsizei const d = desc.size.z;

        gl::bindTexture(target, slot.obj->texture);

        switch (type) {
        case TextureType::Tex1D:
            gl::texSubImage1D(target, level, x, w, pixelFormat, pixelType, desc.sourceData);
            break;

        case TextureType::Tex2D:
            gl::texSubImage2D(target, level, x, y, w, h, pixelFormat, pixelType, desc.sourceData);
            break;

        case TextureType::Tex3D:
        case TextureType::Tex2DArray:
            gl::texSubImage3D(target, level, x, y, z, w, h, d, pixelFormat, pixelType, desc.sourceData);
            break;

        case TextureType::TexCubeMap:
            assert(desc.faceIndex >= 0 && desc.faceIndex < 6 && "TextureUpdateDescriptor: faceIndex out of range");
            gl::texSubImage2D(cubeMapSides[desc.faceIndex], level, x, y, w, h, pixelFormat, pixelType, desc.sourceData);
            break;

        default:
            assert(false && "TextureType is unknown");
        }

        gl::bindTexture(target, {});
    }

    void readTextureGL(RenderDeviceBackendContext *ctx, TextureHandle handle, const TextureReadDescriptor &desc) {
        assert(desc.data != nullptr && "TextureReadDescriptor: data must not be null");
        assert(desc.offset.x == 0 && desc.offset.y == 0 && desc.offset.z == 0 && "TextureReadDescriptor: GL 3.3 backend requires offset == {0,0,0}");

        auto &textures = glctx(ctx)->textures;
        uint32_t const index = handle.index();
        assert(index < textures.size() && "readTextureGL: handle index out of range");
        auto &slot = textures[index];
        assert(slot.obj && "readTextureGL: use of freed handle");
        assert(slot.gen == handle.gen() && "readTextureGL: stale handle (generation mismatch)");

        TextureType const type = handle.subType();
        gl::TextureTarget const bindTarget = toTextureTargetGL(type);
        gl::PixelFormat const pixelFormat = toPixelFormatGL(desc.destFormat);
        gl::PixelType const pixelType = toPixelTypeGL(desc.destDataType);
        gl::TextureTarget const readTarget = (type == TextureType::TexCubeMap) ? cubeMapSides[desc.faceIndex] : bindTarget;

        if (type == TextureType::TexCubeMap) {
            assert(desc.faceIndex >= 0 && desc.faceIndex < 6 && "TextureReadDescriptor: faceIndex out of range");
        }

        gl::bindTexture(bindTarget, slot.obj->texture);

#ifndef NDEBUG
        GLint actualWidth = 0;
        GLint actualHeight = 0;
        gl::getTexLevelParameteriv(readTarget, desc.mipLevel, gl::GetTextureParameter::eTextureWidth, &actualWidth);
        gl::getTexLevelParameteriv(readTarget, desc.mipLevel, gl::GetTextureParameter::eTextureHeight, &actualHeight);
        assert(actualWidth == desc.size.x && "TextureReadDescriptor: GL 3.3 backend requires size.x to match the mip's full width");
        assert((type == TextureType::Tex1D || actualHeight == desc.size.y) && "TextureReadDescriptor: GL 3.3 backend requires size.y to match the mip's full height");
#endif

        gl::getTexImage(readTarget, desc.mipLevel, pixelFormat, pixelType, desc.data);
    }

} // namespace xe

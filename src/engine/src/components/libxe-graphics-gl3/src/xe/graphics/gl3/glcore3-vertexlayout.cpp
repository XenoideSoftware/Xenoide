
#include "glcore3-context.h"

namespace xe {

    gl::AttributeType toAttributeTypeGL(const VertexAttribFormat attributeType) {
        const TypeEncoding te = static_cast<TypeEncoding>(attributeType);
        const uint8_t cols = getTypeCols(te);
        if (getTypeKind(te) == TypeKind::Float) {
            constexpr gl::AttributeType floatTypes[] = {gl::AttributeType::eFloat, gl::AttributeType::eFloatVec2, gl::AttributeType::eFloatVec3, gl::AttributeType::eFloatVec4};
            return floatTypes[cols - 1];
        }
        constexpr gl::AttributeType intTypes[] = {gl::AttributeType::eInt, gl::AttributeType::eIntVec2, gl::AttributeType::eIntVec3, gl::AttributeType::eIntVec4};
        return intTypes[cols - 1];
    }

    gl::VertexAttribPointerType toVertexAttribPointerTypeGL(const VertexAttribFormat attributeType) {
        const TypeKind kind = getTypeKind(static_cast<TypeEncoding>(attributeType));
        return kind == TypeKind::Float ? gl::VertexAttribPointerType::eFloat : gl::VertexAttribPointerType::eInt;
    }

    gl::DrawElementsType toDrawElementsTypeGL(const GeometryIndexType indexType) {
        const TypeSize size = getElementSize(static_cast<TypeEncoding>(indexType));
        return size == TypeSize::Byte2 ? gl::DrawElementsType::eUnsignedShort : gl::DrawElementsType::eUnsignedInt;
    }

    tl::expected<VertexLayoutHandle, BackendError> createVertexLayoutGL(RenderDeviceBackendContext *ctx, const VertexLayoutDescriptor &desc) {
        auto &layouts = glctx(ctx)->layouts;

        if (!poolHasCapacity(layouts)) {
            return makeBackendError(BackendErrorCode::HandlePoolExhausted, "vertex layout pool exhausted (16-bit index field)");
        }

        VertexLayoutGL layout;
        layout.attributes.reserve(desc.attribs.size());
        layout.indexDataType = toDrawElementsTypeGL(desc.indexType);

        for (const VertexAttrib &attr : desc.attribs) {
            layout.attributes.emplace_back(
                gl::AttribLocation{attr.location},
                static_cast<int>(getTypeCols(static_cast<TypeEncoding>(attr.format))),
                toVertexAttribPointerTypeGL(attr.format),
                attr.normalized ? GL_TRUE : GL_FALSE
            );
        }

        auto const [index, gen] = acquireSlot(layouts, std::move(layout));
        return VertexLayoutHandle::make(gen, index);
    }

    void destroyVertexLayoutGL(RenderDeviceBackendContext *ctx, VertexLayoutHandle handle) {
        auto &layouts = glctx(ctx)->layouts;
        uint32_t const index = handle.index();
        assert(index < layouts.size() && "destroyVertexLayoutGL: handle index out of range");
        auto &slot = layouts[index];
        assert(slot.obj && "destroyVertexLayoutGL: slot already free (double destroy)");
        assert(slot.gen == handle.gen() && "destroyVertexLayoutGL: stale handle (generation mismatch)");
        slot.obj.reset();
    }

} // namespace xe


#pragma once

#include <cstdint>
#include <cstddef>
#include <array>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <tl/expected.hpp>

#include <xe/DataType.h>
#include <xe/graphics/GraphicsDevice.h>
#include <xe/graphics/BufferDescriptor.h>
#include <xe/graphics/Uniform.h>

namespace xe {
    /**
     * @brief Failure codes produced by the render-backend factory functions.
     *
     * Used as the discriminant of BackendError. The backend layer is exception-free, so every
     * creation entry point returns tl::expected<Handle, BackendError> and callers must check
     * the result before use. Codes are broad categories; the specific detail (shader InfoLog,
     * descriptor field name, pool identity, ...) lives on BackendError::message.
     */
    enum class BackendErrorCode {
        //! Sentinel used only as the default value of a freshly default-constructed BackendError.
        Ok = 0,
        //! Caller-supplied descriptor violates its contract (unsupported enum, impossible field combo, ...).
        InvalidDescriptor,
        //! GLSL source failed to compile; message carries the GL InfoLog verbatim.
        ShaderCompileFailed,
        //! Linked program failed to link; message carries the GL InfoLog verbatim.
        ShaderLinkFailed,
        //! Backend allocation of a texture resource failed (driver-side).
        TextureAllocationFailed,
        //! Backend allocation of a buffer resource failed (driver-side).
        BufferAllocationFailed,
        //! The 16-bit index field is saturated and no slot is free for reuse.
        HandlePoolExhausted,
        //! Host-side allocation returned null (std::nothrow path).
        AllocationFailed,
        //! Unreachable / unknown backend state. Assert-worthy in debug builds.
        InternalError,
    };

    /**
     * @brief Structured error payload returned by backend factories via tl::expected.
     *
     * `code` identifies the failure category; `message` carries the human-readable detail
     * (shader InfoLog, failed descriptor field, GL driver string, ...). Callers should display
     * `message` as-is and should not attempt to parse it.
     */
    struct BackendError {
        //! Failure category. Default Ok is never returned - a live BackendError has code != Ok.
        BackendErrorCode code = BackendErrorCode::Ok;

        //! Optional human-readable detail; may be empty.
        std::string message;
    };

    /**
     * @brief Sugar for building a tl::unexpected<BackendError> without ceremony at the call site.
     * @param code failure category
     * @param message optional detail; moved into the resulting BackendError
     */
    inline tl::unexpected<BackendError> makeBackendError(BackendErrorCode code, std::string message = {}) {
        return tl::unexpected<BackendError>{BackendError{code, std::move(message)}};
    }

    /**
     * @brief Compile-time tag discriminating handle kinds.
     * Encoded in the C++ type system via HandleT's Tag template parameter; no longer stored in the
     * handle bit payload (the former type:4 field is now reserved).
     */
    enum HandleType : uint32_t { HandleBuffer = 0x01, HandleShader, HandleTexture, HandleVertexLayout, HandlePipeline, HandleGeometry };

    namespace detail {
        //! Sentinel subtype enum used by handles whose resource class has no meaningful subtype.
        enum class NoSubtype : uint32_t { None = 0 };

        /**
         * @brief Zero-cost strongly-typed handle to a backend resource.
         *
         * Instances of HandleT with different Tag (HandleType) or SubEnum template parameters are
         * distinct C++ types, so mixing a buffer handle with a texture handle is a compile error.
         * The raw 32-bit payload is laid out as:
         *
         *   bit: 31       28 27        20 19      16 15                    0
         *       +-----------+-------------+----------+------------------------+
         *       | subType:4 |    gen:8    | reserved |       index:16         |
         *       +-----------+-------------+----------+------------------------+
         *
         * @tparam Tag compile-time HandleType tag (identifies resource class)
         * @tparam SubEnum resource-specific subtype enum (e.g. TextureType for textures). Defaults to
         *         NoSubtype when the resource class has no subtype dimension.
         */
        template <HandleType Tag, class SubEnum = NoSubtype> struct HandleT {
            static constexpr uint32_t IndexMask = 0xFFFFu;
            static constexpr uint32_t ReservedMask = 0xFu;
            static constexpr uint32_t GenMask = 0xFFu;
            static constexpr uint32_t SubTypeMask = 0xFu;

            static constexpr uint32_t IndexShift = 0;
            static constexpr uint32_t ReservedShift = 16;
            static constexpr uint32_t GenShift = 20;
            static constexpr uint32_t SubTypeShift = 28;

            //! Compile-time tag surface, for diagnostics and SFINAE.
            static constexpr HandleType tag = Tag;

            //! Raw bit payload. A default-constructed handle (raw == 0) is invalid.
            uint32_t raw = 0;

            /**
             * @brief Test whether this handle refers to a live resource.
             * A zero-initialized handle is the canonical invalid sentinel.
             */
            constexpr bool isValid() const {
                return raw != 0;
            }

            //! Pool slot index assigned by the backend.
            constexpr uint32_t index() const {
                return (raw >> IndexShift) & IndexMask;
            }

            //! Generation counter captured at creation time; used by the backend to detect use-after-free.
            constexpr uint32_t gen() const {
                return (raw >> GenShift) & GenMask;
            }

            //! Strongly-typed subtype (e.g. TextureType::Tex2D for a TextureHandle).
            constexpr SubEnum subType() const {
                return static_cast<SubEnum>((raw >> SubTypeShift) & SubTypeMask);
            }

            /**
             * @brief Pack an (index, gen, subType) triple into a typed handle.
             * Call site responsibility to ensure each field fits its bit width; debug builds assert.
             * @param gen generation counter supplied by the backend's pool
             * @param index pool slot index
             * @param sub resource-specific subtype (defaults to SubEnum's zero value)
             */
            static constexpr HandleT make(uint32_t gen, uint32_t index, SubEnum sub = SubEnum{}) {
                const uint32_t packed = ((static_cast<uint32_t>(sub) & SubTypeMask) << SubTypeShift) | ((gen & GenMask) << GenShift) | ((index & IndexMask) << IndexShift);
                return HandleT{packed};
            }

            friend constexpr bool operator==(HandleT a, HandleT b) {
                return a.raw == b.raw;
            }
            friend constexpr bool operator!=(HandleT a, HandleT b) {
                return a.raw != b.raw;
            }
        };
    } // namespace detail

    struct RenderDeviceBackendContext {
        // TODO: Put here common utilities / data usable for all contexts (allocators?, logging? profiling?)
    };

    enum class TextureType { Tex1D, Tex2D, Tex3D, TexCubeMap, Tex2DArray };

    /**
     * @brief Non-owning view of a single mip level's source data.
     * Held by the caller; pointers must outlive the createTexture call.
     */
    struct MipLevel {
        //! Pixel data for this level. nullptr means "allocate storage only".
        const void *data = nullptr;
    };

    /**
     * @brief Describes a texture to create on the backend.
     *
     * The mipLevels array is non-owning. For non-cube textures it holds
     * mipLevelCount entries indexed as mipLevels[mip]. For cube textures it
     * holds mipLevelCount == mipCount * 6 entries indexed as
     * mipLevels[mip * 6 + face] (mip-major, face-minor, matches KTX).
     * mipLevels == nullptr / mipLevelCount == 0 implies a single implicit
     * level 0 with null data (storage-only allocation).
     */
    /**
     * @brief Scalar pixel element types accepted by texture creation/update entry points.
     *
     * Backed by TypeEncoding: each member's value is a valid Scalar_* constant, so
     * getTypeKind / getElementSize work directly via static_cast<TypeEncoding>(v).
     */
    enum class PixelDataType : TypeEncoding {
        UInt8 = Scalar_UInt8,
        Int8 = Scalar_Int8,
        UInt16 = Scalar_UInt16,
        Int16 = Scalar_Int16,
        UInt32 = Scalar_UInt32,
        Int32 = Scalar_Int32,
        Float16 = Scalar_Float16,
        Float32 = Scalar_Float32,
    };

    struct TextureDescriptor {
        TextureType type = TextureType::Tex2D;
        PixelFormat format = PixelFormat::R8G8B8;
        Vector3i size;
        PixelFormat sourceFormat;
        PixelDataType sourceDataType = PixelDataType::UInt8;

        //! Borrowed pointer to an array of MipLevel entries. Must outlive createTexture().
        const MipLevel *mipLevels = nullptr;

        //! Number of entries in mipLevels.
        size_t mipLevelCount = 0;

        //! Generate the mip chain from level 0 via glGenerateMipmap. Mutually exclusive with supplying >1 mip level.
        bool generateMipmaps = false;
    };

    /**
     * @brief Describes a partial read from an existing buffer.
     * The backend fills [data, data + size) with bytes from the source buffer
     * starting at byte 'offset'. Synchronous; prefer for tooling / debug / snapshot use.
     */
    struct BufferReadDescriptor {
        //! Byte offset into the source buffer.
        size_t offset = 0;

        //! Number of bytes to read.
        size_t size = 0;

        //! Destination, caller-allocated, at least 'size' bytes.
        void *data = nullptr;
    };

    /**
     * @brief Describes a read of (part of) an existing texture.
     *
     * The offset/size fields describe the sub-region to read. The GL 3.3 backend
     * supports whole-mip reads only (glGetTexImage has no region variant until
     * GL 4.5 / glGetTextureSubImage); it asserts that offset == {0,0,0} and that
     * size matches the mip level's full extent. Future backends (GL 4.5+, PBO-based,
     * Vulkan) may honor arbitrary regions transparently.
     *
     * Synchronous; prefer for tooling / debug / snapshot use.
     */
    struct TextureReadDescriptor {
        //! Region origin inside the target mip level, in texels. Must be {0,0,0} on the GL 3.3 backend.
        Vector3i offset = {0, 0, 0};

        //! Region extent in texels. Must equal the mip level's full extent on the GL 3.3 backend.
        Vector3i size;

        //! Target mip level index.
        int mipLevel = 0;

        //! Cubemap face index [0,6). Ignored for non-cube targets.
        int faceIndex = 0;

        //! Desired pixel layout of the destination buffer.
        PixelFormat destFormat = PixelFormat::Unknown;

        //! Desired component data type of the destination buffer.
        PixelDataType destDataType = PixelDataType::UInt8;

        //! Destination, caller-allocated, sized for the described region.
        void *data = nullptr;
    };

    /**
     * @brief Describes a partial update to an existing texture.
     * offset + size define the region inside the target mip level (and face, for cubemaps).
     */
    struct TextureUpdateDescriptor {
        //! Region origin inside the target mip level, in texels.
        Vector3i offset = {0, 0, 0};

        //! Region extent in texels. Unused axes should be 1.
        Vector3i size;

        //! Target mip level index.
        int mipLevel = 0;

        //! Cubemap face index [0,6). Ignored for non-cube targets.
        int faceIndex = 0;

        //! Pixel layout of sourceData.
        PixelFormat sourceFormat = PixelFormat::Unknown;

        //! Component data type of sourceData.
        PixelDataType sourceDataType = PixelDataType::UInt8;

        //! Source pixel data. Must cover the region described by size.
        const void *sourceData = nullptr;
    };

    /**
     * @brief Vertex attribute element formats, backed by TypeEncoding.
     *
     * Each member is a valid Vec*_Int32 or Vec*_Float32 TypeEncoding constant.
     * Dispatch code can use getTypeKind / getTypeCols via static_cast<TypeEncoding>(fmt)
     * instead of switching over all 8 values.
     */
    enum class VertexAttribFormat : TypeEncoding {
        int1 = Scalar_Int32,
        int2 = Vec2_Int32,
        int3 = Vec3_Int32,
        int4 = Vec4_Int32,
        float1 = Scalar_Float32,
        float2 = Vec2_Float32,
        float3 = Vec3_Float32,
        float4 = Vec4_Float32,
    };

    /**
     * @brief Describes a Vertex Attribute for use in the vertex shader
     */
    struct VertexAttrib {
        int location = 0;

        VertexAttribFormat format;
        bool normalized = false;
    };

    /**
     * @brief Index buffer element type, backed by TypeEncoding.
     *
     * Values are Scalar_UInt16 and Scalar_UInt32; getElementSize gives byte width directly.
     */
    enum class GeometryIndexType : TypeEncoding {
        uint16 = Scalar_UInt16,
        uint32 = Scalar_UInt32,
    };

    enum class VertexLayoutResolveMode { Semantic, Explicit };

    struct VertexLayoutDescriptor {
        std::vector<VertexAttrib> attribs;
        GeometryIndexType indexType = GeometryIndexType::uint16;
        VertexLayoutResolveMode resolveMode = VertexLayoutResolveMode::Semantic;
    };

    // Typed handle aliases - use these at every API boundary. Each alias is a distinct C++ type,
    // so mixing kinds (e.g. passing a ShaderHandle where a BufferHandle is expected) is a compile
    // error. Runtime payload is still a single uint32_t - zero overhead vs. the original untyped
    // Handle.
    using BufferHandle = detail::HandleT<HandleBuffer, BufferType>;
    using TextureHandle = detail::HandleT<HandleTexture, TextureType>;
    using ShaderHandle = detail::HandleT<HandleShader>;
    using VertexLayoutHandle = detail::HandleT<HandleVertexLayout>;
    using PipelineHandle = detail::HandleT<HandlePipeline>;
    using GeometryHandle = detail::HandleT<HandleGeometry>;

    static_assert(sizeof(BufferHandle) == 4);
    static_assert(sizeof(TextureHandle) == 4);
    static_assert(sizeof(ShaderHandle) == 4);
    static_assert(sizeof(VertexLayoutHandle) == 4);
    static_assert(sizeof(PipelineHandle) == 4);
    static_assert(sizeof(GeometryHandle) == 4);
    static_assert(std::is_trivially_copyable_v<BufferHandle>);
    static_assert(std::is_trivially_copyable_v<TextureHandle>);

    /**
     * @brief Relates a Buffer with an Attribute index
     */
    struct GeometryBufferAttrib {
        BufferHandle bufferHandle;
        uint32_t attribIndex = 0;
    };

    struct GeometryDescriptor {
        VertexLayoutHandle layoutHandle;
        std::vector<GeometryBufferAttrib> bufferAttribs;
        BufferHandle indexBufferHandle;
    };

    struct ShaderProgramDescriptor {
        std::string glslVertexShader;
        std::string glslFragmentShader;
    };

    /**
     * @brief Opaque resolved uniform location handed back by RenderDeviceBackendVTable::resolveUniformLocation.
     *
     * Populated by the backend after a successful glGetUniformLocation (or equivalent) lookup.
     * A location is tied to the shader program it was resolved against: programKey holds the
     * raw bits of that ShaderHandle, so applyUniforms can assert (in debug builds) that a
     * location is not accidentally reused against a different program.
     */
    struct UniformLocation {
        //! Backend-specific location identifier. -1 is the canonical invalid sentinel.
        int32_t raw = -1;

        //! Raw bits of the owning ShaderHandle, captured at resolve time.
        uint32_t programKey = 0;

        //! True iff this location refers to a real uniform in a live program.
        constexpr bool isValid() const {
            return raw >= 0;
        }
    };

    /**
     * @brief One scalar/vector uniform upload in an applyUniforms batch.
     *
     * All fields must be populated. data points at count * getTypeCols(static_cast<TypeEncoding>(type))
     * elements of the C type matching the kind encoded in type (float for Float*, int for Int*, unsigned
     * int for UInt*), and must outlive the applyUniforms call.
     */
    struct UniformValueSubmission {
        //! Resolved target location. Must refer to the same program passed to applyUniforms.
        UniformLocation location;

        //! Encodes element kind (Float/Int/UInt) and vector width (1..4).
        UniformVectorType type = UniformVectorType::Float1;

        //! Number of array elements; 1 for a scalar/vector uniform, N for a uniform array.
        uint32_t count = 1;

        //! Borrowed pointer to the upload payload; must outlive applyUniforms.
        const void *data = nullptr;
    };

    /**
     * @brief One matrix uniform upload in an applyUniforms batch.
     *
     * data points at count * getTypeCols(te) * getTypeRows(te) GLfloat elements, where
     * te = static_cast<TypeEncoding>(shape). Only Float32 precision is supported.
     */
    struct UniformMatrixSubmission {
        //! Resolved target location. Must refer to the same program passed to applyUniforms.
        UniformLocation location;

        //! Matrix shape (rows x columns).
        UniformMatrixShape shape = UniformMatrixShape::R4C4;

        //! Number of matrices; 1 for a single uniform, N for an array of matrices.
        uint32_t count = 1;

        //! When true, the backend transposes each matrix on upload (glUniformMatrix*fv transpose flag).
        bool transpose = false;

        //! Borrowed pointer to the upload payload; must outlive applyUniforms.
        const void *data = nullptr;
    };

    /**
     * @brief Pipeline-level declaration that a named uniform block maps to a specific binding point.
     *
     * Consumed by createPipeline at pipeline creation time: the backend calls whatever API it needs
     * (e.g. glUniformBlockBinding on GL 3.3) to bake the block-name-to-binding-point assignment into
     * the pipeline's shader program. At draw time, clients call bindUniformBuffer with the matching
     * bindingPoint to attach actual buffer storage; no block-name lookup is repeated per draw.
     */
    struct PipelineUniformBlock {
        //! Uniform block name as it appears in the GLSL source (e.g. "CameraBlock").
        std::string blockName;

        //! Binding point to assign this block to. Must be < GL_MAX_UNIFORM_BUFFER_BINDINGS on GL backends.
        uint32_t bindingPoint = 0;
    };

    enum class PolygonMode { Point, Line, Fill };

    enum class DepthFunc { Never, Less, Equal, LesserEqual, Greater, NotEqual, GreaterEqual, Always };

    enum class FrontFaceOrder { Clockwise, CounterClockwise };

    enum class BlendParam {
        Zero,
        One,
        SourceColor,
        OneMinusSourceColor,
        DestinationColor,
        OneMinusDestinationColor,
        SourceAlpha,
        OneMinusSourceAlpha,
        DestinationAlpha,
        OneMinusDestinationAlpha,
        ConstantColor,
        OneMinusConstantColor,
        ConstantAlpha,
        OneMinusConstantAlpha
    };

    struct PipelineDescriptor {
        VertexLayoutHandle layoutHandle;
        ShaderHandle shaderProgramHandle;
        ClearFlags clearFlags = ClearFlags::Color;
        xe::Vector4 clearColor = {0.0f, 0.0f, 0.0f, 1.0f};

        bool depthTest = true;
        bool stencilTest = false;
        bool cullBackFace = false;
        bool blendEnable = false;

        BlendParam blendSource = BlendParam::One;
        BlendParam blendDestination = BlendParam::One;

        float lineWidth = 1.0f;
        DepthFunc depthFunc = DepthFunc::LesserEqual;
        FrontFaceOrder frontFace = FrontFaceOrder::Clockwise;

        int clipDistanceCount = 0;

        //! Uniform block bindings baked into this pipeline at creation time.
        std::vector<PipelineUniformBlock> uniformBlocks;
    };

    enum class TextureFilter { Nearest, Linear };

    enum class TextureWrap { Repeat, Clamp };

    struct SamplerDescriptor {
        TextureFilter minFilter = TextureFilter::Nearest;
        TextureFilter magFilter = TextureFilter::Nearest;
        TextureWrap wrapS = TextureWrap::Repeat;
        TextureWrap wrapT = TextureWrap::Repeat;
        TextureWrap wrapR = TextureWrap::Repeat;
    };

    // Command Buffer API
    // It encapsulates inmediate-mode commands

    // Identify which command
    enum class CommandOp { Noop, Clear, Draw, SetUniform, SetUniformMatrix, BindTexture, BindPipeline };

    template <typename> struct CommandTraits {
        static const CommandOp op = CommandOp::Noop;
    };

    struct CommandClear {
        ClearFlags flags = ClearFlags::Color;
        xe::Vector4 color = {0.0f, 0.0f, 0.0f, 1.0f};
        float depth = 1.0f;
        int stencil = 0;
    };

    template <> struct CommandTraits<CommandClear> {
        CommandOp op = CommandOp::Clear;
    };

    enum class PrimitiveType { Points, Lines, LineStrip, LineLoop, Triangles, TriangleStrip, TriangleFan };

    struct CommandDraw {
        PrimitiveType primitiveType = PrimitiveType::TriangleStrip;
        GeometryHandle geometry;
    };

    template <> struct CommandTraits<CommandDraw> {
        CommandOp op = CommandOp::Draw;
    };

    struct CommandSetUniform {
        UniformValueSubmission *uniforms = nullptr;
        size_t count = 0;
    };

    template <> struct CommandTraits<CommandSetUniform> {
        CommandOp op = CommandOp::SetUniform;
    };

    struct CommandSetUniformMatrix {
        UniformMatrixSubmission *uniforms = nullptr;
        size_t count = 0;
    };

    template <> struct CommandTraits<CommandSetUniformMatrix> {
        CommandOp op = CommandOp::SetUniformMatrix;
    };

    struct CommandBindTexture {
        //! The uniform to use
        uint32_t bindingPoint = 0;

        //! A valid texture handle
        TextureHandle textureHandle;
    };

    template <> struct CommandTraits<CommandBindTexture> {
        CommandOp op = CommandOp::BindTexture;
    };

    struct CommandBindPipeline {
        PipelineHandle pipelineHandle;
    };

    template <> struct CommandTraits<CommandBindPipeline> {
        CommandOp op = CommandOp::BindPipeline;
    };

    // ------------------------------------------------------------------------
    // CommandBuffer — cleanup notes (WIP, not yet applied)
    // ------------------------------------------------------------------------
    //
    // Bugs to fix first:
    //   * Command(const CommandDraw &) sets opcode to CommandOp::Clear instead
    //     of CommandOp::Draw — every draw will misdispatch at submit time.
    //   * record(const CommandClear &) is declared but has no inline definition
    //     below; first caller to record a clear produces a linker error.
    //   * CommandBuffer exposes no accessor for the recorded stream, so the
    //     backend's submit() has no way to read it. Add:
    //         const Command *data() const;
    //         std::size_t    size() const;
    //
    // Simplifications (keep pre-C++17 compatibility):
    //   * Replace the five record(...) overloads with a single templated
    //         template <class T> void record(const T &cmd);
    //     gated by a CommandTraits<T>::op specialization that maps payload
    //     type -> CommandOp. Makes the opcode impossible to get wrong by
    //     construction, and new command types need one traits line + one
    //     union member + one Payload ctor.
    //   * Use member-init lists in the union constructors
    //         Payload(const CommandClear &c) : clear(c) {}
    //     instead of the "assign-to-inactive-member" body form. Current form
    //     works only because every payload is trivially copyable.
    //   * Lock the trivially-destructible invariant with static_asserts on
    //     each payload type; the union leaks silently the moment someone adds
    //     a std::string / std::vector field otherwise.
    //
    // Standard-library note:
    //   std::variant is C++17 (not C++20), so on a C++17 target the whole
    //   CommandOp + Payload union + Command wrapper collapses to
    //       using Command = std::variant<CommandClear, CommandDraw, ...>;
    //   and dispatch is std::visit. Blocker: GCC 5.1 (x64/x86-win-gcc
    //   profiles) ships libstdc++ < 7 and has no <variant> header. Revisit
    //   if/when those profiles are dropped or bumped.
    // ------------------------------------------------------------------------

    /**
     * @brief Records a series of render commands for subsequent execution
     *
     * Intended to be instanciated directly by the app
     */
    class CommandBuffer {
    public:
        //! use a Union instead to make porting to old standard easier
        union CommandUnion {
            CommandClear clear;
            CommandBindTexture bindTexture;
            CommandBindPipeline bindPipeline;
            CommandSetUniform setUniform;
            CommandSetUniformMatrix setUniformMatrix;
            CommandDraw draw;

            CommandUnion(const CommandClear &clear) {
                this->clear = clear;
            }
            CommandUnion(const CommandBindTexture &bindTexture) {
                this->bindTexture = bindTexture;
            }
            CommandUnion(const CommandBindPipeline &bindPipeline) {
                this->bindPipeline = bindPipeline;
            }
            CommandUnion(const CommandSetUniform &uniform) {
                this->setUniform = uniform;
            }
            CommandUnion(const CommandSetUniformMatrix &uniform) {
                this->setUniformMatrix = uniform;
            }
            CommandUnion(const CommandDraw &draw) {
                this->draw = draw;
            }
        };

        struct Command {
            CommandOp opcode;
            CommandUnion cmd;
        };

        void clear();

        template <typename CommandT> void record(const CommandT &cmd) {
            commands.push_back({CommandTraits<CommandT>::op, cmd});
        }

        const Command *getCommandPtr() const {
            return commands.data();
        }

        size_t getCommandCount() const {
            return commands.size();
        }

    private:
        std::vector<Command> commands;
    };
} // namespace xe

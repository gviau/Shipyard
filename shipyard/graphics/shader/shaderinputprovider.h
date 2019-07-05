#pragma once

#include <system/platform.h>

#include <math/mathtypes.h>

#include <graphics/graphicstypes.h>

#include <system/array.h>
#include <system/string.h>

#include <type_traits>

namespace Shipyard
{
    enum class ShaderInputProviderUsage : shipUint8
    {
        Default,
        PerInstance,

        Count
    };

    class ShaderInputProviderDeclaration;

    class SHIPYARD_API ShaderInputProvider
    {
        friend class ShaderHandler;
        friend class ShaderInputProviderManager;
        friend class ShaderInputProviderDeclaration;
        friend class ShaderResourceBinder;

    public:
        ShaderInputProvider();
        ~ShaderInputProvider();

    protected:
        virtual shipUint32 GetRequiredSizeForProvider() const = 0;
        virtual shipUint32 GetShaderInputProviderDeclarationIndex() const = 0;
        virtual ShaderInputProviderDeclaration* GetShaderInputProviderDeclaration() const = 0;

        GFXConstantBufferHandle m_GfxConstantBufferHandle;
    };

    class ShaderInputProviderManager;
    SHIPYARD_API ShaderInputProviderManager& GetShaderInputProviderManager();

    template <typename DerivedType>
    class SHIPYARD_API BaseShaderInputProvider : public ShaderInputProvider
    {
    public:
        BaseShaderInputProvider()
        {
            if (ShaderInputProviderUtils::IsUsingConstantBuffer(ms_ShaderInputProviderDeclaration->GetShaderInputProviderUsage()))
            {
                GetShaderInputProviderManager().CreateShaderInputProviderConstantBuffer(this);
            }
        }

        ~BaseShaderInputProvider()
        {
            if (ShaderInputProviderUtils::IsUsingConstantBuffer(ms_ShaderInputProviderDeclaration->GetShaderInputProviderUsage()))
            {
                GetShaderInputProviderManager().DestroyShaderInputProviderConstantBuffer(this);
            }
        }

    public:
        static ShaderInputProviderDeclaration* ms_ShaderInputProviderDeclaration;

    protected:
        virtual shipUint32 GetRequiredSizeForProvider() const override;
        virtual shipUint32 GetShaderInputProviderDeclarationIndex() const override;
        virtual ShaderInputProviderDeclaration* GetShaderInputProviderDeclaration() const override;

    private:
        BaseShaderInputProvider(const BaseShaderInputProvider& src) = delete;
    };

    template <typename DerivedType> SHIPYARD_API ShaderInputProviderDeclaration* BaseShaderInputProvider<DerivedType>::ms_ShaderInputProviderDeclaration = nullptr;

    enum class ShaderInputType : shipUint8
    {
        Scalar,
        Texture2D
    };

    enum class ShaderInputScalarType : shipUint8
    {
        Float,
        Float2,
        Float3,
        Float4,

        Half,
        Half2,
        Half3,
        Half4,

        Double,
        Double2,
        Double3,
        Double4,

        Int,
        Int2,
        Int3,
        Int4,

        Uint,
        Uint2,
        Uint3,
        Uint4,

        Bool,
        Bool2,
        Bool3,
        Bool4,

        Float2x2,
        Float3x3,
        Float4x4,

        Half2x2,
        Half3x3,
        Half4x4,

        Double2x2,
        Double3x3,
        Double4x4,

        Unknown,
        Count = Unknown
    };

    class SHIPYARD_API ShaderInputProviderDeclaration
    {
        friend class ShaderInputProviderManager;

    public:
        struct ShaderInputDeclaration
        {
            const shipChar* Name = "";
            ShaderInputType Type = ShaderInputType::Scalar;
            ShaderInputScalarType ScalarType = ShaderInputScalarType::Unknown;
            shipInt32 DataOffsetInProvider = 0;
            shipInt32 DataOffsetInBuffer = 0;
            size_t DataSize = 0;
        };

        enum
        {
            MaxShaderInputsPerProvider = 1024,
        };

    public:
        ShaderInputProviderDeclaration();

        const ShaderInputDeclaration* GetShaderInputDeclarations(shipUint32& numShaderInputDeclarations) const;

        shipUint32 GetRequiredSizeForProvider() const;
        shipUint32 GetShaderInputProviderDeclarationIndex() const;
        ShaderInputProviderUsage GetShaderInputProviderUsage() const;

    protected:
        const shipChar* m_ShaderInputProviderName;
        ShaderInputDeclaration m_ShaderInputDeclarations[MaxShaderInputsPerProvider];
        ShaderInputProviderUsage m_ShaderInputProviderUsage;
        shipUint32 m_NumShaderInputDeclarations;
        shipUint32 m_RequiredSizeForProvider;
        shipUint32 m_ShaderInputProviderDeclarationIndex;

        ShaderInputProviderDeclaration* m_pNextRegisteredShaderInputProviderDeclaration;
    };

    class BaseRenderDevice;

    class SHIPYARD_API ShaderInputProviderManager
    {
    public:
        static ShaderInputProviderManager& GetInstance()
        {
            static ShaderInputProviderManager s_ShaderInputProviderManager;
            return s_ShaderInputProviderManager;
        }

        void RegisterShaderInputProviderDeclaration(ShaderInputProviderDeclaration& shaderInputProviderDeclaration);

        shipBool Initialize(BaseRenderDevice& gfxRenderDevice);
        void Destroy();

        shipUint32 GetRequiredSizeForProvider(const ShaderInputProvider& shaderInputProvider) const;

        shipUint32 GetShaderInputProviderDeclarationIndex(const ShaderInputProvider& shaderInputProvider) const;

        void GetShaderInputProviderDeclarations(Array<ShaderInputProviderDeclaration*>& shaderInputProviderDeclarations) const;

        void CopyShaderInputsToBuffer(const ShaderInputProvider& shaderInputProvider, void* pBuffer) const;

        void CreateShaderInputProviderConstantBuffer(ShaderInputProvider* shaderInputProvider);
        void DestroyShaderInputProviderConstantBuffer(ShaderInputProvider* shaderInputProvider);

        shipUint32 GetTexture2DHandlesFromProvider(const ShaderInputProvider& shaderInputProvider, GFXTexture2DHandle* pGfxTextureHandles) const;

    private:
        ShaderInputProviderManager();
        ~ShaderInputProviderManager();

        ShaderInputProviderManager(const ShaderInputProviderManager& src) = delete;
        ShaderInputProviderManager(const ShaderInputProviderManager&& src) = delete;
        ShaderInputProviderManager& operator= (const ShaderInputProviderManager& rhs) = delete;

        shipBool WriteEveryShaderInputProviderFile();

        shipBool WriteShaderInputProviderUtilsFile();

        shipBool WriteSingleShaderInputProviderFile(ShaderInputProviderDeclaration* pShaderInputProviderDeclaration, size_t numBytesBeforeThisProviderInUnifiedBuffer);
        shipBool WriteSingleShaderScalarInput(const ShaderInputProviderDeclaration::ShaderInputDeclaration& shaderInputDeclaration, StringA& content, shipBool isUsingConstantBuffer);
        shipBool WriteShaderInputsOutsideOfStruct(ShaderInputProviderDeclaration* pShaderInputProviderDeclaration, StringA& content);
        shipBool WriteSingleShaderInputOutsideOfStruct(const ShaderInputProviderDeclaration::ShaderInputDeclaration& shaderInputDeclaration, StringA& content);
        shipBool WriteShaderInputProviderUnifiedBufferAccessMethod(ShaderInputProviderDeclaration* pShaderInputProviderDeclaration, StringA& content);
        shipBool WriteShaderInputScalarLoadFromBuffer(const ShaderInputProviderDeclaration::ShaderInputDeclaration& shaderInputDeclaration, StringA& content);

        ShaderInputProviderDeclaration* m_pHead;

        BaseRenderDevice* m_GfxRenderDevice;
    };

    namespace ShaderInputProviderUtils
    {
        shipBool SHIPYARD_API IsUsingConstantBuffer(ShaderInputProviderUsage shaderInputProviderUsage);

        template <typename T>
        void SHIPYARD_API VerifyTexture2DVariableType(const T& shaderInputData)
        {
            SHIP_STATIC_ASSERT_MSG((std::is_same<T, GFXTexture2DHandle>::value), "Texture2D shader inputs only support GFXTexture2DHandle as valid data input");
        }
    };

    constexpr size_t gs_ShaderInputScalarTypeSizeInBytes[shipUint32(ShaderInputScalarType::Count)] =
    {
        sizeof(shipFloat), // Float
        sizeof(shipFloat) * 2, // Float2
        sizeof(shipFloat) * 3, // Float3
        sizeof(shipFloat) * 4, // Float4
        
        sizeof(shipFloat), // Half
        sizeof(shipFloat) * 2, // Half2
        sizeof(shipFloat) * 3, // Half3
        sizeof(shipFloat) * 4, // Half4

        sizeof(shipDouble), // Double
        sizeof(shipDouble) * 2, // Double2
        sizeof(shipDouble) * 3, // Double3
        sizeof(shipDouble) * 4, // Double4

        sizeof(shipInt32), // Int
        sizeof(shipInt32) * 2, // Int2
        sizeof(shipInt32) * 3, // Int3
        sizeof(shipInt32) * 4, // Int4

        sizeof(shipUint32), // Uint
        sizeof(shipUint32) * 2, // Uint2
        sizeof(shipUint32) * 3, // Uint3
        sizeof(shipUint32) * 4, // Uint4

        sizeof(shipBool), // Bool
        sizeof(shipBool) * 2,// Bool2
        sizeof(shipBool) * 3,// Bool3
        sizeof(shipBool) * 4,// Bool4

        sizeof(shipFloat) * 2 * 2, // Float2x2
        sizeof(shipFloat) * 3 * 3, // Float3x3
        sizeof(shipFloat) * 4 * 4, // Float4x4

        sizeof(shipFloat) * 2 * 2, // Half2x2
        sizeof(shipFloat) * 3 * 3, // Half3x3
        sizeof(shipFloat) * 4 * 4, // Half4x4

        sizeof(shipDouble) * 2 * 2, // Double2x2
        sizeof(shipDouble) * 3 * 3, // Double3x3
        sizeof(shipDouble) * 4 * 4, // Double4x4
    };

    template <typename T> SHIPYARD_API ShaderInputScalarType GetShaderInputScalarType(const T& data);
    template <> SHIPYARD_API ShaderInputScalarType GetShaderInputScalarType(const shipFloat& data);
    template <> SHIPYARD_API ShaderInputScalarType GetShaderInputScalarType(const shipVec2& data);
    template <> SHIPYARD_API ShaderInputScalarType GetShaderInputScalarType(const shipVec3& data);
    template <> SHIPYARD_API ShaderInputScalarType GetShaderInputScalarType(const shipVec4& data);

    template <> SHIPYARD_API ShaderInputScalarType GetShaderInputScalarType(const shipDouble& data);
    template <> SHIPYARD_API ShaderInputScalarType GetShaderInputScalarType(const shipDVec2& data);
    template <> SHIPYARD_API ShaderInputScalarType GetShaderInputScalarType(const shipDVec3& data);
    template <> SHIPYARD_API ShaderInputScalarType GetShaderInputScalarType(const shipDVec4& data);

    template <> SHIPYARD_API ShaderInputScalarType GetShaderInputScalarType(const shipInt32& data);
    template <> SHIPYARD_API ShaderInputScalarType GetShaderInputScalarType(const shipIVec2& data);
    template <> SHIPYARD_API ShaderInputScalarType GetShaderInputScalarType(const shipIVec3& data);
    template <> SHIPYARD_API ShaderInputScalarType GetShaderInputScalarType(const shipIVec4& data);

    template <> SHIPYARD_API ShaderInputScalarType GetShaderInputScalarType(const shipUint32& data);
    template <> SHIPYARD_API ShaderInputScalarType GetShaderInputScalarType(const shipUVec2& data);
    template <> SHIPYARD_API ShaderInputScalarType GetShaderInputScalarType(const shipUVec3& data);
    template <> SHIPYARD_API ShaderInputScalarType GetShaderInputScalarType(const shipUVec4& data);

    template <> SHIPYARD_API ShaderInputScalarType GetShaderInputScalarType(const shipBool& data);
    template <> SHIPYARD_API ShaderInputScalarType GetShaderInputScalarType(const shipBVec2& data);
    template <> SHIPYARD_API ShaderInputScalarType GetShaderInputScalarType(const shipBVec3& data);
    template <> SHIPYARD_API ShaderInputScalarType GetShaderInputScalarType(const shipBVec4& data);

    template <> SHIPYARD_API ShaderInputScalarType GetShaderInputScalarType(const shipMat2x2& data);
    template <> SHIPYARD_API ShaderInputScalarType GetShaderInputScalarType(const shipMat3x3& data);
    template <> SHIPYARD_API ShaderInputScalarType GetShaderInputScalarType(const shipMat4x4& data);

    template <> SHIPYARD_API ShaderInputScalarType GetShaderInputScalarType(const shipDMat2x2& data);
    template <> SHIPYARD_API ShaderInputScalarType GetShaderInputScalarType(const shipDMat3x3& data);
    template <> SHIPYARD_API ShaderInputScalarType GetShaderInputScalarType(const shipDMat4x4& data);

    // These macros shouldn't be used directly.
#define SHIP_SHADER_INPUT_BASE(shaderInputScalarType, shaderInputName) \
    SHIP_ASSERT_MSG(m_NumShaderInputDeclarations < MaxShaderInputsPerProvider, "ShaderInputProvider exceeds maximum number of %d shader inputs per provider!", MaxShaderInputsPerProvider); \
    SHIP_ASSERT_MSG(shaderInputName[0] != '\0', "ShaderInput name cannot be empty!");

    // These macros are used to create the ShaderInputProviders
#define SHIP_DECLARE_SHADER_INPUT_PROVIDER_BEGIN_INTERNAL_1(shaderInputProviderTypeClassName, shaderInputProviderType, shaderInputProviderUsage) \
    template <typename ShaderInputProviderType> \
    class shaderInputProviderTypeClassName : public ShaderInputProviderDeclaration \
    { \
    public: \
        shaderInputProviderTypeClassName() \
        { \
            m_ShaderInputProviderName = #shaderInputProviderType; \
            m_ShaderInputProviderUsage = ShaderInputProviderUsage::##shaderInputProviderUsage; \
            shipInt32 offsetInBuffer = 0;

#define SHIP_DECLARE_SHADER_INPUT_PROVIDER_BEGIN(shaderInputProviderType, shaderInputProviderUsage) \
    SHIP_DECLARE_SHADER_INPUT_PROVIDER_BEGIN_INTERNAL_1(##shaderInputProviderType_StaticDeclaration, ##shaderInputProviderType, shaderInputProviderUsage)

#define SHIP_DECLARE_SHADER_INPUT_PROVIDER_END_INTERNAL_1(shaderInputProviderTypeStaticDeclarationType, shaderInputProviderType, shaderInputProviderDeclarationName) \
            ShaderInputProviderType::ms_ShaderInputProviderDeclaration = this; \
        } \
    }; static shaderInputProviderTypeStaticDeclarationType<shaderInputProviderType> shaderInputProviderDeclarationName;

#define SHIP_DECLARE_SHADER_INPUT_PROVIDER_END(shaderInputProviderType) \
    SHIP_DECLARE_SHADER_INPUT_PROVIDER_END_INTERNAL_1(##shaderInputProviderType_StaticDeclaration, shaderInputProviderType, gs_##shaderInputProviderType_StaticDeclaration)

#define SHIP_SHADER_INPUT_INTERNAL(shaderInputType, shaderInputScalarType, shaderInputName, shaderInputData) \
        m_ShaderInputDeclarations[m_NumShaderInputDeclarations].Name = shaderInputName; \
        m_ShaderInputDeclarations[m_NumShaderInputDeclarations].Type = shaderInputType; \
        m_ShaderInputDeclarations[m_NumShaderInputDeclarations].ScalarType = shaderInputScalarType; \
        void* pDataPtr = &pBasePtr->shaderInputData; \
        m_ShaderInputDeclarations[m_NumShaderInputDeclarations].DataOffsetInProvider = static_cast<shipInt32>(size_t(pDataPtr)) - static_cast<shipInt32>(size_t(pBasePtr)); \
        m_ShaderInputDeclarations[m_NumShaderInputDeclarations].DataOffsetInBuffer = offsetInBuffer; \
        m_ShaderInputDeclarations[m_NumShaderInputDeclarations].DataSize = sizeof(pBasePtr->shaderInputData); \
        offsetInBuffer += sizeof(pBasePtr->shaderInputData); \
        m_NumShaderInputDeclarations += 1; \
        m_RequiredSizeForProvider += ((shaderInputType == ShaderInputType::Scalar) ? sizeof(pBasePtr->shaderInputData) : 0);

    // These macros are used to create the ShaderInputs of a provider
#define SHIP_SCALAR_SHADER_INPUT(shaderInputName, shaderInputData) \
    { \
        ShaderInputProviderType* pBasePtr = (ShaderInputProviderType*)0x10000; \
        ShaderInputScalarType shaderInputScalarType = GetShaderInputScalarType(pBasePtr->shaderInputData); \
        SHIP_SHADER_INPUT_BASE(shaderInputScalarType, shaderInputName) \
        SHIP_SHADER_INPUT_INTERNAL(ShaderInputType::Scalar, shaderInputScalarType, shaderInputName, shaderInputData) \
    }

#define SHIP_TEXTURE2D_SHADER_INPUT(shaderInputScalarType, shaderInputName, shaderInputData) \
    { \
        SHIP_SHADER_INPUT_BASE(shaderInputScalarType, shaderInputName) \
        SHIP_STATIC_ASSERT_MSG(shaderInputScalarType != ShaderInputScalarType::Unknown, "ShaderInput type cannot be set to ShaderInputScalarType::Unknown!"); \
        ShaderInputProviderType* pBasePtr = (ShaderInputProviderType*)0x10000; \
        ShaderInputProviderUtils::VerifyTexture2DVariableType(pBasePtr->shaderInputData); \
        SHIP_SHADER_INPUT_INTERNAL(ShaderInputType::Texture2D, shaderInputScalarType, shaderInputName, shaderInputData) \
    }
}

#include <graphics/shader/shaderinputprovider.inl>
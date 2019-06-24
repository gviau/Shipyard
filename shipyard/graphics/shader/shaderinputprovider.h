#pragma once

#include <system/platform.h>

#include <graphics/graphicstypes.h>

#include <system/array.h>
#include <system/string.h>

#include <cinttypes>

namespace Shipyard
{
    enum class ShaderInputProviderUsage : uint8_t
    {
        Default,
        PerInstance,

        Count
    };

    namespace ShaderInputProviderUtils
    {
        bool SHIPYARD_API IsUsingConstantBuffer(ShaderInputProviderUsage shaderInputProviderUsage);
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
        virtual uint32_t GetRequiredSizeForProvider() const = 0;
        virtual uint32_t GetShaderInputProviderDeclarationIndex() const = 0;
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
        virtual uint32_t GetRequiredSizeForProvider() const override;
        virtual uint32_t GetShaderInputProviderDeclarationIndex() const override;
        virtual ShaderInputProviderDeclaration* GetShaderInputProviderDeclaration() const override;

    private:
        BaseShaderInputProvider(const BaseShaderInputProvider& src) = delete;
    };

    template <typename DerivedType> SHIPYARD_API ShaderInputProviderDeclaration* BaseShaderInputProvider<DerivedType>::ms_ShaderInputProviderDeclaration = nullptr;

    enum class ShaderInputType : uint8_t
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
            const char* Name = "";
            ShaderInputType Type = ShaderInputType::Unknown;
            int32_t DataOffsetInProvider = 0;
            int32_t DataOffsetInBuffer = 0;
            size_t DataSize = 0;
        };

        enum
        {
            MaxShaderInputsPerProvider = 1024,
        };

    public:
        ShaderInputProviderDeclaration();

        const ShaderInputDeclaration* GetShaderInputDeclarations(uint32_t& numShaderInputDeclarations) const;

        uint32_t GetRequiredSizeForProvider() const;
        uint32_t GetShaderInputProviderDeclarationIndex() const;
        ShaderInputProviderUsage GetShaderInputProviderUsage() const;

    protected:
        const char* m_ShaderInputProviderName;
        ShaderInputDeclaration m_ShaderInputDeclarations[MaxShaderInputsPerProvider];
        ShaderInputProviderUsage m_ShaderInputProviderUsage;
        uint32_t m_NumShaderInputDeclarations;
        uint32_t m_RequiredSizeForProvider;
        uint32_t m_ShaderInputProviderDeclarationIndex;

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

        bool Initialize(BaseRenderDevice& gfxRenderDevice);
        void Destroy();

        uint32_t GetRequiredSizeForProvider(const ShaderInputProvider& shaderInputProvider) const;

        uint32_t GetShaderInputProviderDeclarationIndex(const ShaderInputProvider& shaderInputProvider) const;

        void GetShaderInputProviderDeclarations(Array<ShaderInputProviderDeclaration*>& shaderInputProviderDeclarations) const;

        void CopyShaderInputsToBuffer(const ShaderInputProvider& shaderInputProvider, void* pBuffer) const;

        void CreateShaderInputProviderConstantBuffer(ShaderInputProvider* shaderInputProvider);
        void DestroyShaderInputProviderConstantBuffer(ShaderInputProvider* shaderInputProvider);

    private:
        ShaderInputProviderManager();
        ~ShaderInputProviderManager();

        ShaderInputProviderManager(const ShaderInputProviderManager& src) = delete;
        ShaderInputProviderManager(const ShaderInputProviderManager&& src) = delete;
        ShaderInputProviderManager& operator= (const ShaderInputProviderManager& rhs) = delete;

        bool WriteEveryShaderInputProviderFile();
        bool WriteSingleShaderInputProviderFile(ShaderInputProviderDeclaration* pShaderInputProviderDeclaration);
        bool WriteSingleShaderInput(const ShaderInputProviderDeclaration::ShaderInputDeclaration& shaderInputDeclaration, StringA& content);

        ShaderInputProviderDeclaration* m_pHead;

        BaseRenderDevice* m_GfxRenderDevice;
    };

    constexpr size_t gs_ShaderInputTypeSizeInBytes[uint32_t(ShaderInputType::Count)] =
    {
        sizeof(float), // Float
        sizeof(float) * 2, // Float2
        sizeof(float) * 3, // Float3
        sizeof(float) * 4, // Float4
        
        sizeof(float), // Half
        sizeof(float) * 2, // Half2
        sizeof(float) * 3, // Half3
        sizeof(float) * 4, // Half4

        sizeof(double), // Double
        sizeof(double) * 2, // Double2
        sizeof(double) * 3, // Double3
        sizeof(double) * 4, // Double4

        sizeof(int32_t), // Int
        sizeof(int32_t) * 2, // Int2
        sizeof(int32_t) * 3, // Int3
        sizeof(int32_t) * 4, // Int4

        sizeof(uint32_t), // Uint
        sizeof(uint32_t) * 2, // Uint2
        sizeof(uint32_t) * 3, // Uint3
        sizeof(uint32_t) * 4, // Uint4

        sizeof(bool), // Bool
        sizeof(bool) * 2,// Bool2
        sizeof(bool) * 3,// Bool3
        sizeof(bool) * 4,// Bool4

        sizeof(float) * 2 * 2, // Float2x2
        sizeof(float) * 3 * 3, // Float3x3
        sizeof(float) * 4 * 4, // Float4x4

        sizeof(float) * 2 * 2, // Half2x2
        sizeof(float) * 3 * 3, // Half3x3
        sizeof(float) * 4 * 4, // Half4x4

        sizeof(double) * 2 * 2, // Double2x2
        sizeof(double) * 3 * 3, // Double3x3
        sizeof(double) * 4 * 4, // Double4x4
    };

    // These macros shouldn't be used directly.
#define SHIP_SHADER_INPUT_BASE(shaderInputType, shaderInputName) \
    SHIP_ASSERT_MSG(m_NumShaderInputDeclarations < MaxShaderInputsPerProvider, "ShaderInputProvider exceeds maximum number of %d shader inputs per provider!", MaxShaderInputsPerProvider); \
    SHIP_STATIC_ASSERT_MSG(shaderInputType != ShaderInputType::Unknown, "ShaderInput type cannot be set to ShaderInputType::Unknown!"); \
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
            int32_t offsetInBuffer = 0;

#define SHIP_DECLARE_SHADER_INPUT_PROVIDER_BEGIN(shaderInputProviderType, shaderInputProviderUsage) \
    SHIP_DECLARE_SHADER_INPUT_PROVIDER_BEGIN_INTERNAL_1(##shaderInputProviderType_StaticDeclaration, ##shaderInputProviderType, shaderInputProviderUsage)

#define SHIP_DECLARE_SHADER_INPUT_PROVIDER_END_INTERNAL_1(shaderInputProviderTypeStaticDeclarationType, shaderInputProviderType, shaderInputProviderDeclarationName) \
            ShaderInputProviderType::ms_ShaderInputProviderDeclaration = this; \
        } \
    }; static shaderInputProviderTypeStaticDeclarationType<shaderInputProviderType> shaderInputProviderDeclarationName;

#define SHIP_DECLARE_SHADER_INPUT_PROVIDER_END(shaderInputProviderType) \
    SHIP_DECLARE_SHADER_INPUT_PROVIDER_END_INTERNAL_1(##shaderInputProviderType_StaticDeclaration, shaderInputProviderType, gs_##shaderInputProviderType_StaticDeclaration)

    // These macros are used to create the ShaderInputs of a provider
#define SHIP_SCALAR_SHADER_INPUT(shaderInputType, shaderInputName, shaderInputData) \
    { \
        SHIP_SHADER_INPUT_BASE(shaderInputType, shaderInputName) \
        ShaderInputProviderType* pBasePtr = (ShaderInputProviderType*)0x10000; \
        SHIP_STATIC_ASSERT_MSG(sizeof(pBasePtr->shaderInputData) == gs_ShaderInputTypeSizeInBytes[uint32_t(shaderInputType)], "Invalid shader input size!"); \
        m_ShaderInputDeclarations[m_NumShaderInputDeclarations].Name = shaderInputName; \
        m_ShaderInputDeclarations[m_NumShaderInputDeclarations].Type = shaderInputType; \
        void* pDataPtr = &pBasePtr->shaderInputData; \
        m_ShaderInputDeclarations[m_NumShaderInputDeclarations].DataOffsetInProvider = static_cast<int32_t>(size_t(pDataPtr)) - static_cast<int32_t>(size_t(pBasePtr)); \
        m_ShaderInputDeclarations[m_NumShaderInputDeclarations].DataOffsetInBuffer = offsetInBuffer; \
        m_ShaderInputDeclarations[m_NumShaderInputDeclarations].DataSize = sizeof(pBasePtr->shaderInputData); \
        offsetInBuffer += sizeof(pBasePtr->shaderInputData); \
        m_NumShaderInputDeclarations += 1; \
        m_RequiredSizeForProvider += sizeof(pBasePtr->shaderInputData); \
    }
}

#include <graphics/shader/shaderinputprovider.inl>
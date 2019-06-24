namespace Shipyard
{;

template <class T>
uint32_t BaseShaderInputProvider<T>::GetRequiredSizeForProvider() const
{
    return ms_ShaderInputProviderDeclaration->GetRequiredSizeForProvider();
}

template <class T>
uint32_t BaseShaderInputProvider<T>::GetShaderInputProviderDeclarationIndex() const
{
    return ms_ShaderInputProviderDeclaration->GetShaderInputProviderDeclarationIndex();
}

template <class T>
ShaderInputProviderDeclaration* BaseShaderInputProvider<T>::GetShaderInputProviderDeclaration() const
{
    return ms_ShaderInputProviderDeclaration;
}

}
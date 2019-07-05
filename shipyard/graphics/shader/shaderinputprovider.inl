namespace Shipyard
{;

template <class T>
shipUint32 BaseShaderInputProvider<T>::GetRequiredSizeForProvider() const
{
    return ms_ShaderInputProviderDeclaration->GetRequiredSizeForProvider();
}

template <class T>
shipUint32 BaseShaderInputProvider<T>::GetShaderInputProviderDeclarationIndex() const
{
    return ms_ShaderInputProviderDeclaration->GetShaderInputProviderDeclarationIndex();
}

template <class T>
ShaderInputProviderDeclaration* BaseShaderInputProvider<T>::GetShaderInputProviderDeclaration() const
{
    return ms_ShaderInputProviderDeclaration;
}

}
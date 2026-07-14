#include "Utils/AvatarDataRPC.h"
#include "ShibSaveGame.h"

void FAvatarDataRPC::CopySaveFileToAvatarData(const FAvatarSave& SaveFile, FAvatarDataRPC& NewAvatarData)
{
    NewAvatarData.AvatarBaseRow = SaveFile.AvatarBaseRow;
    NewAvatarData.bIsMale = SaveFile.bIsMale;

    // Mesh Data
    for (const auto& Elem : SaveFile.MeshAssetRowNameData)
    {
        FShibMeshData MeshData;
        MeshData.MeshType = Elem.Key;      // Assuming EShibMesh is the type of the key in the TMap
        MeshData.RowName = Elem.Value;     // FName for the row name in the TMap
        NewAvatarData.MeshAssetData.Add(MeshData);
    }

    // Texture Data
    for (const auto& Elem : SaveFile.ShibTextureAssetRowNameData)
    {
        FShibTextureData TextureData;
        TextureData.TextureType = Elem.Key; // Assuming EShibTexture is the type of the key in the TMap
        TextureData.RowName = Elem.Value;   // FName for the row name in the TMap
        NewAvatarData.TextureAssetData.Add(TextureData);
    }

    // Accessory Data
    for (const auto& Elem : SaveFile.AccessoryRowNameData)
    {
        FShibAccessoryData NewAccessoryData;
        NewAccessoryData.AccessoryType = Elem.Key;  // Assuming EShibAccessory is the type of the key
        NewAccessoryData.RowName = Elem.Value;      // FName for the row name
        NewAvatarData.AccessoryData.Add(NewAccessoryData);
    }

    // Tattoo Data
    for (const auto& Elem : SaveFile.TattooRowNameData)
    {
        FShibTattooData NewTattooData;
        NewTattooData.TattooType = Elem.Key;  // Assuming EShibTattoo is the type of the key
        NewTattooData.RowName = Elem.Value;   // FName for the row name
        NewAvatarData.TattooData.Add(NewTattooData);
    }

    // Morph Data
    for (const auto& Elem : SaveFile.MorphData)
    {
        FShibMorphData NewMorphData;
        NewMorphData.MorphType = Elem.Key; // Assuming EShibBodyMorphs is the type of the key
        NewMorphData.Value = Elem.Value;   // double for the morph value
        NewAvatarData.MorphData.Add(NewMorphData);
    }

    // Mesh Colors
    for (const auto& Elem : SaveFile.ShibMeshColorData)
    {
        FShibMeshColorData NewMeshColorData;
        NewMeshColorData.MeshType = Elem.Key; // Assuming EShibMesh is the type of the key
        NewMeshColorData.Color = Elem.Value;  // FLinearColor for the color
        NewAvatarData.MeshColorData.Add(NewMeshColorData);
    }

    // Texture Colors
    for (const auto& Elem : SaveFile.ShibTextureColorData)
    {
        FShibTextureColorData NewTextureColorData;
        NewTextureColorData.TextureType = Elem.Key; // Assuming EShibTexture is the type of the key
        NewTextureColorData.Color = Elem.Value;     // FLinearColor for the color
        NewAvatarData.TextureColorData.Add(NewTextureColorData);
    }

    // Accessory Colors
    for (const auto& Elem : SaveFile.AccessoryShibColorData)
    {
        FShibAccessoryColorData NewAccessoryColorData;
        NewAccessoryColorData.AccessoryType = Elem.Key; // Assuming EShibAccessory is the type of the key
        NewAccessoryColorData.Color = Elem.Value;        // FLinearColor for the color
        NewAvatarData.AccessoryColorData.Add(NewAccessoryColorData);
    }

    // Tattoo Colors
    for (const auto& Elem : SaveFile.TattooShibColorData)
    {
        FShibTattooColorData NewTattooColorData;
        NewTattooColorData.TattooType = Elem.Key;  // Assuming EShibTattoo is the type of the key
        NewTattooColorData.Color = Elem.Value;     // FLinearColor for the color
        NewAvatarData.TattooColorData.Add(NewTattooColorData);
    }
}

void FAvatarDataRPC::CopyAvatarDataToSaveFile(const FAvatarDataRPC& AvatarData, FAvatarSave& SaveFile)
{
    SaveFile.AvatarBaseRow = AvatarData.AvatarBaseRow;
    SaveFile.bIsMale = AvatarData.bIsMale;

    // Mesh Data
    for (const auto& Elem : AvatarData.MeshAssetData)
    {
        SaveFile.MeshAssetRowNameData.Add(Elem.MeshType, Elem.RowName);
    }

    // Texture Data
    for (const auto& Elem : AvatarData.TextureAssetData)
    {
        SaveFile.ShibTextureAssetRowNameData.Add(Elem.TextureType, Elem.RowName);
    }

    // Accessory Data
    for (const auto& Elem : AvatarData.AccessoryData)
    {
        SaveFile.AccessoryRowNameData.Add(Elem.AccessoryType, Elem.RowName);
    }

    // Tattoo Data
    for (const auto& Elem : AvatarData.TattooData)
    {
        SaveFile.TattooRowNameData.Add(Elem.TattooType, Elem.RowName);
    }

    // Morph Data
    for (const auto& Elem : AvatarData.MorphData)
    {
        SaveFile.MorphData.Add(Elem.MorphType, Elem.Value);
    }

    // Mesh Colors
    for (const auto& Elem : AvatarData.MeshColorData)
    {
        SaveFile.ShibMeshColorData.Add(Elem.MeshType, Elem.Color);
    }

    // Texture Colors
    for (const auto& Elem : AvatarData.TextureColorData)
    {
        SaveFile.ShibTextureColorData.Add(Elem.TextureType, Elem.Color);
    }

    // Accessory Colors
    for (const auto& Elem : AvatarData.AccessoryColorData)
    {
        SaveFile.AccessoryShibColorData.Add(Elem.AccessoryType, Elem.Color);
    }

    // Tattoo Colors
    for (const auto& Elem : AvatarData.TattooColorData)
    {
        SaveFile.TattooShibColorData.Add(Elem.TattooType, Elem.Color);
    }
}
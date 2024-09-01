// Copyright Dylan Dumesnil. All Rights Reserved.

#include "MDStyleSetFunctionLibrary.h"

#include "AssetRegistry/AssetData.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "Blueprint/BlueprintExceptionInfo.h"
#include "MDStyleSet.h"
#include "Util/MDStyleSetTypes.h"

#define LOCTEXT_NAMESPACE "MDGameDataBlueprintFunctionLibrary"

void UMDStyleSetFunctionLibrary::GetStyleValue(UMDStyleSet* StyleSet, const FGameplayTag& StyleTag, int32& OutValue)
{
	checkNoEntry();
}

DEFINE_FUNCTION(UMDStyleSetFunctionLibrary::execGetStyleValue)
{
	P_GET_OBJECT(UMDStyleSet, StyleSet);
	P_GET_STRUCT_REF(FGameplayTag, StyleTag);

	Stack.StepCompiledIn<FProperty>(nullptr);
	const FProperty* ValueProp = Stack.MostRecentProperty;
	void* ValuePtr = Stack.MostRecentPropertyAddress;

	P_FINISH;

	if (StyleSet == nullptr)
	{
		const FBlueprintExceptionInfo ExceptionInfo(
			EBlueprintExceptionType::AccessViolation,
			LOCTEXT("GetValue_MissingStyleSet", "A valid Style Set is required.")
		);
		FBlueprintCoreDelegates::ThrowScriptException(P_THIS, Stack, ExceptionInfo);
		return;
	}

	if (ValueProp == nullptr || ValuePtr == nullptr)
	{
		const FBlueprintExceptionInfo ExceptionInfo(
			EBlueprintExceptionType::AccessViolation,
			LOCTEXT("GetValue_MissingOutputProperty", "Failed to resolve the output parameter for GetStyleValue.")
		);
		FBlueprintCoreDelegates::ThrowScriptException(P_THIS, Stack, ExceptionInfo);
		return;
	}

	bool bResult = false;
	P_NATIVE_BEGIN
	bResult = StyleSet->TrySetPropertyValue(StyleTag, ValueProp, ValuePtr);
	P_NATIVE_END

	if (!bResult)
	{
		const FBlueprintExceptionInfo ExceptionInfo(
			EBlueprintExceptionType::AccessViolation,
			FText::Format(LOCTEXT("GetValue_IncompatibleTypes", "Failed to get Style Value ({0}), it may not exist or it's not compatible with type ({1}).")
				, StyleSet->GetDisplayName()
				, ValueProp->GetClass()->GetDisplayNameText())
		);
		FBlueprintCoreDelegates::ThrowScriptException(P_THIS, Stack, ExceptionInfo);
	}
}

TTuple<FPropertyBagPropertyDesc, const uint8*> UMDStyleSetFunctionLibrary::GetStyleSetValue(const FMDStyleSetValueReference& ValueReference)
{
	if (UMDStyleSet* StyleSet = ValueReference.StyleSet)
	{
		return StyleSet->GetStyleValue(ValueReference.StyleValueTag);
	}

	return {};
}

bool UMDStyleSetFunctionLibrary::HasConvertibleStyleSets(const FPropertyBagPropertyDesc& Desc)
{
	IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();

	TArray<FAssetData> Assets;
	if (AssetRegistry.GetAssetsByClass(UMDStyleSet::StaticClass()->GetClassPathName(), Assets, true))
	{
		for (const FAssetData& Asset : Assets)
		{
			TArray<FPropertyBagPropertyDesc> ConvertibleTypes = GetConvertibleTypes(Asset);
			for (const FPropertyBagPropertyDesc& ConvertibleType : ConvertibleTypes)
			{
				if (ConvertibleType.CompatibleType(Desc))
				{
					return true;
				}
			}
		}
	}

	return false;
}

TArray<UMDStyleSet*> UMDStyleSetFunctionLibrary::GetConvertibleStyleSets(const FPropertyBagPropertyDesc& Desc)
{
	TArray<UMDStyleSet*> Result;

	IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();

	TArray<FAssetData> Assets;
	if (AssetRegistry.GetAssetsByClass(UMDStyleSet::StaticClass()->GetClassPathName(), Assets, true))
	{
		for (const FAssetData& Asset : Assets)
		{
			TArray<FPropertyBagPropertyDesc> ConvertibleTypes = GetConvertibleTypes(Asset);
			for (const FPropertyBagPropertyDesc& ConvertibleType : ConvertibleTypes)
			{
				if (ConvertibleType.CompatibleType(Desc))
				{
					if (FPackageName::GetPackageMountPoint(Asset.PackageName.ToString()) != NAME_None)
					{
						if (UMDStyleSet* StyleSet = Cast<UMDStyleSet>(Asset.GetAsset()))
						{
							Result.Add(StyleSet);
							break;
						}
					}
				}
			}
		}
	}

	return Result;
}

TArray<FPropertyBagPropertyDesc> UMDStyleSetFunctionLibrary::GetConvertibleTypes(const FAssetData& Asset)
{
	TArray<FPropertyBagPropertyDesc> Result;

	FString ConvertibleTypesString;
	if (Asset.GetTagValue(UMDStyleSet::ConvertibleTypesAssetTagName, ConvertibleTypesString))
	{
		TArray<FString> ConvertibleTypeStrings;
		ConvertibleTypesString.ParseIntoArrayLines(ConvertibleTypeStrings);

		for (const FString& ConvertibleTypeString : ConvertibleTypeStrings)
		{
			FPropertyBagPropertyDesc ConvertibleTypeDesc;
			FPropertyBagPropertyDesc::StaticStruct()->ImportText(*ConvertibleTypeString, &ConvertibleTypeDesc, nullptr, PPF_None, nullptr, FPropertyBagPropertyDesc::StaticStruct()->GetName());
			Result.Add(MoveTemp(ConvertibleTypeDesc));
		}
	}

	return Result;
}

#undef LOCTEXT_NAMESPACE

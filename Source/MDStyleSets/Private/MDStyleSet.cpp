// Copyright Dylan Dumesnil. All Rights Reserved.

#include "MDStyleSet.h"

#include "AssetRegistry/AssetData.h"
#include "TypeHandlers/MDStyleSetTypeHandlerBase.h"
#include "UObject/AssetRegistryTagsContext.h"

const FName FMDStyleValue::ValuePropertyName = TEXT("Value");

TTuple<FPropertyBagPropertyDesc, const uint8*> FMDStyleValue::GetValue() const
{
	if (const FPropertyBagPropertyDesc* Desc = Value.FindPropertyDescByName(ValuePropertyName))
	{
		struct LazyHack : FInstancedPropertyBag
		{
			const uint8* GetValueAddressFromDesc(const FPropertyBagPropertyDesc* Desc) const
			{
				return static_cast<const uint8*>(GetValueAddress(Desc));
			}
		};

		const uint8* ValuePtr = static_cast<const LazyHack&>(Value).GetValueAddressFromDesc(Desc);
		return { *Desc, ValuePtr };
	}

	return {};
}

const FName UMDStyleSet::ConvertibleTypesAssetTagName = TEXT("ConvertibleTypesAssetTag");

#if WITH_EDITOR
EPropertyBagPropertyType UMDStyleSet::GetValueTypeFromPinType(const FEdGraphPinType& PinType)
{
	if (PinType.PinCategory == TEXT("bool"))
	{
		return EPropertyBagPropertyType::Bool;
	}
	if (PinType.PinCategory == TEXT("byte"))
	{
		return EPropertyBagPropertyType::Byte;
	}
	if (PinType.PinCategory == TEXT("enum"))
	{
		return EPropertyBagPropertyType::Enum;
	}
	if (PinType.PinCategory == TEXT("int"))
	{
		return EPropertyBagPropertyType::Int32;
	}
	if (PinType.PinCategory == TEXT("int64"))
	{
		return EPropertyBagPropertyType::Int64;
	}
	if (PinType.PinCategory == TEXT("float"))
	{
		return EPropertyBagPropertyType::Float;
	}
	if (PinType.PinCategory == TEXT("real") && PinType.PinSubCategory == TEXT("float"))
	{
		return EPropertyBagPropertyType::Float;
	}
	if (PinType.PinCategory == TEXT("double"))
	{
		return EPropertyBagPropertyType::Double;
	}
	if (PinType.PinCategory == TEXT("real") && PinType.PinSubCategory == TEXT("double"))
	{
		return EPropertyBagPropertyType::Double;
	}
	if (PinType.PinCategory == TEXT("name"))
	{
		return EPropertyBagPropertyType::Name;
	}
	if (PinType.PinCategory == TEXT("string"))
	{
		return EPropertyBagPropertyType::String;
	}
	if (PinType.PinCategory == TEXT("text"))
	{
		return EPropertyBagPropertyType::Text;
	}
	if (PinType.PinCategory == TEXT("struct"))
	{
		return EPropertyBagPropertyType::Struct;
	}
	if (PinType.PinCategory == TEXT("class"))
	{
		return EPropertyBagPropertyType::Class;
	}
	if (PinType.PinCategory == TEXT("object"))
	{
		return EPropertyBagPropertyType::Object;
	}
	if (PinType.PinCategory == TEXT("softclass"))
	{
		return EPropertyBagPropertyType::SoftClass;
	}
	if (PinType.PinCategory == TEXT("softobject"))
	{
		return EPropertyBagPropertyType::SoftObject;
	}

	return EPropertyBagPropertyType::None;
}

void UMDStyleSet::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(UMDStyleSet, StyleEntries)
		&& PropertyChangedEvent.ChangeType == EPropertyChangeType::ArrayAdd)
	{
		const EPropertyBagContainerType ContainerType = StyleType.IsArray() ? EPropertyBagContainerType::Array : EPropertyBagContainerType::None;
		const EPropertyBagPropertyType ValueType = UMDStyleSet::GetValueTypeFromPinType(StyleType);
		UObject* TypeObject = StyleType.PinSubCategoryObject.Get();
		if (ValueType != EPropertyBagPropertyType::None)
		{
			for (TPair<FGameplayTag, FMDStyleValue>& Pair : StyleEntries)
			{
				if (!Pair.Value.Value.IsValid())
				{
					Pair.Value.Value.AddContainerProperty(FMDStyleValue::ValuePropertyName, ContainerType, ValueType, TypeObject);
				}
			}
		}
	}
}

EDataValidationResult UMDStyleSet::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = EDataValidationResult::Valid;

	return CombineDataValidationResults(Result, Super::IsDataValid(Context));
}
#endif

void UMDStyleSet::GetAssetRegistryTags(FAssetRegistryTagsContext Context) const
{
	Super::GetAssetRegistryTags(Context);

	static const FPropertyBagPropertyDesc DefaultDesc;

	TArray<FPropertyBagPropertyDesc> ConvertibleTypes;
	if (IsValid(TypeHandler))
	{
		TypeHandler->GetConvertibleTypes(ConvertibleTypes);
	}

	if (const FPropertyBagPropertyDesc* Desc = FallbackValue.Value.FindPropertyDescByName(FMDStyleValue::ValuePropertyName))
	{
		ConvertibleTypes.Add(*Desc);
	}

	FStringBuilderBase ConvertibleTypesStringBuilder;

	for (const FPropertyBagPropertyDesc& Type : ConvertibleTypes)
	{
		FString DescString;
		FPropertyBagPropertyDesc::StaticStruct()->ExportText(DescString, &Type, &DefaultDesc, nullptr, PPF_None, nullptr);

		if (ConvertibleTypesStringBuilder.Len() == 0)
		{
			ConvertibleTypesStringBuilder = DescString;
		}
		else
		{
			ConvertibleTypesStringBuilder.Appendf(TEXT("\n%s"), *DescString);
		}
	}

	FAssetRegistryTag StyleTypeTag = {
		ConvertibleTypesAssetTagName,
		ConvertibleTypesStringBuilder.ToString(),
		FAssetRegistryTag::TT_Hidden
	};
	Context.AddTag(MoveTemp(StyleTypeTag));
}

TTuple<FPropertyBagPropertyDesc, const uint8*> UMDStyleSet::GetStyleValue(const FGameplayTag& ValueTag) const
{
	if (const FMDStyleValue* ValuePtr = StyleEntries.Find(ValueTag))
	{
		return ValuePtr->GetValue();
	}

	return FallbackValue.GetValue();
}

bool UMDStyleSet::TrySetPropertyValue(const FGameplayTag& ValueTag, const FProperty* DestProp, void* DestPtr) const
{
	if (DestProp != nullptr && DestPtr != nullptr)
	{
		TTuple<FPropertyBagPropertyDesc, const uint8*> Value = GetStyleValue(ValueTag);
		if (Value.Value != nullptr)
		{
			const FPropertyBagPropertyDesc DestDesc = { FMDStyleValue::ValuePropertyName, DestProp };
			if (DestDesc.CompatibleType(Value.Key))
			{
				DestProp->CopyCompleteValue(DestPtr, Value.Value);
				return true;
			}

			if (IsValid(TypeHandler))
			{
				return TypeHandler->TrySetValue(Value, DestDesc, DestPtr);
			}
		}
	}

	return false;
}

FText UMDStyleSet::GetValueDisplayName(const FGameplayTag& ValueTag) const
{
	if (IsValid(TypeHandler))
	{
		return TypeHandler->GetValueAsText(this, ValueTag);
	}

	TTuple<FPropertyBagPropertyDesc, const uint8*> Value = GetStyleValue(ValueTag);
	if (Value.Key.CachedProperty != nullptr && Value.Value != nullptr)
	{
		FString ValueString;
		if (Value.Key.CachedProperty->ExportText_Direct(ValueString, Value.Value, Value.Value, nullptr, PPF_None))
		{
			return FText::FromString(ValueString);
		}
	}

	return INVTEXT("Invalid Value");
}

bool UMDStyleSet::DoesHaveValueWithTag(const FGameplayTag& ValueTag) const
{
	return StyleEntries.Contains(ValueTag);
}

void UMDStyleSet::SortEntries()
{
#if WITH_EDITOR
	PreEditChange(GetClass()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(UMDStyleSet, StyleEntries)));

	StyleEntries.KeySort([](const FGameplayTag& A, const FGameplayTag& B)
	{
		return !B.IsValid() || (A.IsValid() && A.ToString() < B.ToString());
	});
#endif
}

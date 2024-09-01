// Copyright Dylan Dumesnil. All Rights Reserved.


#include "TypeHandlers/MDStyleSetTypeHandler_Color.h"

#include "MDStyleSet.h"
#include "Styling/SlateColor.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/SNullWidget.h"

namespace MDSSTHC
{
	FSlateColor GetValueAsSlateColor(const UMDStyleSet* StyleSet, const FGameplayTag& StyleTag)
	{
		if (!IsValid(StyleSet))
		{
			return {};
		}

		TTuple<FPropertyBagPropertyDesc, const uint8*> StyleValue = StyleSet->GetStyleValue(StyleTag);
		if (StyleValue.Value == nullptr)
		{
			return {};
		}

		if (StyleValue.Key.ValueTypeObject == TBaseStructure<FLinearColor>::Get())
		{
			return *reinterpret_cast<const FLinearColor*>(StyleValue.Value);
		}
		else if (StyleValue.Key.ValueTypeObject == TBaseStructure<FColor>::Get())
		{
			return *reinterpret_cast<const FColor*>(StyleValue.Value);
		}
		else if (StyleValue.Key.ValueTypeObject == TBaseStructure<FSlateColor>::Get())
		{
			return *reinterpret_cast<const FSlateColor*>(StyleValue.Value);
		}

		return {};
	}
}

void UMDStyleSetTypeHandler_Color::GetConvertibleTypes(TArray<FPropertyBagPropertyDesc>& OutConvertibleTypes) const
{
	Super::GetConvertibleTypes(OutConvertibleTypes);

	OutConvertibleTypes.Add(FPropertyBagPropertyDesc(FMDStyleValue::ValuePropertyName, EPropertyBagPropertyType::Struct, TBaseStructure<FLinearColor>::Get()));
	OutConvertibleTypes.Add(FPropertyBagPropertyDesc(FMDStyleValue::ValuePropertyName, EPropertyBagPropertyType::Struct, TBaseStructure<FColor>::Get()));
	OutConvertibleTypes.Add(FPropertyBagPropertyDesc(FMDStyleValue::ValuePropertyName, EPropertyBagPropertyType::Struct, TBaseStructure<FSlateColor>::Get()));
}

bool UMDStyleSetTypeHandler_Color::TrySetValue(const TTuple<FPropertyBagPropertyDesc, const uint8*>& Value, const FPropertyBagPropertyDesc& DestDesc, void* DescPtr) const
{
	static const TSet<const UObject*> ColorStructs = { TBaseStructure<FLinearColor>::Get(), TBaseStructure<FColor>::Get(), TBaseStructure<FSlateColor>::Get() };
	if (Value.Value == nullptr || Value.Key.ValueType != EPropertyBagPropertyType::Struct || !ColorStructs.Contains(Value.Key.ValueTypeObject))
	{
		return false;
	}

	TOptional<FLinearColor> SourceColor;
	if (Value.Key.ValueTypeObject == TBaseStructure<FLinearColor>::Get())
	{
		const FLinearColor* Color = reinterpret_cast<const FLinearColor*>(Value.Value);
		SourceColor = *Color;
	}
	else if (Value.Key.ValueTypeObject == TBaseStructure<FColor>::Get())
	{
		const FColor* Color = reinterpret_cast<const FColor*>(Value.Value);
		SourceColor = *Color;
	}
	else if (Value.Key.ValueTypeObject == TBaseStructure<FSlateColor>::Get())
	{
		const FSlateColor* Color = reinterpret_cast<const FSlateColor*>(Value.Value);
		if (Color->IsColorSpecified())
		{
			SourceColor = Color->GetSpecifiedColor();
			return true;
		}
	}

	if (SourceColor.IsSet())
	{
		if (DestDesc.ValueTypeObject == TBaseStructure<FLinearColor>::Get())
		{
			FLinearColor* DestColor = static_cast<FLinearColor*>(DescPtr);
			*DestColor = SourceColor.GetValue();
		}
		else if (DestDesc.ValueTypeObject == TBaseStructure<FColor>::Get())
		{
			FColor* DestColor = static_cast<FColor*>(DescPtr);
			*DestColor = SourceColor.GetValue().ToFColorSRGB();
		}
		else if (DestDesc.ValueTypeObject == TBaseStructure<FSlateColor>::Get())
		{
			FSlateColor* DestColor = static_cast<FSlateColor*>(DescPtr);
			*DestColor = SourceColor.GetValue();
		}
	}

	return false;
}

TSharedRef<SWidget> UMDStyleSetTypeHandler_Color::CreateValuePreviewWidget(UMDStyleSet* StyleSet, const FGameplayTag& StyleTag) const
{
	TSharedRef<SWidget> Result = Super::CreateValuePreviewWidget(StyleSet, StyleTag);

	if (Result == SNullWidget::NullWidget)
	{
		Result = SNew(SImage)
			.Image(FAppStyle::GetBrush("WhiteBrush"))
			.DesiredSizeOverride(FVector2D(32.f))
			.ColorAndOpacity(MDSSTHC::GetValueAsSlateColor(StyleSet, StyleTag));
	}

	return Result;
}

FText UMDStyleSetTypeHandler_Color::GetValueAsText_Implementation(const UMDStyleSet* StyleSet, const FGameplayTag& StyleTag) const
{
	if (IsValid(StyleSet))
	{
		TTuple<FPropertyBagPropertyDesc, const uint8*> StyleValue = StyleSet->GetStyleValue(StyleTag);
		if (StyleValue.Value != nullptr)
		{
			if (StyleValue.Key.ValueTypeObject == TBaseStructure<FLinearColor>::Get())
			{
				const FLinearColor* Color = reinterpret_cast<const FLinearColor*>(StyleValue.Value);
				return FText::FromString(Color->ToString());
			}
			else if (StyleValue.Key.ValueTypeObject == TBaseStructure<FColor>::Get())
			{
				const FColor* Color = reinterpret_cast<const FColor*>(StyleValue.Value);
				return FText::FromString(Color->ToString());
			}
			else if (StyleValue.Key.ValueTypeObject == TBaseStructure<FSlateColor>::Get())
			{
				const FSlateColor* Color = reinterpret_cast<const FSlateColor*>(StyleValue.Value);
				if (Color->IsColorSpecified())
				{
					return FText::FromString(Color->GetSpecifiedColor().ToString());
				}

				struct FSlateColorHack : public FSlateColor
				{
					ESlateColorStylingMode GetStylingMode() const { return ColorUseRule; }
				};

				return UEnum::GetDisplayValueAsText(static_cast<const FSlateColorHack*>(Color)->GetStylingMode());
			}
		}
	}

	return Super::GetValueAsText_Implementation(StyleSet, StyleTag);
}

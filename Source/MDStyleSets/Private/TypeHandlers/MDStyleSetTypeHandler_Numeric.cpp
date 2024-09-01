// Copyright Dylan Dumesnil. All Rights Reserved.


#include "TypeHandlers/MDStyleSetTypeHandler_Numeric.h"

#include "MDStyleSet.h"

void UMDStyleSetTypeHandler_Numeric::GetConvertibleTypes(TArray<FPropertyBagPropertyDesc>& OutConvertibleTypes) const
{
	Super::GetConvertibleTypes(OutConvertibleTypes);

	OutConvertibleTypes.Add(FPropertyBagPropertyDesc(FMDStyleValue::ValuePropertyName, EPropertyBagPropertyType::Double));
	OutConvertibleTypes.Add(FPropertyBagPropertyDesc(FMDStyleValue::ValuePropertyName, EPropertyBagPropertyType::Float));
	OutConvertibleTypes.Add(FPropertyBagPropertyDesc(FMDStyleValue::ValuePropertyName, EPropertyBagPropertyType::Int32));
	OutConvertibleTypes.Add(FPropertyBagPropertyDesc(FMDStyleValue::ValuePropertyName, EPropertyBagPropertyType::Int64));
	OutConvertibleTypes.Add(FPropertyBagPropertyDesc(FMDStyleValue::ValuePropertyName, EPropertyBagPropertyType::UInt32));
	OutConvertibleTypes.Add(FPropertyBagPropertyDesc(FMDStyleValue::ValuePropertyName, EPropertyBagPropertyType::UInt64));
}

bool UMDStyleSetTypeHandler_Numeric::TrySetValue(const TTuple<FPropertyBagPropertyDesc, const uint8*>& Value, const FPropertyBagPropertyDesc& DestDesc, void* DestPtr) const
{
	static const TSet<EPropertyBagPropertyType> NumericTypes = {
		EPropertyBagPropertyType::Double,
		EPropertyBagPropertyType::Float,
		EPropertyBagPropertyType::Int32,
		EPropertyBagPropertyType::Int64,
		EPropertyBagPropertyType::UInt32,
		EPropertyBagPropertyType::UInt64
	};

	if (Value.Value == nullptr || !NumericTypes.Contains(Value.Key.ValueType))
	{
		return false;
	}

	TOptional<double> SourceDouble;
	TOptional<int64> SourceInt;
	TOptional<uint64> SourceUInt;
	if (Value.Key.ValueType == EPropertyBagPropertyType::Double)
	{
		const double* Number = reinterpret_cast<const double*>(Value.Value);
		SourceDouble = *Number;
	}
	else if (Value.Key.ValueType == EPropertyBagPropertyType::Float)
	{
		const float* Number = reinterpret_cast<const float*>(Value.Value);
		SourceDouble = *Number;
	}
	else if (Value.Key.ValueType == EPropertyBagPropertyType::Int32)
	{
		const int32* Number = reinterpret_cast<const int32*>(Value.Value);
		SourceInt = *Number;
	}
	else if (Value.Key.ValueType == EPropertyBagPropertyType::Int64)
	{
		const int64* Number = reinterpret_cast<const int64*>(Value.Value);
		SourceInt = *Number;
	}
	else if (Value.Key.ValueType == EPropertyBagPropertyType::UInt32)
	{
		const uint32* Number = reinterpret_cast<const uint32*>(Value.Value);
		SourceUInt = *Number;
	}
	else if (Value.Key.ValueType == EPropertyBagPropertyType::UInt64)
	{
		const uint64* Number = reinterpret_cast<const uint64*>(Value.Value);
		SourceUInt = *Number;
	}

	auto CopyNumber = [&DestDesc, DestPtr](auto SourceValue)
	{
		if (DestDesc.ValueType == EPropertyBagPropertyType::Double)
		{
			double* Number = static_cast<double*>(DestPtr);
			*Number = SourceValue;
			return true;
		}
		else if (DestDesc.ValueType == EPropertyBagPropertyType::Float)
		{
			float* Number = static_cast<float*>(DestPtr);
			*Number = SourceValue;
			return true;
		}
		else if (DestDesc.ValueType == EPropertyBagPropertyType::Int32)
		{
			int32* Number = static_cast<int32*>(DestPtr);
			*Number = SourceValue;
			return true;
		}
		else if (DestDesc.ValueType == EPropertyBagPropertyType::Int64)
		{
			int64* Number = static_cast<int64*>(DestPtr);
			*Number = SourceValue;
			return true;
		}
		else if (DestDesc.ValueType == EPropertyBagPropertyType::UInt32)
		{
			uint32* Number = static_cast<uint32*>(DestPtr);
			*Number = SourceValue;
			return true;
		}
		else if (DestDesc.ValueType == EPropertyBagPropertyType::UInt64)
		{
			uint64* Number = static_cast<uint64*>(DestPtr);
			*Number = SourceValue;
			return true;
		}

		return false;
	};

	if (SourceDouble.IsSet())
	{
		return CopyNumber(SourceDouble.GetValue());
	}
	else if (SourceInt.IsSet())
	{
		return CopyNumber(SourceInt.GetValue());
	}
	else if (SourceUInt.IsSet())
	{
		return CopyNumber(SourceUInt.GetValue());
	}

	return false;
}

FText UMDStyleSetTypeHandler_Numeric::CreateValuePreviewText_Implementation(UMDStyleSet* StyleSet, const FGameplayTag& StyleTag) const
{
	return GetValueAsText(StyleSet, StyleTag);
}

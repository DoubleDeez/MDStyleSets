// Copyright Dylan Dumesnil. All Rights Reserved.

#pragma once

#include "IDetailCustomization.h"
#include "EdGraph/EdGraphPin.h"

class IPropertyHandle;

/**
 *
 */
class MDSTYLESETSEDITOR_API FMDStyleSetDetailCustomization : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance() { return MakeShared<FMDStyleSetDetailCustomization>(); }

	virtual void CustomizeDetails(const TSharedPtr<IDetailLayoutBuilder>& DetailBuilder) override;
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

private:
	FEdGraphPinType GetPinType() const;
	void OnPinTypeChanged(const FEdGraphPinType& PinType);
	void OnPinTypePropertyChanged();

	void OnGetCategoriesMetaFromPropertyHandle(TSharedPtr<IPropertyHandle> PropertyHandle, FString& MetaString) const;
	void RefreshDetails() const;

	TWeakPtr<IDetailLayoutBuilder> DetailBuilderPtr;
	TSharedPtr<IPropertyHandle> StyleTypePropertyPtr;
	TSharedPtr<IPropertyHandle> StyleSetTagPropertyPtr;
	TSharedPtr<IPropertyHandle> FallbackValuePropertyPtr;
	TSharedPtr<IPropertyHandle> StyleEntriesPropertyPtr;
};

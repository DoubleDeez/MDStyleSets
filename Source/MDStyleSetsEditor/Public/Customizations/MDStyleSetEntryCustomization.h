// Copyright Dylan Dumesnil. All Rights Reserved.

#pragma once

#include "IPropertyTypeCustomization.h"

class IDetailLayoutBuilder;

class MDSTYLESETSEDITOR_API FMDStyleSetEntryCustomization : public IPropertyTypeCustomization
{
public:
	FMDStyleSetEntryCustomization(const TWeakPtr<IDetailLayoutBuilder>& DetailBuilderPtr)
		: DetailBuilderPtr(DetailBuilderPtr)
	{}

	static TSharedRef<IPropertyTypeCustomization> MakeInstance(TWeakPtr<IDetailLayoutBuilder> DetailBuilderPtr) { return MakeShared<FMDStyleSetEntryCustomization>(DetailBuilderPtr); }

	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils) override;

private:
	bool IsResetToDefaultVisible(TSharedPtr<IPropertyHandle> Handle) const;
	void ResetValueToDefault(TSharedPtr<IPropertyHandle> Handle);

	TWeakPtr<IDetailLayoutBuilder> DetailBuilderPtr;
	TSharedPtr<IPropertyHandle> EntryValueHandle;
};

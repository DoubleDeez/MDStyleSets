// Copyright Dylan Dumesnil. All Rights Reserved.

#include "MDStyleSetsEditor.h"

#include "Customizations/MDStyleSetDetailCustomization.h"
#include "Customizations/MDStyleSetsPropertyBindingExtension.h"
#include "EdGraphUtilities.h"
#include "MDStyleSet.h"
#include "Nodes/MDStyleSetNode_GetStyleValue.h"
#include "PropertyEditorModule.h"
#include "UMGEditorModule.h"
#include "Widgets/SMDStyleSetGraphNode.h"

#define LOCTEXT_NAMESPACE "FMDStyleSetsEditorModule"

class FMDStyleSetNodeFactory : public FGraphPanelNodeFactory
{
	virtual TSharedPtr<SGraphNode> CreateNode(UEdGraphNode* InNode) const override
	{
		if (auto* StyleNode = Cast<UMDStyleSetNode_GetStyleValue>(InNode))
		{
			return SNew(SMDStyleSetGraphNode, StyleNode);
		}

		return nullptr;
	}
};

void FMDStyleSetsEditorModule::StartupModule()
{
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.RegisterCustomClassLayout(UMDStyleSet::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FMDStyleSetDetailCustomization::MakeInstance));
	PropertyModule.NotifyCustomizationModuleChanged();

	IUMGEditorModule& UMGEditorInterface = FModuleManager::GetModuleChecked<IUMGEditorModule>("UMGEditor");

	PropertyBinding = MakeShared<FMDStyleSetsPropertyBindingExtension>();
	UMGEditorInterface.GetPropertyBindingExtensibilityManager()->AddExtension(PropertyBinding.ToSharedRef());

	StyleSetNodeFactory = MakeShared<FMDStyleSetNodeFactory>();
	FEdGraphUtilities::RegisterVisualNodeFactory(StyleSetNodeFactory);
}

void FMDStyleSetsEditorModule::ShutdownModule()
{
	if (IUMGEditorModule* UMGEditorInterface = FModuleManager::GetModulePtr<IUMGEditorModule>("UMGEditor"))
	{
		UMGEditorInterface->GetPropertyBindingExtensibilityManager()->RemoveExtension(PropertyBinding.ToSharedRef());
	}

	PropertyBinding.Reset();

	if (StyleSetNodeFactory.IsValid())
	{
		FEdGraphUtilities::UnregisterVisualNodeFactory(StyleSetNodeFactory);
		StyleSetNodeFactory.Reset();
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FMDStyleSetsEditorModule, MDStyleSetsEditor)
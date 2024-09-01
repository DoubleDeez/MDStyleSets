// Copyright Dylan Dumesnil. All Rights Reserved.


#include "Nodes/MDStyleSetNode_GetStyleValue.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "BlueprintActionDatabaseRegistrar.h"
#include "BlueprintNodeSpawner.h"
#include "EdGraphSchema_K2.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "MDStyleSetFunctionLibrary.h"
#include "MDStyleSet.h"

#if WITH_EDITOR
#include "Editor.h"
#include "Misc/DataValidation.h"
#include "Subsystems/AssetEditorSubsystem.h"
#endif

UMDStyleSetNode_GetStyleValue::UMDStyleSetNode_GetStyleValue()
{
	bIsPureFunc = true;
	FunctionReference.SetExternalMember(GET_FUNCTION_NAME_CHECKED(UMDStyleSetFunctionLibrary, GetStyleValue), UMDStyleSetFunctionLibrary::StaticClass());

#if WITH_EDITOR
	if (!IsTemplate())
	{
		FCoreUObjectDelegates::OnObjectPropertyChanged.AddUObject(this, &UMDStyleSetNode_GetStyleValue::OnPropertyValueChanged);
	}
#endif
}

void UMDStyleSetNode_GetStyleValue::BeginDestroy()
{
#if WITH_EDITOR
	if (!IsTemplate())
	{
		FCoreUObjectDelegates::OnObjectPropertyChanged.RemoveAll(this);
	}
#endif

	Super::BeginDestroy();
}

FLinearColor UMDStyleSetNode_GetStyleValue::GetNodeTitleColor() const
{
	if (IsValid(BoundStyleSet))
	{
		return GetDefault<UEdGraphSchema_K2>()->GetPinTypeColor(BoundStyleSet->StyleType);
	}

	return Super::GetNodeTitleColor();
}

FText UMDStyleSetNode_GetStyleValue::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	FText StyleName;
	if (IsValid(BoundStyleSet))
	{
		StyleName = BoundStyleSet->GetDisplayName();
	}

	if (!StyleName.IsEmptyOrWhitespace())
	{
		if (TitleType == ENodeTitleType::MenuTitle)
		{
			return FText::Format(INVTEXT("Get '{0}' Style Value"), StyleName);
		}

		return StyleName;
	}

	return INVTEXT("Invalid Style");
}

FText UMDStyleSetNode_GetStyleValue::GetPinDisplayName(const UEdGraphPin* Pin) const
{
	if (IsValid(BoundStyleSet) && Pin != nullptr && Pin->GetFName() == TEXT("StyleTag"))
	{
		return FText::Format(INVTEXT("'{0}' Style"), BoundStyleSet->GetDisplayName());
	}

	if (IsValid(BoundStyleSet) && Pin != nullptr && Pin->GetFName() == TEXT("OutValue"))
	{
		return INVTEXT("Value");
	}

	return FText::GetEmpty();
}

FText UMDStyleSetNode_GetStyleValue::GetTooltipText() const
{
	const FText BaseToolTip = Super::GetTooltipText();
	const FText StyleName = IsValid(BoundStyleSet)
		? BoundStyleSet->GetDisplayName()
		: INVTEXT("[INVALID]");
	return FText::Format(INVTEXT("Style Set: {0} - {1}\r{2}"), StyleName, FText::FromName(GetFNameSafe(BoundStyleSet)), BaseToolTip);
}

FString UMDStyleSetNode_GetStyleValue::GetPinMetaData(FName InPinName, FName InKey)
{
	if (IsValid(BoundStyleSet) && InPinName == TEXT("StyleTag"))
	{
		if (InKey == TEXT("Categories") || InKey == TEXT("GameplayTagFilter"))
		{
			return BoundStyleSet->StyleSetTag.ToString();
		}
	}

	return Super::GetPinMetaData(InPinName, InKey);
}

void UMDStyleSetNode_GetStyleValue::AllocateDefaultPins()
{
	Super::AllocateDefaultPins();

	UpdatePinData();
}

void UMDStyleSetNode_GetStyleValue::ReconstructNode()
{
	Super::ReconstructNode();

	UpdatePinData();
}

void UMDStyleSetNode_GetStyleValue::PinConnectionListChanged(UEdGraphPin* Pin)
{
	Super::PinConnectionListChanged(Pin);

	UpdatePinData();
	UpdatePreviewWidget();
}

void UMDStyleSetNode_GetStyleValue::PinDefaultValueChanged(UEdGraphPin* Pin)
{
	Super::PinDefaultValueChanged(Pin);

	UpdatePreviewWidget();
}

void UMDStyleSetNode_GetStyleValue::ValidateNodeDuringCompilation(FCompilerResultsLog& MessageLog) const
{
	Super::ValidateNodeDuringCompilation(MessageLog);

	const FGameplayTag StyleTag = GetStyleTag();

	if (!IsValid(BoundStyleSet))
	{
		MessageLog.Error(TEXT("The Style Set for @@ is invalid. Try recreating the node."), this);
	}
	else if (StyleTag.IsValid() && !BoundStyleSet->DoesHaveValueWithTag(StyleTag))
	{
		MessageLog.Error(TEXT("The Style Set @@ does not have an entry with tag [%s]."), this, *StyleTag.ToString());
	}
#if WITH_EDITOR
	else
	{
		const UMDStyleSet* ConstStyleSet = BoundStyleSet;
		FDataValidationContext Context;
		ConstStyleSet->IsDataValid(Context);

		for (const FDataValidationContext::FIssue& Issue : Context.GetIssues())
		{
			if (Issue.TokenizedMessage.IsValid())
			{
				MessageLog.AddTokenizedMessage(Issue.TokenizedMessage.ToSharedRef());
			}
			else
			{
				switch (Issue.Severity) {
				case EMessageSeverity::Error:
					MessageLog.Error(*Issue.Message.ToString());
					break;
				case EMessageSeverity::PerformanceWarning:
				case EMessageSeverity::Warning:
					MessageLog.Warning(*Issue.Message.ToString());
					break;
				default:
					MessageLog.Note(*Issue.Message.ToString());
					break;
				}
			}
		}
	}
#endif
}

UObject* UMDStyleSetNode_GetStyleValue::GetJumpTargetForDoubleClick() const
{
	return BoundStyleSet;
}

bool UMDStyleSetNode_GetStyleValue::CanJumpToDefinition() const
{
	return IsValid(BoundStyleSet);
}

void UMDStyleSetNode_GetStyleValue::JumpToDefinition() const
{
#if WITH_EDITOR
	if (IsValid(BoundStyleSet) && GEditor != nullptr)
	{
		if (UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>())
		{
			AssetEditorSubsystem->OpenEditorForAsset(BoundStyleSet);
		}
	}
#endif
}

void UMDStyleSetNode_GetStyleValue::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	auto CustomizeNodeLambda = [](UEdGraphNode* NewNode, bool bIsTemplateNode, TWeakObjectPtr<UMDStyleSet> InStyleSet)
	{
		UMDStyleSetNode_GetStyleValue* InputNode = CastChecked<UMDStyleSetNode_GetStyleValue>(NewNode);
		InputNode->BoundStyleSet = InStyleSet.Get();
	};

	if (ActionRegistrar.IsOpenForRegistration(GetClass()))
	{
		IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();

		static bool bRegisterOnce = true;
		if (bRegisterOnce)
		{
			bRegisterOnce = false;
			if (AssetRegistry.IsLoadingAssets())
			{
				AssetRegistry.OnFilesLoaded().AddLambda([]() { FBlueprintActionDatabase::Get().RefreshClassActions(StaticClass()); });
			}
		}

		TArray<FAssetData> StyleAssets;
		AssetRegistry.GetAssetsByClass(UMDStyleSet::StaticClass()->GetClassPathName(), StyleAssets, true);
		for (const FAssetData& StyleAsset : StyleAssets)
		{
			UBlueprintNodeSpawner* NodeSpawner = UBlueprintNodeSpawner::Create(GetClass());
			check(NodeSpawner != nullptr);

			if (FPackageName::GetPackageMountPoint(StyleAsset.PackageName.ToString()) != NAME_None)
			{
				if (UMDStyleSet* StyleSet = Cast<UMDStyleSet>(StyleAsset.GetAsset()))
				{
					NodeSpawner->CustomizeNodeDelegate = UBlueprintNodeSpawner::FCustomizeNodeDelegate::CreateStatic(CustomizeNodeLambda, MakeWeakObjectPtr(StyleSet));
					ActionRegistrar.AddBlueprintAction(StyleSet, NodeSpawner);
				}
			}
		}
	}
	else if (UMDStyleSet* StyleSet = const_cast<UMDStyleSet*>(Cast<UMDStyleSet>(ActionRegistrar.GetActionKeyFilter())))
	{
		UBlueprintNodeSpawner* NodeSpawner = UBlueprintNodeSpawner::Create(GetClass());
		check(NodeSpawner != nullptr);

		NodeSpawner->CustomizeNodeDelegate = UBlueprintNodeSpawner::FCustomizeNodeDelegate::CreateStatic(CustomizeNodeLambda, MakeWeakObjectPtr(StyleSet));
		ActionRegistrar.AddBlueprintAction(StyleSet, NodeSpawner);
	}
}

FGameplayTag UMDStyleSetNode_GetStyleValue::GetStyleTag() const
{
	FGameplayTag Result = FGameplayTag::EmptyTag;
	if (UEdGraphPin* StyleTagPin = FindPin(TEXT("StyleTag")))
	{
		if (StyleTagPin->LinkedTo.IsEmpty())
		{
			Result.FromExportString(StyleTagPin->GetDefaultAsString(), PPF_SerializedAsImportText);
		}
	}

	return Result;
}

void UMDStyleSetNode_GetStyleValue::UpdatePinData()
{
	if (IsValid(BoundStyleSet))
	{
		if (BoundStyleSet->HasAnyFlags(RF_NeedLoad))
		{
			BoundStyleSet->GetLinker()->Preload(BoundStyleSet);
		}

		if (BoundStyleSet->HasAnyFlags(RF_NeedPostLoad))
		{
			BoundStyleSet->ConditionalPostLoad();
		}

		if (UEdGraphPin* StyleSetPin = FindPin(TEXT("StyleSet")))
		{
			StyleSetPin->bHidden = true;
			if (StyleSetPin->DefaultObject != BoundStyleSet)
			{
				Modify();
				StyleSetPin->DefaultObject = BoundStyleSet;
				PinDefaultValueChanged(StyleSetPin);
				FBlueprintEditorUtils::MarkBlueprintAsModified(FBlueprintEditorUtils::FindBlueprintForNode(this));
			}
		}

		if (UEdGraphPin* ValuePin = FindPin(TEXT("OutValue")))
		{
			if (ValuePin->PinType != BoundStyleSet->StyleType)
			{
				Modify();
				ValuePin->PinType = BoundStyleSet->StyleType;
				FBlueprintEditorUtils::MarkBlueprintAsModified(FBlueprintEditorUtils::FindBlueprintForNode(this));
			}
		}
	}
}

void UMDStyleSetNode_GetStyleValue::UpdatePreviewWidget()
{
	OnUpdatePreview.Broadcast();
}

void UMDStyleSetNode_GetStyleValue::OnPropertyValueChanged(UObject* Object, FPropertyChangedEvent& Event)
{
	static const TSet<const FProperty*> PropertiesThatDirty = {
		UMDStyleSet::StaticClass()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(UMDStyleSet, StyleType)),
		UMDStyleSet::StaticClass()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(UMDStyleSet, StyleSetTag))
	};
	if (Object == BoundStyleSet && PropertiesThatDirty.Contains(Event.Property))
	{
		GetSchema()->ReconstructNode(*this);
	}
}

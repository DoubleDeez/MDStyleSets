// Copyright Dylan Dumesnil. All Rights Reserved.

#include "Widgets/SMDStyleSetGraphNode.h"

#include "Components/VerticalBox.h"
#include "Framework/Application/SlateApplication.h"
#include "MDStyleSet.h"
#include "Nodes/MDStyleSetNode_GetStyleValue.h"
#include "SGraphPanel.h"
#include "TypeHandlers/MDStyleSetTypeHandlerBase.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Layout/SSpacer.h"

namespace SMDSSGN
{
	const FVector2D ValuePadding = FVector2D(0, 8.f);
}

void SMDStyleSetGraphNode::UpdateGraphNode()
{
	SGraphNodeK2Var::UpdateGraphNode();

	UMDStyleSetNode_GetStyleValue* Node = CastChecked<UMDStyleSetNode_GetStyleValue>(GraphNode, ECastCheckedType::NullAllowed);
	if (IsValid(Node))
	{
		Node->OnUpdatePreview.AddSP(this, &SMDStyleSetGraphNode::UpdatePreviewWidget);
	}

	LeftNodeBox->AddSlot()
		.AutoHeight()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Center)
		[
			SNew(SSpacer).Size(this, &SMDStyleSetGraphNode::GetStyleValuePadding)
		];
	RightNodeBox->AddSlot()
		.AutoHeight()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Center)
		[
			SNew(SSpacer).Size(this, &SMDStyleSetGraphNode::GetStyleValuePadding)
		];

	GetOrAddSlot(ENodeZone::BottomLeft)
	.SlotSize(TAttribute<FVector2D>(this, &SMDStyleSetGraphNode::GetStyleValueSize))
	.SlotOffset(TAttribute<FVector2D>(this, &SMDStyleSetGraphNode::GetStyleValueOffset))
	[
		SNew(SHorizontalBox)
		+SHorizontalBox::Slot()
		.VAlign(VAlign_Center)
		.AutoWidth()
		.Padding(0, 0, 4.f, 0)
		[
			SNew(SBox)
			.Visibility(this, &SMDStyleSetGraphNode::GetPreviewWidgetVisibility)
			.ToolTipText(this, &SMDStyleSetGraphNode::GetToolTipText)
			.HeightOverride(24.f)
			[
				SAssignNew(PreviewWidgetSlot, SScaleBox)
				.Stretch(EStretch::ScaleToFitY)
				[
					SNullWidget::NullWidget
				]
			]
		]
		+SHorizontalBox::Slot()
		.VAlign(VAlign_Center)
		.FillWidth(1.f)
		[
			SNew(STextBlock)
			.Text(this, &SMDStyleSetGraphNode::GetLabelText)
			.ToolTipText(this, &SMDStyleSetGraphNode::GetToolTipText)
		]
	];

	UpdatePreviewWidget();
}

FVector2D SMDStyleSetGraphNode::GetStyleValueSize() const
{
	if (const FNodeSlot* Slot = const_cast<SMDStyleSetGraphNode*>(this)->GetSlot(ENodeZone::BottomLeft))
	{
		return Slot->GetWidget()->GetDesiredSize();
	}

	return FVector2D::Zero();
}

FVector2D SMDStyleSetGraphNode::GetStyleValueOffset() const
{
	const FVector2D NodeSize = ComputeDesiredSize(FSlateApplication::Get().GetApplicationScale());
	const FVector2D ContentSize = GetStyleValuePadding();
	return FVector2D((NodeSize.X - ContentSize.X) / 2.f, NodeSize.Y - ContentSize.Y);
}

FVector2D SMDStyleSetGraphNode::GetStyleValuePadding() const
{
	return GetStyleValueSize() + (SMDSSGN::ValuePadding * GetOwnerPanel()->GetZoomAmount());
}

FText SMDStyleSetGraphNode::GetLabelText() const
{
	UMDStyleSetNode_GetStyleValue* Node = CastChecked<UMDStyleSetNode_GetStyleValue>(GraphNode, ECastCheckedType::NullAllowed);
	if (IsValid(Node) && IsValid(Node->BoundStyleSet) && Node->GetStyleTag().IsValid())
	{
		const FGameplayTag StyleTag = Node->GetStyleTag();
		const FText TagText = FText::FromString(StyleTag.ToString().RightChop(Node->BoundStyleSet->StyleSetTag.ToString().Len() + 1));
		if (Node->BoundStyleSet->DoesHaveValueWithTag(StyleTag))
		{
			const FText ValuePreviewText = IsValid(Node->BoundStyleSet->TypeHandler)
				? Node->BoundStyleSet->TypeHandler->CreateValuePreviewText(Node->BoundStyleSet, StyleTag)
				: FText::GetEmpty();
			return ValuePreviewText.IsEmptyOrWhitespace() ? TagText : FText::Format(INVTEXT("{0} ({1})"), TagText, ValuePreviewText);
		}

		return FText::Format(INVTEXT("Style Set '{0}' is missing entry '{1}'"), Node->BoundStyleSet->GetDisplayName(), TagText);
	}

	return FText::GetEmpty();
}

FText SMDStyleSetGraphNode::GetToolTipText() const
{
	UMDStyleSetNode_GetStyleValue* Node = CastChecked<UMDStyleSetNode_GetStyleValue>(GraphNode, ECastCheckedType::NullAllowed);
	if (IsValid(Node) && IsValid(Node->BoundStyleSet) && Node->GetStyleTag().IsValid())
	{
		const FGameplayTag StyleTag = Node->GetStyleTag();
		return FText::Format(INVTEXT("{0}: {1}"), FText::FromString(StyleTag.ToString()), Node->BoundStyleSet->GetValueDisplayName(StyleTag));
	}

	return FText::GetEmpty();
}

EVisibility SMDStyleSetGraphNode::GetPreviewWidgetVisibility() const
{
	const bool bHasPreviewWidget = PreviewWidgetSlot.IsValid()
		&& PreviewWidgetSlot->GetChildren() != nullptr
		&& PreviewWidgetSlot->GetChildren()->GetChildAt(0) != SNullWidget::NullWidget;

	return bHasPreviewWidget ? EVisibility::Visible : EVisibility::Collapsed;
}

void SMDStyleSetGraphNode::UpdatePreviewWidget()
{
	if (PreviewWidgetSlot.IsValid())
	{
		UMDStyleSetNode_GetStyleValue* Node = CastChecked<UMDStyleSetNode_GetStyleValue>(GraphNode, ECastCheckedType::NullAllowed);
		TSharedRef<SWidget> PreviewWidget = IsValid(Node) && IsValid(Node->BoundStyleSet) && IsValid(Node->BoundStyleSet->TypeHandler) && Node->GetStyleTag().IsValid()
			? Node->BoundStyleSet->TypeHandler->CreateValuePreviewWidget(Node->BoundStyleSet, Node->GetStyleTag())
			: SNullWidget::NullWidget;

		PreviewWidgetSlot->SetContent(PreviewWidget);
	}
}

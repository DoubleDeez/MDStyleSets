// Copyright Dylan Dumesnil. All Rights Reserved.

#pragma once

#include "KismetNodes/SGraphNodeK2Var.h"

class SScaleBox;

class MDSTYLESETSEDITOR_API SMDStyleSetGraphNode : public SGraphNodeK2Var
{
public:
	// SGraphNode interface
	virtual void UpdateGraphNode() override;
	// End of SGraphNode interface

	void UpdatePreviewWidget();

private:
	FVector2D GetStyleValueSize() const;
	FVector2D GetStyleValueOffset() const;
	FVector2D GetStyleValuePadding() const;
	FText GetLabelText() const;
	FText GetToolTipText() const;

	EVisibility GetPreviewWidgetVisibility() const;

	TSharedPtr<SScaleBox> PreviewWidgetSlot;
};

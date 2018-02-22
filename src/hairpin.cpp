/////////////////////////////////////////////////////////////////////////////
// Name:        hairpin.cpp
// Author:      Laurent Pugin
// Created:     2014
// Copyright (c) Authors and others. All rights reserved.
/////////////////////////////////////////////////////////////////////////////

#include "hairpin.h"

//----------------------------------------------------------------------------

#include <assert.h>

//----------------------------------------------------------------------------

#include "dynam.h"
#include "functorparams.h"
#include "verticalaligner.h"
#include "vrv.h"

namespace vrv {

//----------------------------------------------------------------------------
// Hairpin
//----------------------------------------------------------------------------

Hairpin::Hairpin()
    : ControlElement("hairpin-")
    , TimeSpanningInterface()
    , AttColor()
    , AttHairpinLog()
    , AttPlacement()
    , AttVerticalAlignment()
{
    RegisterInterface(TimeSpanningInterface::GetAttClasses(), TimeSpanningInterface::IsInterface());
    RegisterAttClass(ATT_COLOR);
    RegisterAttClass(ATT_HAIRPINLOG);
    RegisterAttClass(ATT_PLACEMENT);
    RegisterAttClass(ATT_VERTICALALIGNMENT);

    Reset();
}

Hairpin::~Hairpin() {}

void Hairpin::Reset()
{
    ControlElement::Reset();
    TimeSpanningInterface::Reset();
    ResetColor();
    ResetHairpinLog();
    ResetPlacement();
    AttVerticalAlignment::ResetVerticalAlignment();

    m_leftLink = NULL;
    m_rightLink = NULL;
}

void Hairpin::SetLeftLink(ControlElement *leftLink)
{
    m_leftLink = leftLink;
    if (this->GetDrawingGrpId() != 0) {
        // LogDebug("Grp id LF already set %d", this->GetDrawingGrpId());
        return;
    }

    int grpId = leftLink->GetDrawingGrpId();
    if (grpId == 0) {
        grpId = leftLink->SetDrawingGrpObject(leftLink);
    }
    this->SetDrawingGrpId(grpId);
}

void Hairpin::SetRightLink(ControlElement *rightLink)
{
    m_rightLink = rightLink;
    int grpId = this->GetDrawingGrpId();
    if (grpId == 0) {
        grpId = this->SetDrawingGrpObject(this);
    }

    if (rightLink->GetDrawingGrpId() != 0) {
        // LogDebug("Grp id RL already set %d", rightLink->GetDrawingGrpId());
        return;
    }
    rightLink->SetDrawingGrpId(grpId);
}

//----------------------------------------------------------------------------
// Hairpin functor methods
//----------------------------------------------------------------------------

int Hairpin::PrepareFloatingGrps(FunctorParams *functorParams)
{
    PrepareFloatingGrpsParams *params = dynamic_cast<PrepareFloatingGrpsParams *>(functorParams);
    assert(params);

    if (this->HasVgrp()) {
        this->SetDrawingGrpId(-this->GetVgrp());
    }

    // Only try to link them if start and end are resolved
    if (!this->GetStart() || !this->GetEnd()) return FUNCTOR_CONTINUE;

    for (auto &dynam : params->m_dynams) {
        if (dynam->GetStart() == this->GetStart() && (dynam->GetStaff() == this->GetStaff())) {
            this->SetLeftLink(dynam);
        }
        else if (dynam->GetStart() == this->GetEnd() && (dynam->GetStaff() == this->GetStaff())) {
            this->SetRightLink(dynam);
        }
    }

    for (auto &hairpin : params->m_hairpins) {
        if (hairpin->GetEnd() == this->GetStart() && (hairpin->GetStaff() == this->GetStaff())) {
            if (!this->m_leftLink) this->SetLeftLink(hairpin);
        }
        if (hairpin->GetEnd() == this->GetStart() && (hairpin->GetStaff() == this->GetStaff())) {
            if (!this->m_rightLink) this->SetRightLink(hairpin);
        }
    }

    params->m_hairpins.push_back(this);

    return FUNCTOR_CONTINUE;
}

int Hairpin::ResetDrawing(FunctorParams *functorParams)
{
    // Call parent one too
    ControlElement::ResetDrawing(functorParams);

    m_leftLink = NULL;
    m_rightLink = NULL;

    return FUNCTOR_CONTINUE;
}

} // namespace vrv

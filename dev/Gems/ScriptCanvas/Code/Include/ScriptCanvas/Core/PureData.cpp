/*
* All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
* its licensors.
*
* For complete copyright and license terms please see the LICENSE at the root of this
* distribution (the "License"). All use of this software is governed by the License,
* or, if provided, by the license below or the license accompanying this file. Do not
* remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*
*/

#include "precompiled.h"

#include "PureData.h"

namespace ScriptCanvas
{
    const char* PureData::k_getThis("Get");
    const char* PureData::k_setThis("Set");
    
    void PureData::AddInputAndOutputTypeSlot(const Data::Type& type, const void* source)
    {
        const Data::Type copy(type);
        AddInputDatumSlot(k_setThis, "", AZStd::move(Data::Type(type)), source, Datum::eOriginality::Original);
        AddOutputTypeSlot(k_getThis, "", AZStd::move(Data::Type(copy)), OutputStorage::Optional);
    }

    void PureData::AddInputTypeAndOutputTypeSlot(const Data::Type& type)
    {
        const Data::Type copy(type);
        AddInputTypeSlot(k_setThis, "", AZStd::move(Data::Type(type)), InputTypeContract::DatumType);
        AddOutputTypeSlot(k_getThis, "", AZStd::move(Data::Type(copy)), OutputStorage::Optional);
    }

    void PureData::OnActivate()
    {
        PushThis();

        for (const auto& propertySlotIdsPair : m_propertyAccount.m_getterSetterIdPairs)
        {
            const SlotId& getterSlotId = propertySlotIdsPair.second.first;
            CallGetter(getterSlotId);
        }
    }

    void PureData::OnInputChanged(const Datum& input, const SlotId& id) 
    {
        OnOutputChanged(input);
    }

    AZStd::string_view PureData::GetInputDataName() const
    {
        return k_setThis;
    }

    AZStd::string_view PureData::GetOutputDataName() const
    {
        return k_getThis;
    }

    void PureData::CallGetter(const SlotId& getterSlotId)
    {
        auto getterFuncIt = m_propertyAccount.m_gettersByInputSlot.find(getterSlotId);
        Slot* getterSlot = GetSlot(getterSlotId);
        if (getterSlot && getterFuncIt != m_propertyAccount.m_gettersByInputSlot.end())
        {
            AZStd::vector<AZStd::pair<Node*, const SlotId>> outputNodes(ModConnectedNodes(*getterSlot));

            if (!outputNodes.empty())
            {
                auto getterOutcome = getterFuncIt->second.m_getterFunction(m_inputData[k_thisDatumIndex]);
                if (!getterOutcome)
                {
                    SCRIPTCANVAS_REPORT_ERROR((*this), getterOutcome.GetError().data());
                    return;
                }


                for (auto& nodePtrSlot : outputNodes)
                {
                    if (nodePtrSlot.first)
                    {
                        Node::SetInput(*nodePtrSlot.first, nodePtrSlot.second, getterOutcome.GetValue());
                    }
                }
            }
        }
    }

    void PureData::SetInput(const Datum& input, const SlotId& id)
    {
        if (IsSetThisSlot(id))
        {
            // push this value, as usual
            Node::SetInput(input, id);

            // now, call every getter, as every property has (presumably) been changed
            for (const auto& propertyNameSlotIdsPair : m_propertyAccount.m_getterSetterIdPairs)
            {
                const SlotId& getterSlotId = propertyNameSlotIdsPair.second.first;
                CallGetter(getterSlotId);
            }
        }
        else
        {
            SetProperty(input, id);
        }
    }

    void PureData::SetProperty(const Datum& input, const SlotId& setterId)
    {
        auto methodBySlotIter = m_propertyAccount.m_settersByInputSlot.find(setterId);
        if (methodBySlotIter == m_propertyAccount.m_settersByInputSlot.end())
        {
            AZ_Error("Script Canvas", false, "BehaviorContextObject SlotId %s did not route to a setter", setterId.m_id.ToString<AZStd::string>().data());
            return;
        }
        if (!methodBySlotIter->second.m_setterFunction)
        {
            AZ_Error("Script Canvas", false, "BehaviorContextObject setter is not invocable for SlotId %s is nullptr", setterId.m_id.ToString<AZStd::string>().data());
            return;
        }
        auto setterOutcome = methodBySlotIter->second.m_setterFunction(m_inputData[k_thisDatumIndex], input);
        if (!setterOutcome)
        {
            SCRIPTCANVAS_REPORT_ERROR((*this), setterOutcome.TakeError().data());
            return;
        }
        PushThis();

        auto getterSetterIt = m_propertyAccount.m_getterSetterIdPairs.find(methodBySlotIter->second.m_propertyName);

        if (getterSetterIt != m_propertyAccount.m_getterSetterIdPairs.end())
        {
            CallGetter(getterSetterIt->second.first);
        }
    }


    void PureData::Reflect(AZ::ReflectContext* reflectContext)
    {
        AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(reflectContext);
        if (serializeContext)
        {
            serializeContext->Class<PureData, Node>()
                ->Version(0)
                ;

            AZ::EditContext* editContext = serializeContext->GetEditContext();
            if (editContext)
            {
                editContext->Class<PureData>("PureData", "")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                        ->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)
                    ;
            }
        }
    }
}

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

#pragma once

#include <ScriptCanvas/Libraries/Entity/Entity.h>
#include <ScriptCanvas/Core/NativeDatumNode.h>

namespace ScriptCanvas
{
    namespace Nodes
    {
        namespace Entity
        {
            class EntityRef
                : public NativeDatumNode<EntityRef, Data::EntityIDType>
            {
            public:
                using ParentType = NativeDatumNode<EntityRef, Data::EntityIDType>;
                AZ_COMPONENT(EntityRef, "{0EE5782F-B241-4127-AE53-E6746B00447F}", ParentType);
                
                static void Reflect(AZ::ReflectContext* reflection)
                {
                    if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(reflection))
                    {
                        serializeContext->Class<EntityRef, PureData>()
                            ->Version(2)
                            ;

                        if (AZ::EditContext* editContext = serializeContext->GetEditContext())
                        {
                            // EntityRef node is special in that it's only created when we drag in an entity from the main scene.
                            // And is unmodifiable(essentially an external constant). As such, we hide it from the node palette.
                            editContext->Class<EntityRef>("EntityID", "Stores a reference to an entity")
                                ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                                ->Attribute(AZ::Edit::Attributes::Icon, "Editor/Icons/ScriptCanvas/EntityRef.png")
                                ->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)
                                ->Attribute(AZ::Script::Attributes::ExcludeFrom, AZ::Script::Attributes::ExcludeFlags::List)
                                ;
                        }
                    }
                }
                                
                AZ_INLINE void SetEntityRef(const AZ::EntityId& id)
                {
                    if (auto input = ModInput(GetSlotId(k_setThis)))
                    {
                        // only called on edit time creation, so no need to push out the data, consider cutting this function if possible
                        input->Set(id);
                        OnOutputChanged(*input);
                    }
                }
                                
                void Visit(NodeVisitor& visitor) const override
                {
                    visitor.Visit(*this);
                }
            };
        }
    }
}
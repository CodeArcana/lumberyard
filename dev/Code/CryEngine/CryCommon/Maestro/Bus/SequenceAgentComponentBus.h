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

#include <AzCore/Component/ComponentBus.h>
#include <AzCore/EBus/EBus.h>
#include <AzCore/std/string/string.h>
#include <Maestro/Bus/SequenceComponentBus.h>

namespace Maestro
{
    /*!
    * SequenceAgentComponentRequests EBus Interface
    * Messages serviced by SequenceAgentComponents.
    *
    * The EBus is Id'ed on a pair of SequenceEntityId, SequenceAgentEntityId 
    */

    //
    // SequenceComponents broadcast to SequenceAgentComponents via a pair of Ids:
    //     sequenceEntityId, sequenceAgentEntityId
    using SequenceAgentEventBusId = AZStd::pair<AZ::EntityId, AZ::EntityId>;        // SequenceComponenet Entity Id, SequenceAgent EntityId

    class SequenceAgentComponentBus
        : public AZ::EBusTraits
    {
    public:
        virtual ~SequenceAgentComponentBus() = default;

        //////////////////////////////////////////////////////////////////////////
        // EBusTraits overrides
        static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::ById;
        typedef SequenceAgentEventBusId BusIdType;
        //////////////////////////////////////////////////////////////////////////
    };

    class SequenceAgentComponentRequests
        : public SequenceAgentComponentBus
    {
    public:
        using AnimatablePropertyAddress = Maestro::SequenceComponentRequests::AnimatablePropertyAddress;
        using AnimatedValue = Maestro::SequenceComponentRequests::AnimatedValue;

        //////////////////////////////////////////////////////////////////////////
        // EBusTraits overrides - application is a singleton
        static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;  // Only one component on a entity can implement the events
        //////////////////////////////////////////////////////////////////////////

        //! Called when a SequenceComponent is connected
        virtual void ConnectSequence(const AZ::EntityId& sequenceEntityId) = 0;

        //! Called when a SequenceComponent is disconnected
        virtual void DisconnectSequence() = 0;

        //! Get the value for an animated float property at the given address on the same entity as the agent.
        //! @param returnValue holds the value to get - this must be instance of one of the concrete subclasses of AnimatedValue, corresponding to the type of the property referenced by the animatedAdresss
        //! @param animatedAddress identifies the component and property to be set
        virtual void GetAnimatedPropertyValue(AnimatedValue& returnValue, const AnimatablePropertyAddress& animatableAddress) = 0;

        //! Set the value for an animated property at the given address on the same entity as the agent
        //! @param animatedAddress identifies the component and property to be set
        //! @param value the value to set - this must be instance of one of the concrete subclasses of AnimatedValue, corresponding to the type of the property referenced by the animatedAdresss
        //! @return true if the value was changed.
        virtual bool SetAnimatedPropertyValue(const AnimatablePropertyAddress& animatableAddress, const AnimatedValue& value) = 0;

        //! Returns the Uuid of the type that the 'getter' returns for this animatableAddress
        virtual AZ::Uuid GetAnimatedAddressTypeId(const Maestro::SequenceComponentRequests::AnimatablePropertyAddress& animatableAddress) = 0;
    };

    using SequenceAgentComponentRequestBus = AZ::EBus<SequenceAgentComponentRequests>;

    class SequenceAgentExternalRequests
        : public AZ::EBusTraits
    {
    public:
        static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;
        static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;

        virtual AZ::EntityId AddToEntity(AZ::EntityId entityId) = 0;
    };

    using SequenceAgentExternalBus = AZ::EBus<SequenceAgentExternalRequests>;

} // namespace Maestro

namespace AZStd
{
    template <>
    struct hash < Maestro::SequenceAgentEventBusId >
    {
        inline size_t operator()(const Maestro::SequenceAgentEventBusId& eventBusId) const
        {
            AZStd::hash<AZ::EntityId> entityIdHasher;
            size_t retVal = entityIdHasher(eventBusId.first);
            AZStd::hash_combine(retVal, eventBusId.second);
            return retVal;
        }
    };
}
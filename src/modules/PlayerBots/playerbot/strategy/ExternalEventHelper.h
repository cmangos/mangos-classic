#pragma once
#include "Trigger.h"
#include "triggers/WorldPacketTrigger.h"
#include "playerbot/ChatHelper.h"

namespace ai
{
    class ExternalEventHelper 
    {
    public:
        ExternalEventHelper(AiObjectContext* aiObjectContext) : aiObjectContext(aiObjectContext) {}

        bool ParseChatCommand(std::string command, Player* owner = NULL)
        {
            std::string linkCommand = ChatHelper::parseValue("command", command);
            bool forceCommand = false;

            if (!linkCommand.empty())
            {
                command = linkCommand;
                forceCommand = true;
            }

            if (HandleCommand(command, "", owner, forceCommand))
                return true;

            size_t i = std::string::npos;
            while (true)
            {
                size_t found = command.rfind(" ", i);
                if (found == std::string::npos || !found)
                    break;

                std::string name = command.substr(0, found);
                std::string param = command.substr(found + 1);

                i = found - 1;

                if (HandleCommand(name, param, owner, forceCommand))
                    return true;
            }

            if (!ChatHelper::parseable(command))
                return false;

            if (command.find("Hvalue:help") != std::string::npos || command.find("[h:") != std::string::npos)
            {
                HandleCommand("help", command, owner);
                return true;
            }

            if (owner->isRealPlayer())
            {
                HandleCommand("c", command, owner);
                HandleCommand("t", command, owner);
            }
            return true;
        }

        bool HandlePacket(std::map<uint16, std::string> &handlers, const WorldPacket &packet, Player* owner = NULL)
        {
            uint16 opcode = packet.GetOpcode();
            std::string name = handlers[opcode];
            if (name.empty())
                return true;

            Trigger* trigger = aiObjectContext->GetTrigger(name);
            if (!trigger)
                return true;

            Event event = trigger->Check();            
            if (!event.getSource().empty())
                return false;
                        
            WorldPacket p(packet);
            trigger->ExternalEvent(p, owner);

            return true;
        }

        bool HandleCommand(std::string name, std::string param, Player* owner = NULL, bool forceCommand = false)
        {
            Trigger* trigger = aiObjectContext->GetTrigger(name);
            if (!trigger)
                return false;

            if(!forceCommand)
                trigger->ExternalEvent(param, owner);
            else
                trigger->ExternalEventForce(param, owner);
            return true;
        }

    private:
        AiObjectContext* aiObjectContext;
    };
}

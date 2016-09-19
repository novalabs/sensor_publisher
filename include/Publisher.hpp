/* COPYRIGHT (c) 2016 Nova Labs SRL
 *
 * All rights reserved. All use of this software and documentation is
 * subject to the License Agreement located in the file LICENSE.
 */

#pragma once

#include <core/mw/Publisher.hpp>
#include <core/mw/CoreNode.hpp>
#include <core/utils/BasicSensor.hpp>

#include <core/sensor_publisher/Configuration.hpp>

#include <ModuleConfiguration.hpp>

namespace core {
namespace sensor_publisher {
template <class _DATATYPE, class _MESSAGETYPE>
struct Passthru {
   static inline void
   _(
      const _DATATYPE& from,
      _MESSAGETYPE*    to
   )
   {
      * to = from;
   }
};

template <class _DATATYPE, class _MESSAGETYPE = _DATATYPE, class _CONVERTER = Passthru<_DATATYPE, _MESSAGETYPE> >
class Publisher:
   public core::mw::CoreNode,
   public core::mw::CoreConfigurable<core::sensor_publisher::Configuration>
{
public:
   using DataType    = _DATATYPE;
   using MessageType = _MESSAGETYPE;
   using Converter   = _CONVERTER;

public:
   Publisher(
      const char*                         name,
      core::utils::BasicSensor<DataType>& sensor,
      core::os::Thread::Priority          priority = core::os::Thread::PriorityEnum::NORMAL
   ) :
      CoreNode::CoreNode(name, priority),
      CoreConfigurable<core::sensor_publisher::Configuration>::CoreConfigurable(name),
      _sensor(sensor)
   {
      _workingAreaSize = 256;
   }

   virtual
   ~Publisher()
   {
      teardown();
   }

private:
   core::mw::Publisher<MessageType>    _publisher;
   core::utils::BasicSensor<DataType>& _sensor;

private:
   bool
   onConfigure()
   {
      return isConfigured();
   }


   bool
   onPrepareMW()
   {
      this->advertise(_publisher, configuration().topic);

      return true;
   }

   bool
   onPrepareHW()
   {
      return _sensor.init();
   }

   bool
   onStart()
   {
      return _sensor.start();
   }

   bool
   onLoop()
   {
      MessageType* msgp;
      DataType     tmp;

      if (_sensor.waitUntilReady()) {
         _sensor.update();
         _sensor.get(tmp);
      }

      if (_publisher.alloc(msgp)) {
         Converter::_(tmp, msgp);

         if (!_publisher.publish(*msgp)) {
            return false;
         }
      }

      return true;
   }       // onLoop

   bool
   onStop()
   {
      return _sensor.stop();
   }
};
}
}

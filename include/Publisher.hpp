/* COPYRIGHT (c) 2016 Nova Labs SRL
 *
 * All rights reserved. All use of this software and documentation is
 * subject to the License Agreement located in the file LICENSE.
 */

#include <Core/MW/Publisher.hpp>
#include <Core/MW/CoreNode.hpp>
#include <Core/MW/CoreSensor.hpp>

#include <sensor_publisher/Configuration.hpp>

#include <Configuration.hpp>
/*
 * template <class A, class B>
   struct TypeAdaptor {};

   template <>
   struct TypeAdaptor<GyroData_i16, common_msgs::XYZ_i16> {
   static inline void
   _(
      const GyroData_i16&   from,
      common_msgs::XYZ_i16& to
   )
   {
    //		to.t = from.t;
    to.x = from.x;
    to.y = from.y;
    to.z = from.z;
   }

   static inline void
   _(
      const GyroData_i16&   from,
      common_msgs::XYZ_i16* to
   )
   {
    //to->t = from.t;
    to->x = from.x;
    to->y = from.y;
    to->z = from.z;
   }

   static inline void
   _(
      const common_msgs::XYZ_i16& from,
      GyroData_i16&               to
   )
   {
    //		to.t = from.t;
    to.x = from.x;
    to.y = from.y;
    to.z = from.z;
   }

   static inline void
   _(
      const common_msgs::XYZ_i16& from,
      GyroData_i16*               to
   )
   {
    //to->t = from.t;
    to->x = from.x;
    to->y = from.y;
    to->z = from.z;
   }
   };
 * */

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
      public Core::MW::CoreNode
   {
public:
      using DataType    = _DATATYPE;
      using MessageType = _MESSAGETYPE;
      using Converter   = _CONVERTER;

public:
      Publisher(
         const char*                     name,
         Core::MW::CoreSensor<DataType>& sensor,
         Core::MW::Thread::Priority      priority = Core::MW::Thread::PriorityEnum::NORMAL
      ) :
         CoreNode::CoreNode(name, priority),
         _sensor(sensor)
      {
         _workingAreaSize = 256;
      }

      virtual
      ~Publisher()
      {
         teardown();
      }

public:
      Configuration configuration;

private:
      Core::MW::Publisher<MessageType> _publisher;
      Core::MW::CoreSensor<DataType>&  _sensor;

private:
      bool
      onPrepareMW()
      {
         this->advertise(_publisher, configuration.topic);

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
         bool ret = _sensor.start();


         return ret;
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
      } // onLoop

      bool
      onStop()
      {
         return _sensor.stop();
      }
   };
}

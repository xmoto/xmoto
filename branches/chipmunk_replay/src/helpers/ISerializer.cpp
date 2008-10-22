#include "helpers/ISerializer.h"
#include "GameEvents.h"
#include "xmscene/Block.h"
#include "xmscene/Bike.h"

void ISerializer::registerSerializers()
{
  SerializerFactory::instance()->REGISTER_OBJECT("EventSerializer");
  SerializerFactory::instance()->REGISTER_OBJECT("BlockSerializer");
  SerializerFactory::instance()->REGISTER_OBJECT("BikerSerializer");
}


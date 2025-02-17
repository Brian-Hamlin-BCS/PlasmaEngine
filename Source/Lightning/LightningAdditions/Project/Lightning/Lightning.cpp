#include "Lightning.hpp"

namespace Lightning
{
  //***************************************************************************
  String GetDocumentationStringOrEmpty(StringParam string)
  {
    LightningErrorIfNotStarted(Documentation);

    if (LightningSetup::Instance->Flags & SetupFlags::NoDocumentationStrings)
      return String();

    return string;
  }
  
  //***************************************************************************
  String GetDocumentationCStringOrEmpty(cstr string)
  {
    LightningErrorIfNotStarted(Documentation);

    if (LightningSetup::Instance->Flags & SetupFlags::NoDocumentationStrings)
      return String();

    return string;
  }
}

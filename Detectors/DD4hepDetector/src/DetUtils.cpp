#include "DetUtils.h"




namespace det {
namespace utils {
dd4hep::xml::Component getNodeByStrAttr(const dd4hep::xml::Handle_t& mother, const std::string& nodeName, const std::string& attrName,
                                             const std::string& attrValue) {
  for (dd4hep::xml::Collection_t xCompColl(mother, nodeName.c_str()); nullptr != xCompColl; ++xCompColl) {
    if (xCompColl.attr<std::string>(attrName.c_str()) == attrValue) {
      return static_cast<dd4hep::xml::Component>(xCompColl);
    }
  }
  // in case there was no xml daughter with matching name
  return dd4hep::xml::Component(nullptr);
}

double getAttrValueWithFallback(const dd4hep::xml::Component& node, const std::string& attrName, const double& defaultValue) {
  if (node.hasAttr(_Unicode(attrName.c_str()))) {
    return node.attr<double>(attrName.c_str());
  } else {
    return defaultValue;
  }
}
}
}

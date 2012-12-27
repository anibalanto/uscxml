#include "OSGInvoker.h"
#include "uscxml/URL.h"
#include <glog/logging.h>

#ifdef BUILD_AS_PLUGINS
#include <Pluma/Connector.hpp>
#endif

namespace uscxml {
  
#ifdef BUILD_AS_PLUGINS
PLUMA_CONNECTOR
bool connect(pluma::Host& host) {
	host.add( new OSGInvokerProvider() );
	return true;
}
#endif
  
OSGInvoker::OSGInvoker() {
}

OSGInvoker::~OSGInvoker() {
};

Invoker* OSGInvoker::create(Interpreter* interpreter) {
	OSGInvoker* invoker = new OSGInvoker();
	invoker->_interpreter = interpreter;
	return invoker;
}

Data OSGInvoker::getDataModelVariables() {
	Data data;
	return data;
}

void OSGInvoker::send(SendRequest& req) {
}

void OSGInvoker::cancel(const std::string sendId) {
}

void OSGInvoker::sendToParent(SendRequest& req) {
}

void OSGInvoker::invoke(InvokeRequest& req) {
  tthread::lock_guard<tthread::recursive_mutex> lock(_mutex);
  
  Arabica::XPath::NodeSet<std::string> content = Interpreter::filterChildElements("content", req.dom);

  std::set<std::string> validChilds;
  validChilds.insert("display");
  processChildren(validChilds, content[0]);
}

void OSGInvoker::runOnMainThread() {
  _displays_t::iterator dispIter = _displays.begin();
  if (_mutex.try_lock()) {
    while(dispIter != _displays.end()) {
      dispIter->second->osgViewer::ViewerBase::frame();
      dispIter++;
    }
    _mutex.unlock();
  }
}
  
void OSGInvoker::processDisplay(const Arabica::DOM::Node<std::string>& element) {
//  std::cout << element << std::endl;

  if (_displays.find(element) == _displays.end()) {
    
    int screenId = 0;
    unsigned int actualX = 0;
    unsigned int actualY = 0;
    unsigned int actualWidth = 0;
    unsigned int actualHeight = 0;
    getViewport(element, actualX, actualY, actualWidth, actualHeight, screenId);
    
    CompositeDisplay* compDisp = new CompositeDisplay(actualX, actualY, actualWidth, actualHeight, screenId);
    _displays[element] = compDisp;
    
    std::set<std::string> validChilds;
    validChilds.insert("viewport");
    processChildren(validChilds, element);
  }
}

void OSGInvoker::processViewport(const Arabica::DOM::Node<std::string>& element) {
  if (_displays.find(element.getParentNode()) == _displays.end())
    return;

  CompositeDisplay* compDisp = _displays[element.getParentNode()];
  osgViewer::View* sceneView = new osgViewer::View();
  _views[element] = sceneView;

  osg::Group* group = new osg::Group();
  _nodes[element] = group;
  sceneView->setSceneData(group);
  
  std::string name = (HAS_ATTR(element, "id") ? ATTR(element, "id") : Interpreter::getUUID());

  unsigned int actualX = 0;
  unsigned int actualY = 0;
  unsigned int actualWidth = 0;
  unsigned int actualHeight = 0;
  getViewport(element, actualX, actualY, actualWidth, actualHeight, compDisp);

  osg::Viewport* viewPort = new osg::Viewport(actualX, actualY, actualWidth, actualHeight);
  compDisp->addView(name, viewPort, sceneView);
  
  std::set<std::string> validChilds;
  validChilds.insert("translation");
  validChilds.insert("rotation");
  validChilds.insert("scale");
  validChilds.insert("node");
  processChildren(validChilds, element);
}

void OSGInvoker::processTranslation(const Arabica::DOM::Node<std::string>& element) {
  assert(_nodes.find(element.getParentNode()) != _nodes.end());
  osg::Node* node = _nodes[element.getParentNode()];

  double x = 0, y = 0, z = 0;
  if (HAS_ATTR(element, "x"))
    x = strTo<float>(ATTR(element, "x"));
  if (HAS_ATTR(element, "y"))
    y = strTo<float>(ATTR(element, "y"));
  if (HAS_ATTR(element, "z"))
    z = strTo<float>(ATTR(element, "z"));
  
  osg::Matrix translate;
  translate.makeTranslate(x, y, z);

  osg::MatrixTransform* transform = new osg::MatrixTransform();
  transform->setMatrix(translate);
  node->asGroup()->addChild(transform);
  _nodes[element] = transform;

  std::set<std::string> validChilds;
  validChilds.insert("translation");
  validChilds.insert("rotation");
  validChilds.insert("scale");
  validChilds.insert("node");
  processChildren(validChilds, element);
}

void OSGInvoker::processRotation(const Arabica::DOM::Node<std::string>& element) {
  assert(_nodes.find(element.getParentNode()) != _nodes.end());
  osg::Node* node = _nodes[element.getParentNode()];
  
  double x = 0, y = 0, z = 0, angle = 0;
  if (HAS_ATTR(element, "x"))
    x = strTo<float>(ATTR(element, "x"));
  if (HAS_ATTR(element, "y"))
    y = strTo<float>(ATTR(element, "y"));
  if (HAS_ATTR(element, "z"))
    z = strTo<float>(ATTR(element, "z"));
  if (HAS_ATTR(element, "angle"))
    z = strTo<float>(ATTR(element, "angle"));

  osg::Matrix rotation;
  rotation.makeRotate(angle, x, y, z);
  
  osg::MatrixTransform* transform = new osg::MatrixTransform();
  transform->setMatrix(rotation);
  node->asGroup()->addChild(transform);
  _nodes[element] = transform;

  std::set<std::string> validChilds;
  validChilds.insert("translation");
  validChilds.insert("rotation");
  validChilds.insert("scale");
  validChilds.insert("node");
  processChildren(validChilds, element);
}

void OSGInvoker::processScale(const Arabica::DOM::Node<std::string>& element) {
  assert(_nodes.find(element.getParentNode()) != _nodes.end());
  osg::Node* node = _nodes[element.getParentNode()];
  
  double x = 1, y = 1, z = 1;
  if (HAS_ATTR(element, "x"))
    x = strTo<float>(ATTR(element, "x"));
  if (HAS_ATTR(element, "y"))
    y = strTo<float>(ATTR(element, "y"));
  if (HAS_ATTR(element, "z"))
    z = strTo<float>(ATTR(element, "z"));
  
  osg::Matrix scale;
  scale.makeScale(x, y, z);
  
  osg::MatrixTransform* transform = new osg::MatrixTransform();
  transform->setMatrix(scale);
  node->asGroup()->addChild(transform);
  _nodes[element] = transform;

  std::set<std::string> validChilds;
  validChilds.insert("translation");
  validChilds.insert("rotation");
  validChilds.insert("scale");
  validChilds.insert("node");
  processChildren(validChilds, element);
}

void OSGInvoker::processNode(const Arabica::DOM::Node<std::string>& element) {
  assert(_nodes.find(element.getParentNode()) != _nodes.end());
  osg::Node* parent = _nodes[element.getParentNode()];

  std::string filename;
  if (HAS_ATTR(element, "src")) {
    filename = ATTR(element, "src");
    
    if (filename.length() > 0) {
      std::string extension;
      size_t extensionStart = filename.find_last_of(".");
      if (extensionStart != std::string::npos) {
        extension = filename.substr(extensionStart);
      }
      
      URL srcURI(filename);
      if (!srcURI.toAbsolute(_interpreter->getBaseURI())) {
        LOG(ERROR) << "invoke element has relative src URI with no baseURI set.";
        return;
      }
      filename = srcURI.asLocalFile(extension);
      osg::ref_ptr<osg::Node> model = osgDB::readNodeFile(filename);
      if (model.get())
        parent->asGroup()->addChild(model);

    }
  }
}

void OSGInvoker::processChildren(const std::set<std::string>& validChildren, const Arabica::DOM::Node<std::string>& element) {
  Arabica::DOM::NodeList<std::string> childs = element.getChildNodes();
  for (int i = 0; i < childs.getLength(); ++i) {
    if (childs.item(i).getNodeType() != Arabica::DOM::Node_base::ELEMENT_NODE)
      continue;
    if (false) {
    } else if (boost::iequals(LOCALNAME(childs.item(i)), "node") &&
               validChildren.find("node") != validChildren.end()) {
      processNode(childs.item(i));
    } else if (boost::iequals(LOCALNAME(childs.item(i)), "translation") &&
               validChildren.find("translation") != validChildren.end()) {
      processTranslation(childs.item(i));
    } else if (boost::iequals(LOCALNAME(childs.item(i)), "rotation") &&
               validChildren.find("rotation") != validChildren.end()) {
      processRotation(childs.item(i));
    } else if (boost::iequals(LOCALNAME(childs.item(i)), "scale") &&
               validChildren.find("scale") != validChildren.end()) {
      processScale(childs.item(i));
    } else if (boost::iequals(LOCALNAME(childs.item(i)), "viewport") &&
               validChildren.find("viewport") != validChildren.end()) {
      processViewport(childs.item(i));
    } else if (boost::iequals(LOCALNAME(childs.item(i)), "display") &&
               validChildren.find("display") != validChildren.end()) {
      processDisplay(childs.item(i));
    } else {
      LOG(INFO) << "Unknown XML element " << TAGNAME(childs.item(i));
    }
  }
}

void OSGInvoker::getViewport(const Arabica::DOM::Node<std::string>& element,
                             unsigned int& x,
                             unsigned int& y,
                             unsigned int& width,
                             unsigned int& height,
                             CompositeDisplay* display) {
  getViewport(element, x, y, width, height, display->getWidth(), display->getHeight());

}

void OSGInvoker::getViewport(const Arabica::DOM::Node<std::string>& element,
                             unsigned int& x,
                             unsigned int& y,
                             unsigned int& width,
                             unsigned int& height,
                             int& screenId)
{
  
  screenId = (HAS_ATTR(element, "screenId") ? strTo<int>(ATTR(element, "screenId")) : 0);
  
  unsigned int fullWidth = 0;
  unsigned int fullHeight = 0;
  CompositeDisplay::getResolution(fullWidth, fullHeight, screenId);
  getViewport(element, x, y, width, height, fullWidth, fullHeight);
}

void OSGInvoker::getViewport(const Arabica::DOM::Node<std::string>& element,
                             unsigned int& x,
                             unsigned int& y,
                             unsigned int& width,
                             unsigned int& height,
                             unsigned int fullWidth,
                             unsigned int fullHeight)
{
  if (HAS_ATTR(element, "x")) {
    NumAttr xAttr = NumAttr(ATTR(element, "x"));
    x = strTo<float>(xAttr.value);
    if (boost::iequals(xAttr.unit, "%"))
      x = (x * fullWidth) / 100;
  }
  if (HAS_ATTR(element, "y")) {
    NumAttr yAttr = NumAttr(ATTR(element, "y"));
    y = strTo<float>(yAttr.value);
    if (boost::iequals(yAttr.unit, "%"))
      y = (y * fullHeight) / 100;
  }
  if (HAS_ATTR(element, "width")) {
    NumAttr widthAttr = NumAttr(ATTR(element, "width"));
    width = strTo<float>(widthAttr.value);
    if (boost::iequals(widthAttr.unit, "%"))
      width = (width * fullWidth) / 100;
  }
  if (HAS_ATTR(element, "height")) {
    NumAttr heightAttr = NumAttr(ATTR(element, "height"));
    height = strTo<float>(heightAttr.value);
    if (boost::iequals(heightAttr.unit, "%"))
      height = (height * fullHeight) / 100;
  }
}

osgViewer::View* OSGInvoker::getView(const Arabica::DOM::Node<std::string>& element) {
  Arabica::DOM::Node<std::string> curr = element;
  while(curr && !boost::iequals(LOCALNAME(curr), "viewport")) {
    curr = curr.getParentNode();
  }
  if (curr && _views.find(curr) != _views.end())
    return _views[curr];
  return NULL;
}

  
}
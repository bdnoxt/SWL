#include "swl/Config.h"
#include "swl/graphics/ShapeSceneNode.h"
#include "swl/graphics/ISceneVisitor.h"


#if defined(_DEBUG) && defined(__SWL_CONFIG__USE_DEBUG_NEW)
#include "swl/ResourceLeakageCheck.h"
#define new DEBUG_NEW
#endif


namespace swl {

//--------------------------------------------------------------------------
// class ShapeSceneNode

template<typename SceneVisitor>
#if defined(UNICODE) || defined(_UNICODE)
ShapeSceneNode<SceneVisitor>::ShapeSceneNode(shape_type &shape, const std::wstring &name /*= std::wstring()*/)
#else
ShapeSceneNode<SceneVisitor>::ShapeSceneNode(shape_type &shape, const std::string &name /*= std::string()*/);
#endif
: base_type(name),
  shape_(shape)
{
}

template<typename SceneVisitor>
ShapeSceneNode<SceneVisitor>::ShapeSceneNode(const ShapeSceneNode &rhs)
: base_type(rhs),
  shape_(rhs.shape_)
{
}

template<typename SceneVisitor>
ShapeSceneNode<SceneVisitor>::~ShapeSceneNode()
{
}

template<typename SceneVisitor>
ShapeSceneNode<SceneVisitor> & ShapeSceneNode<SceneVisitor>::operator=(const ShapeSceneNode &rhs)
{
	if (this == &rhs) return *this;
	static_cast<base_type &>(*this) = rhs;
	shape_ = rhs.shape_;
	return *this;
}

void ShapeSceneNode<SceneVisitor>::accept(const visitor_type &visitor) const
{
	visitor.visit(*this);
}

}  // namespace swl

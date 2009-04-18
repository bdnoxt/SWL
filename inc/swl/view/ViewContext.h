#if !defined(__SWL_VIEW__VIEW_CONTEXT__H_)
#define __SWL_VIEW__VIEW_CONTEXT__H_ 1


#include "swl/common/Region.h"


namespace swl {

//-----------------------------------------------------------------------------------
// 

struct ViewContext
{
public:
	//typedef ViewContext base_type;

protected:
	explicit ViewContext(const Region2<int>& drawRegion, const bool isOffScreenUsed)
	: drawRegion_(drawRegion), isOffScreenUsed_(isOffScreenUsed), isActivated_(false), isDrawing_(false)
	{}
public:
	virtual ~ViewContext()
	{}

private:
	ViewContext(const ViewContext&);
	ViewContext& operator=(const ViewContext&);

public:
	/// swap buffers
	virtual bool swapBuffer() = 0;
	/// resize the context
	virtual bool resize(const int x1, const int y1, const int x2, const int y2)
	{
		if (isActivated()) return false;
		drawRegion_ = Region2<int>(x1, y1, x2, y2);
		return true;
	}

	/// activate the context
	virtual bool activate() = 0;
	/// de-activate the context
	virtual bool deactivate() = 0;

	/// get the off-screen flag
	bool isOffScreenUsed() const  {  return isOffScreenUsed_;  }

	/// get the context activation flag
	bool isActivated() const  {  return isActivated_;  }

	/// get the drawing flag
	bool isDrawing() const  {  return isDrawing_;  }

	/// get the native context
	virtual void * getNativeContext() = 0;
	virtual const void * const getNativeContext() const = 0;

	/// get the drawing region
	const Region2<int> & getRegion() const  {  return drawRegion_;  }

protected:
	/// set the context activation flag
	void setActivation(const bool isActivated)  {  isActivated_ = isActivated;  }

	/// set the drawing flag
	void setDrawing(const bool isDrawing)  {  isDrawing_ = isDrawing;  }

protected:
	/// a drawing region
	Region2<int> drawRegion_;

private:
	// a flag to check whether off-screen buffer is used
	bool isOffScreenUsed_;

	/// a context activation flag
	bool isActivated_;

	/// a flag to check whether drawing is proceeding
	bool isDrawing_;
};

}  // namespace swl


#endif  // __SWL_VIEW__VIEW_CONTEXT__H_

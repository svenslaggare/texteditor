#include "renderviewport.h"

float RenderViewPort::top() const {
	return position.y;
}

float RenderViewPort::bottom() const {
	return position.y + height;
}

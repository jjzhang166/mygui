/*!
    @file
    @author     Albert Semenov
    @date       08/2008
    @module
*/
#include "WobbleNodeAnimator.h"

namespace demo
{

	float squaredDistance(const MyGUI::FloatPoint& _value, const MyGUI::FloatPoint& _other)
	{
		float x1 = _other.left-_value.left;
		float y1 = _other.top-_value.top;

		return x1*x1 + y1*y1;				
	}

	float getLength(const MyGUI::FloatPoint& _value)
	{
		return (float)sqrt( _value.left*_value.left + _value.top*_value.top );
	}

	void setLength(MyGUI::FloatPoint& _value, float _len)
	{
		float temp = getLength(_value);
		_value.left *= _len / temp;
		_value.top *= _len / temp;
	}

	float squaredLength(const MyGUI::FloatPoint& _value)
	{
		return _value.left*_value.left + _value.top*_value.top;
	}

	WoobleNodeAnimator::WoobleNodeAnimator() :
		mInertiaMode(false),
		mDragStrength(0.001f),
		mResizeStrength(0.0009f),
		mNode(nullptr),
		mDestroy(true)
	{
	}

	void WoobleNodeAnimator::deserialization(MyGUI::xml::ElementPtr _node, MyGUI::Version _version)
	{
		MyGUI::xml::ElementEnumerator node = _node->getElementEnumerator();
		while (node.next("Property"))
		{
			const std::string& key = node->findAttribute("key");
			const std::string& value = node->findAttribute("value");

			if (key == "DragStrength") mDragStrength = MyGUI::utility::parseFloat(value);
			else if (key == "ResizeStrength") mResizeStrength = MyGUI::utility::parseFloat(value);
		}
	}

	size_t WoobleNodeAnimator::animate(
		bool _update,
		size_t _quad_count,
		MyGUI::VectorQuadData& _data,
		float _time,
		MyGUI::IVertexBuffer* _buffer,
		MyGUI::ITexture* _texture,
		const MyGUI::RenderTargetInfo& _info,
		const MyGUI::IntCoord& _coord,
		bool& _isAnimate
		)
	{
		if (mDestroy)
		{
			return _quad_count;
		}

		// ��������� �������� �������
		if (mOldCoord.size() != _coord.size() && mOldCoord.point() != _coord.point())
		{
			mInertiaPoint.set(0.5, 0.5);
			mInertiaMode = false;

			addInertia(MyGUI::FloatPoint(_coord.left-mOldCoord.left, _coord.top-mOldCoord.top));
		}
		else if (mOldCoord.size() != _coord.size())
		{
			mInertiaMode = true;

			addInertia(MyGUI::FloatPoint(_coord.width - mOldCoord.width, _coord.height-mOldCoord.height));
		}
		else if (mOldCoord.point() != _coord.point())
		{
			const MyGUI::IntPoint& point = MyGUI::InputManager::getInstance().getMousePosition();
			mInertiaPoint = MyGUI::FloatPoint((float)(point.left - _coord.left) / (float)_coord.width , (float)(point.top - _coord.top) / (float)_coord.height);
			mInertiaMode = false;

			addInertia(MyGUI::FloatPoint(_coord.left-mOldCoord.left, _coord.top-mOldCoord.top));
		}

		mOldCoord = _coord;

		addTime(_time);

		bool anim_update = squaredLength(mDragOffset) >= 0.3f;

		if (!anim_update)
		{
			return _quad_count;
		}

		_isAnimate = true;

		const int count_w = 16;
		const int count_h = 16;
		size_t count = count_w * count_h;

		// ���������� ������ ������ ������
		_data.resize(count);

		float vertex_z = _info.maximumDepth;

		float vertex_left = ((_info.pixScaleX * (float)(_coord.left) + _info.hOffset) * 2) - 1;
		float vertex_top = -(((_info.pixScaleY * (float)(_coord.top) + _info.vOffset) * 2) - 1);

		float vertex_width = (_info.pixScaleX * (float)_coord.width * 2);
		float vertex_height = -(_info.pixScaleY * (float)_coord.height * 2);

		float texture_u = (float)_coord.width / (float)_texture->getWidth();
		float texture_v = (float)_coord.height / (float)_texture->getHeight();

		buildQuadVertex(
			MyGUI::FloatCoord(vertex_left, vertex_top, vertex_width, vertex_height), 
			vertex_z,
			_data,
			count_w,
			count_h,
			texture_u,
			texture_v,
			_info.rttFlipY
			);

		return count;
	}

	void WoobleNodeAnimator::addInertia(const MyGUI::FloatPoint& _value)
	{
		const float clampFactor = 50.0f;
		mInertia = mInertia + _value;
		if ( getLength(mInertia) > clampFactor )
			setLength(mInertia, clampFactor);
	}

	void WoobleNodeAnimator::addTime(float _time)
	{
		const float speed = 4;
		_time = std::min(0.05f, _time);

		MyGUI::FloatPoint previousdrag = mDragOffset;

		mDragOffset.left = mDragOffset.left + mInertia.left * 5.0f * speed * _time;
		mDragOffset.top = mDragOffset.top + mInertia.top * 5.0f * speed * _time;
		
		mInertia.left += (mInertia.left * -5 * speed * _time);
		mInertia.left += (previousdrag.left * -4.0f * speed * _time);

		mInertia.top += (mInertia.top * -5 * speed * _time);
		mInertia.top += (previousdrag.top * -4.0f * speed * _time);
	}

	void WoobleNodeAnimator::buildQuadVertex(const MyGUI::FloatCoord& _coord, float _z, MyGUI::VectorQuadData& _data, int _count_w, int _count_h, float _u, float _v, bool _flipY)
	{
		for (int rx=0; rx<_count_w+1; rx++)
		{
			for (int ry=0; ry<_count_h+1; ry++)
			{
				MyGUI::FloatPoint point((float)rx / (float)_count_w, (float)ry / (float)_count_h);

				float drageffect = 0;
				if (mInertiaMode)
				{
					float drageffect1 = squaredDistance(point, MyGUI::FloatPoint(0, 0)) * mResizeStrength;
					float drageffect2 = squaredDistance(point, MyGUI::FloatPoint(1, 1)) * mResizeStrength;

					drageffect = std::min(drageffect1, drageffect2);
				}
				else
				{
					drageffect = squaredDistance(mInertiaPoint, point) * mDragStrength;
				}

				float fx = _coord.left + _coord.width * point.left;
				float fy = _coord.top + _coord.height * point.top;

				MyGUI::FloatPoint vert(fx + (-mDragOffset.left) * drageffect, fy + mDragOffset.top * drageffect);

				float u = point.left * _u;
				float v = point.top * _v;

				if (_flipY) v = 1 - v;

				MyGUI::Vertex vertex;
				vertex.set(vert.left, vert.top, _z, u, v, 0xFFFFFFFF);

				if (rx < _count_w && ry < _count_h)
				{
					_data[rx + ry*_count_w].vertex[MyGUI::VertexQuad::CornerLT] = vertex;
				}

				if (rx > 0 && ry > 0)
				{
					_data[(rx-1) + (ry-1)*_count_w].vertex[MyGUI::VertexQuad::CornerRB] = vertex;
				}

				if (rx > 0 && ry < _count_h)
				{
					_data[(rx-1) + ry*_count_w].vertex[MyGUI::VertexQuad::CornerRT] = vertex;
				}

				if (rx < _count_w && ry > 0)
				{
					_data[rx + (ry-1)*_count_w].vertex[MyGUI::VertexQuad::CornerLB] = vertex;
				}
			}
		}
	}

	void WoobleNodeAnimator::create()
	{
		mDestroy = false;
	}

	void WoobleNodeAnimator::destroy()
	{
		mDestroy = true;
	}

	void WoobleNodeAnimator::attach(MyGUI::ILayerNode* _node)
	{
		mNode = _node->castType<MyGUI::RTTLayerNode>();
		mNode->addLayerNodeAnimation(this);
	}

}
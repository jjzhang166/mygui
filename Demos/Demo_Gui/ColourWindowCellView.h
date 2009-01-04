/*!
	@file
	@author		Albert Semenov
	@date		01/2009
	@module
*/
#ifndef __COLOUR_WINDOW_CELL_VIEW_H__
#define __COLOUR_WINDOW_CELL_VIEW_H__

#include <MyGUI.h>
#include "ItemBox\BaseCellView.h"
#include "ColourWindowCellData.h"

namespace demo
{

	template class wraps::BaseCellView2<ColourWindowCellData>;

	class ColourWindowCellView : public wraps::BaseCellView2<ColourWindowCellData>
	{
	public:
		ColourWindowCellView(MyGUI::WidgetPtr _parent);

		void update(const MyGUI::ItemInfo & _info, ColourWindowCellData * _data);
		static void getCellDimension(MyGUI::WidgetPtr _sender, MyGUI::IntCoord & _coord, bool _drop);

	private:

		MyGUI::StaticTextPtr mText;
		MyGUI::WidgetPtr mColour;
		MyGUI::RawRect * mRawColourView;

	};

} // namespace demo

#endif // __COLOUR_WINDOW_CELL_VIEW_H__

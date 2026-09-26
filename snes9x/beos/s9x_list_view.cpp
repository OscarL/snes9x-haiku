#include <ListView.h>
#include <Mime.h>
#include "s9x_list_view.h"

const int kDEFAULT_ITEM_HEIGHT			= (B_MINI_ICON + 4);
const rgb_color kLIST_COLOR				= { 255, 255, 255, 255 };
const rgb_color kSELECTED_ITEM_COLOR	= { 220, 192, 192, 255 };
const rgb_color	kTEXT_COLOR				= { 0, 0, 0, 255 };
const rgb_color kpercent_TEXT_COLOR		= { 245, 245, 245, 255 };
const rgb_color colors[] = {
	{ 255, 255, 255, 255 },
	{ 245, 245, 245, 255 }
};

s9x_list_view::s9x_list_view(BRect r)
	: BListView(r, "s9x_list_view", B_SINGLE_SELECTION_LIST, B_FOLLOW_ALL) {}

void s9x_list_view::AttachedToWindow()
{
	SetViewColor(B_TRANSPARENT_32_BIT);
	BListView::AttachedToWindow();
}

void s9x_list_view::Draw(BRect where)
{
	BRect	r = Bounds() & where;
	if (r.IsValid()) {
		SetHighColor(kLIST_COLOR);
		FillRect(r);
	}
	BListView::Draw(where);
}

s9x_list_view::~s9x_list_view()
{
	int32 index = 0;
	s9x_list_item *item;
	while ((item = dynamic_cast<s9x_list_item *>(ItemAt(index++))) != 0) {
		delete item;
	}
}

s9x_list_item::s9x_list_item(const char *text, int32 count)
	: BStringItem(text),
	cPos(count)
{
	SetHeight(kDEFAULT_ITEM_HEIGHT);
}

void s9x_list_item::DrawItem(BView *view, BRect rect, bool complete)
{
	float offset = 10;
	BFont font = be_plain_font;
	font_height	finfo;
	
	rgb_color col = colors[this->cPos & 1];
	view->SetHighColor(col);
	view->SetLowColor(col);
	
	view->FillRect(rect);
	rect.InsetBy(-1, -1);
	view->SetHighColor(0, 0, 0);
	view->StrokeRect(rect);
	view->SetHighColor(kTEXT_COLOR);
	
	font.SetSize(10);
	font.GetHeight(&finfo);
	view->SetFont(&font);
	
	view->MovePenTo(offset,
		rect.top + ((rect.Height() - (finfo.ascent + finfo.descent + finfo.leading)) / 2) +
					(finfo.ascent + finfo.descent) - 2);
	view->DrawString(Text());
}

void s9x_list_item::Update(BView *owner, const BFont *finfo)
{
	BStringItem::Update(owner, finfo);
	SetHeight(kDEFAULT_ITEM_HEIGHT);
}

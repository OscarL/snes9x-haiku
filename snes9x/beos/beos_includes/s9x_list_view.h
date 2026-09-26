#ifndef __S9X_LISTVIEW_H__
#define __S9X_LISTVIEW_H__

class s9x_list_view : public BListView
{
public:
	s9x_list_view(BRect r);
	~s9x_list_view();
	void AttachedToWindow();
	void Draw(BRect where);
};

class s9x_list_item : public BStringItem
{
public:
	s9x_list_item(const char *text, int32 count);
	void DrawItem(BView *view, BRect rect, bool complete);
	void Update(BView *owner, const BFont *finfo);
private:
	int cPos;
};

#endif
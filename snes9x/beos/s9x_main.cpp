#include <Application.h>
#include <Roster.h>
#include <unistd.h>
#include "s9x_main_window.h"

class s9x_app : public BApplication
{
public:
	s9x_app();
	virtual void RefsReceived(BMessage *m);
private:
	s9x_main_window *s9x_window;
};

s9x_app::s9x_app()
	: BApplication("application/x-vnd.s9x")
{
	app_info ai;
	BPath path;
	GetAppInfo(&ai);
	BEntry entry(&ai.ref);
	entry.GetPath(&path);
	path.GetParent(&path);
	chdir(path.Path());
	
	BRect r(200, 200, 400, 400);
	s9x_window = new s9x_main_window(r);
	s9x_window->Show();
}

void s9x_app::RefsReceived(BMessage *m)
{
	if(s9x_window) s9x_window->PostMessage(m);
}

int main()
{
	s9x_app a;
	a.Run();
	return(0);
}
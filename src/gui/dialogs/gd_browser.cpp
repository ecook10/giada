/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * gd_browser
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2016 Giovanni A. Zuliani | Monocasual
 *
 * This file is part of Giada - Your Hardcore Loopmachine.
 *
 * Giada - Your Hardcore Loopmachine is free software: you can
 * redistribute it and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * Giada - Your Hardcore Loopmachine is distributed in the hope that it
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Giada - Your Hardcore Loopmachine. If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * -------------------------------------------------------------------------- */


#include "../../core/graphics.h"
#include "../../core/wave.h"
#include "../../core/channel.h"
#include "../../core/sampleChannel.h"
#include "../../core/conf.h"
#include "../../core/const.h"
#include "../../glue/main.h"
#include "../../glue/channel.h"
#include "../../glue/storage.h"
#include "../../utils/gui.h"
#include "../elems/ge_browser.h"
#include "../elems/channel.h"
#include "gd_browser.h"


using std::string;


extern Conf G_Conf;


gdBaseBrowser::gdBaseBrowser(int x, int y, int w, int h, const string &title,
		const string &path, void (*callback)(void*))
	:	gWindow(x, y, w, h, title.c_str()), callback(callback)
{
	set_non_modal();

	groupTop = new Fl_Group(8, 8, w-16, 20);
		where = new gInput(groupTop->x(), groupTop->y(), 152, 20);
		updir	= new gClick(groupTop->x()+groupTop->w()-20, groupTop->y(), 20, 20, "", updirOff_xpm, updirOn_xpm);
	groupTop->end();
	groupTop->resizable(where);

	where->readonly(true);
	where->cursor_color(COLOR_BG_DARK);
	where->value(path.c_str());

	updir->callback(cb_up, (void*) this);

	browser = new gBrowser(8, groupTop->y()+groupTop->h()+8, w-16, h-73);
	browser->loadDir(path);
	if (path == G_Conf.browserLastPath)
		browser->preselect(G_Conf.browserPosition, G_Conf.browserLastValue);

	Fl_Group *groupButtons = new Fl_Group(8, browser->y()+browser->h()+8, w-16, 20);
		ok  	  = new gClick(w-88, groupButtons->y(), 80, 20);
		cancel  = new gClick(w-ok->w()-96, groupButtons->y(), 80, 20, "Cancel");
		status  = new gProgress(8, groupButtons->y(), cancel->x()-16, 20);
		status->minimum(0);
		status->maximum(1);
		status->hide();   // show the bar only if necessary
	groupButtons->resizable(status);
	groupButtons->end();

	end();

	cancel->callback(cb_close, (void*) this);

	resizable(browser);
	size_range(320, 200);

	gu_setFavicon(this);
	show();
}


/* -------------------------------------------------------------------------- */


gdBaseBrowser::~gdBaseBrowser()
{
	G_Conf.browserX = x();
	G_Conf.browserY = y();
	G_Conf.browserW = w();
	G_Conf.browserH = h();
	G_Conf.browserPosition = browser->position();
	G_Conf.browserLastPath = gu_dirname(browser->getSelectedItem());
	G_Conf.browserLastValue = browser->value();
}


/* -------------------------------------------------------------------------- */


void gdBaseBrowser::cb_up   (Fl_Widget *v, void *p) { ((gdBaseBrowser*)p)->__cb_up(); }
void gdBaseBrowser::cb_close(Fl_Widget *v, void *p) { ((gdBaseBrowser*)p)->__cb_close(); }


/* -------------------------------------------------------------------------- */


void gdBaseBrowser::__cb_up() {
	string dir = browser->getCurrentDir();
	dir = dir.substr(0, dir.find_last_of(G_SLASH_STR));  // remove up to the next slash
	browser->loadDir(dir);
	where->value(browser->getCurrentDir().c_str());
}


/* -------------------------------------------------------------------------- */


void gdBaseBrowser::__cb_close() {
	do_callback();
}


/* -------------------------------------------------------------------------- */


void gdBaseBrowser::setStatusBar(float v)
{
	status->value(status->value() + v);
	Fl::wait(0);
}


/* -------------------------------------------------------------------------- */


string gdBaseBrowser::getSelectedItem()
{
	return browser->getSelectedItem();
}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


gdSaveBrowser::gdSaveBrowser(int x, int y, int w, int h, const string &title,
		const string &path, const string &_name, void (*cb)(void*), Channel *ch)
	:	gdBaseBrowser(x, y, w, h, title, path, cb)
{
	channel = ch;

	where->size(groupTop->w()-236, 20);

	name = new gInput(where->x()+where->w()+8, 8, 200, 20);
	name->value(_name.c_str());
	groupTop->add(name);

	browser->callback(cb_down, (void*) this);

	ok->label("Save");
	ok->callback(cb_save, (void*) this);
	ok->shortcut(FL_ENTER);
}


/* -------------------------------------------------------------------------- */


void gdSaveBrowser::cb_save(Fl_Widget *v, void *p) { ((gdSaveBrowser*)p)->__cb_save(); }
void gdSaveBrowser::cb_down(Fl_Widget *v, void *p) { ((gdSaveBrowser*)p)->__cb_down(); }


/* -------------------------------------------------------------------------- */


void gdSaveBrowser::__cb_down()
{
	string path = browser->getSelectedItem();

	if (path.empty())  // when click on an empty area
		return;

	/* if the selected item is a directory just load its content. If it's a file
	 * use it as the file name (i.e. fill name->value()). */

	if (gu_isDir(path)) {
		browser->loadDir(path);
		where->value(browser->getCurrentDir().c_str());
	}
	else
		name->value(browser->getSelectedItem(false).c_str());
}


/* -------------------------------------------------------------------------- */


void gdSaveBrowser::__cb_save()
{
	callback((void*) this);
}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


gdLoadBrowser::gdLoadBrowser(int x, int y, int w, int h, const string &title,
		const string &path, void (*cb)(void*), Channel *ch)
	:	gdBaseBrowser(x, y, w, h, title, path, cb)
{
	channel = ch;

	where->size(groupTop->w()-updir->w()-8, 20);

	browser->callback(cb_down, (void*) this);

	ok->label("Load");
	ok->callback(cb_load, (void*) this);
	ok->shortcut(FL_ENTER);
}




/* -------------------------------------------------------------------------- */


void gdLoadBrowser::cb_load(Fl_Widget *v, void *p) { ((gdLoadBrowser*)p)->__cb_load(); }
void gdLoadBrowser::cb_down(Fl_Widget *v, void *p) { ((gdLoadBrowser*)p)->__cb_down(); }


/* -------------------------------------------------------------------------- */


void gdLoadBrowser::__cb_load()
{
	callback((void*) this);
}


/* -------------------------------------------------------------------------- */


void gdLoadBrowser::__cb_down()
{
	string path = browser->getSelectedItem();

	if (path.empty() || !gu_isDir(path)) // when click on an empty area or not a dir
		return;

	browser->loadDir(path);
	where->value(browser->getCurrentDir().c_str());
}

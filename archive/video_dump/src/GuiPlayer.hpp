/*
	This file is part of vlc_dump.

	vlc_dump is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	vlc_dump is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with vlc_dump.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef GUI_PLAYER
#define GUI_PLAYER

#include "VlcPlayer.hpp"
#include <QWidget>
#include <QUrl>

class ARender;

class GuiPlayer : public QWidget{
	Q_OBJECT
	
	private:
		VlcInstance &inst2;
		VlcPlayer player;
		ARender *render;
		class Ui_player *ui;
		
	protected:
		void dragEnterEvent( QDragEnterEvent *event );
		void dropEvent( QDropEvent *event );
		void mousePressEvent( QMouseEvent *event );
		
	public:
		GuiPlayer( VlcInstance &inst );
		~GuiPlayer();
		
		void start_video( QUrl path );
		void stop_video();
		
		void set_render( ARender *r );
		
	public slots:
		void update_time();
		void set_time( int time ){ player.set_time( time ); }
		void update_record();
};

#endif


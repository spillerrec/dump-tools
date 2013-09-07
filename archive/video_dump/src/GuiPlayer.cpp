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

#include "GuiPlayer.hpp"
#include "render.hpp"
#include "ui_player.h"

#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMouseEvent>
#include <QUrl>
#include <QLayout>


static void event_time_changed( const struct libvlc_event_t *event, void *data ){
	((GuiPlayer*)data)->update_time();
}


GuiPlayer::GuiPlayer( VlcInstance &inst ) : QWidget( NULL ), inst2( inst ), player( inst ), ui( new Ui_player ){
	ui->setupUi( this );
	
	libvlc_event_attach(
			libvlc_media_player_event_manager( player )
		,	libvlc_MediaPlayerTimeChanged
		,	&event_time_changed
		,	this
		);
	
	render = NULL;
	set_render( new Render( (QWidget*)NULL ) );
	
	connect( ui->btn_play, SIGNAL( clicked() ), &player, SLOT( play() ) );
	connect( ui->btn_pause, SIGNAL( clicked() ), &player, SLOT( pause() ) );
	connect( ui->btn_stop, SIGNAL( clicked() ), &player, SLOT( set_stop() ) );
	connect( ui->btn_next, SIGNAL( clicked() ), &player, SLOT( next_chapter() ) );
	connect( ui->btn_prev, SIGNAL( clicked() ), &player, SLOT( previous_chapter() ) );
	connect( ui->btn_record, SIGNAL( clicked() ), this, SLOT( update_record() ) );
	connect( ui->sld_time, SIGNAL( valueChanged(int) ), this, SLOT( set_time(int) ) );
	
	setAcceptDrops( true );
}

GuiPlayer::~GuiPlayer(){
	
}

void GuiPlayer::set_render( ARender *r ){
	if( render != NULL ){
		//TODO: do something...
		layout()->removeWidget( render );
	}
	render = r;
	if( render != NULL ){
		ui->main_layout->insertWidget( 0, render );
		
		//Set callbacks
		render->set_calbacks( player );
		//TODO: remove callbacks?
	}
}


void GuiPlayer::dragEnterEvent( QDragEnterEvent *event ){
	if( event->mimeData()->hasUrls() )
		if( event->mimeData()->urls().count() == 1 )
			event->acceptProposedAction();
}
void GuiPlayer::dropEvent( QDropEvent *event ){
	if( event->mimeData()->hasUrls() ){
		event->setDropAction( Qt::CopyAction );
		
		start_video( event->mimeData()->urls()[0] );
		
		event->accept();
	}
}


void GuiPlayer::start_video( QUrl path ){
	qDebug( "starting video: %s", path.toEncoded().constData() );
	
	//Create a media player playing environement
	VlcMedia media( inst2, path );
	player.set_media( media );
	
	//play the media_player
	player.play();
}


void GuiPlayer::mousePressEvent( QMouseEvent *event ){
	//save = true;
}

void GuiPlayer::update_time(){
	disconnect( ui->sld_time, SIGNAL( valueChanged(int) ), this, SLOT( set_time(int) ) );
	ui->sld_time->setMaximum( player.get_length() );
	ui->sld_time->setValue( player.get_time() );
	connect( ui->sld_time, SIGNAL( valueChanged(int) ), this, SLOT( set_time(int) ) );
}

void GuiPlayer::update_record(){
	render->set_recording( ui->btn_record->isChecked() );
}



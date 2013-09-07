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

#ifndef VLC_PLAYER
#define VLC_PLAYER

#include "VlcInstance.hpp"
#include "VlcMedia.hpp"
#include <QObject>
#include <vlc/vlc.h>

class VlcPlayer : public QObject{
	Q_OBJECT
	
	private:
		libvlc_media_player_t *mp;
		
	public:
		VlcPlayer( const VlcInstance &inst ){ mp = libvlc_media_player_new( inst ); }
		~VlcPlayer(){ libvlc_media_player_release( mp ); }
		
	public slots:
		//Chapter info
		int get_chapter(){ return libvlc_media_player_get_chapter( mp ); }
		int get_chapter_count(){ return libvlc_media_player_get_chapter_count( mp ); }
		int get_chapter_count_for_title( int title ){ return libvlc_media_player_get_chapter_count_for_title( mp, title ); }
		int get_title(){ return libvlc_media_player_get_title( mp ); }
		int get_title_count(){ return libvlc_media_player_get_title_count( mp ); }
		
		//Chapter modifiers
		void next_chapter(){ return libvlc_media_player_next_chapter( mp ); }
		void previous_chapter(){ return libvlc_media_player_previous_chapter( mp ); }
		void set_chapter( int chapter ){ return libvlc_media_player_set_chapter( mp, chapter ); }
		void set_title( int title ){ libvlc_media_player_set_title( mp, title ); }
		
	public:
		//Speed info
		float get_fps(){ return libvlc_media_player_get_fps( mp ); }
		float get_rate(){ return libvlc_media_player_get_rate( mp ); }
		
	public slots:
		//Playback time info
		float get_position(){ return libvlc_media_player_get_position( mp ); }
		libvlc_time_t get_length(){ return libvlc_media_player_get_length( mp ); }
		libvlc_time_t get_time(){ return libvlc_media_player_get_time( mp ); }
		
		//Playback time modifiers
		void set_time( libvlc_time_t time ){ libvlc_media_player_set_time( mp, time ); }
		void set_position( float position ){ libvlc_media_player_set_position( mp, position ); }
		
	public slots:
		//Playback info
		bool will_play(){ return libvlc_media_player_will_play( mp ); }
		bool can_pause(){ return libvlc_media_player_can_pause( mp ); }
		bool is_playing(){ return libvlc_media_player_is_playing( mp ); }
		bool is_seekable(){ return libvlc_media_player_is_seekable( mp ); }
		libvlc_state_t get_state(){ return libvlc_media_player_get_state( mp ); }
		unsigned has_vout(){ return libvlc_media_player_has_vout( mp ); }
		
		//Playback modifiers
		bool play(){ return libvlc_media_player_play( mp ) == 0; } //-1 on error, so true returned on 0
		void pause(){ libvlc_media_player_pause( mp ); }
		void next_frame(){ return libvlc_media_player_next_frame( mp ); }
		void navigate( unsigned navigate ){ libvlc_media_player_navigate( mp, navigate ); }
		
		void set_pause( bool do_pause = false ){ libvlc_media_player_set_pause( mp, do_pause ); }
		void set_stop(){ libvlc_media_player_stop( mp ); }
		
		void set_media( VlcMedia &media ){ libvlc_media_player_set_media( mp, media ); }
		
	public:
		operator libvlc_media_player_t*() const{ return mp; }
};

#endif


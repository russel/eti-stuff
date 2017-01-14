#
/*
 *    Copyright (C) 2013, 2014, 2015
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Programming
 *
 *    This file is part of the  SDR-J (JSDR).
 *    Many of the ideas as implemented in SDR-J are derived from
 *    other work, made available through the GNU general Public License. 
 *    All copyrights of the original authors are acknowledged.
 *
 *    SDR-J is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    SDR-J is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with SDR-J; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include	<QSettings>
#include	<QMessageBox>
#include	<QFileDialog>
#include	<QDebug>
#include	<QDateTime>
#include	<QFile>
#include	<QStringList>
#include	<QStringListModel>
#include	"dab-constants.h"
#include	"gui.h"
#include	"eti-controller.h"
#include	"audiosink.h"
#ifdef	TCP_STREAMER
#include	"tcp-streamer.h"
#endif
/**
  *	We use the creation function merely to set up the
  *	user interface and make the connections between the
  *	gui elements and the handling agents. All real action
  *	is embedded in actions, initiated by gui buttons
  */
	RadioInterface::RadioInterface (QSettings	*Si,
	                                FILE		*input,
	                                QWidget		*parent):
	                                           QMainWindow (parent) {
int16_t	latency;

// 	the setup for the generated part of the ui
	setupUi (this);
	dabSettings		= Si;
//
//	Before printing anything, we set
	setlocale (LC_ALL, "");
///	The default, most likely to be overruled
//
	running			= false;
	
	streamoutSelector	-> hide ();
//
//	latency is used to allow different settings for different
//	situations wrt the output buffering
	latency			=
	           dabSettings -> value ("latency", 1). toInt ();
/**
  *	The current setup of the audio output is that
  *	you have a choice, take one of (soundcard, tcp streamer or rtp streamer)
  */
	audioBuffer		= new RingBuffer<int16_t>(8 * 32768);
	fibBuffer		= new RingBuffer<fibElement> (4);
//
//	In this version, the default is sending the resulting PCM samples to the
//	soundcard. However, defining TCP_STREAMER  will
//	cause the PCM samples to be send to port 20040.
#ifdef	TCP_STREAMER
	soundOut		= new tcpStreamer	(audioBuffer,
	                                                 20040);
#else			// just sound out
	soundOut		= new audioSink		(latency,
	                                                 streamoutSelector,
	                                                 audioBuffer);
#endif
/**
  *	By default we select Mode 1 
  */

	uint8_t dabMode	= 1;
	setModeParameters (dabMode);
/**
  *	The etiController reads the etiframes and processes them.
  *	The fib segments are put in a buffer and the GUI is
  *	asked to invoke the fib handler.
  *	If nothing further is set, the eticontroller
  *	will do nothing. It will just wait until
  *	parameters, for the selection of a subchannel are given.
  *	With that information, the etiController will extract the 
  *	data segment and pass it on to the appropriate decoder.
  */
	my_fibHandler		= new fib_processor	(this);
	my_etiController	= new etiController	(this,
	                                                 &dabModeParameters,
	                                                 input,
	                                                 my_fibHandler,
	                                                 audioBuffer);
//
	init_your_gui ();		// gui specific stuff
}

	RadioInterface::~RadioInterface () {
	fprintf (stderr, "deleting radioInterface\n");
}
//
///	the values for the different Modes:
void	RadioInterface::setModeParameters (uint8_t Mode) {
	if (Mode == 2) {
	   dabModeParameters. dabMode	= 2;
	   dabModeParameters. L		= 76;		// blocks per frame
	   dabModeParameters. K		= 384;		// carriers
	   dabModeParameters. T_null	= 664;		// null length
	   dabModeParameters. T_F	= 49152;	// samples per frame
	   dabModeParameters. T_s	= 638;		// block length
	   dabModeParameters. T_u	= 512;		// useful part
	   dabModeParameters. guardLength	= 126;
	   dabModeParameters. carrierDiff	= 4000;
	} else
	if (Mode == 4) {
	   dabModeParameters. dabMode		= 4;
	   dabModeParameters. L			= 76;
	   dabModeParameters. K			= 768;
	   dabModeParameters. T_F		= 98304;
	   dabModeParameters. T_null		= 1328;
	   dabModeParameters. T_s		= 1276;
	   dabModeParameters. T_u		= 1024;
	   dabModeParameters. guardLength	= 252;
	   dabModeParameters. carrierDiff	= 2000;
	} else 
	if (Mode == 3) {
	   dabModeParameters. dabMode		= 3;
	   dabModeParameters. L			= 153;
	   dabModeParameters. K			= 192;
	   dabModeParameters. T_F		= 49152;
	   dabModeParameters. T_null		= 345;
	   dabModeParameters. T_s		= 319;
	   dabModeParameters. T_u		= 256;
	   dabModeParameters. guardLength	= 63;
	   dabModeParameters. carrierDiff	= 2000;
	} else {	// default = Mode I
	   dabModeParameters. dabMode		= 1;
	   dabModeParameters. L			= 76;
	   dabModeParameters. K			= 1536;
	   dabModeParameters. T_F		= 196608;
	   dabModeParameters. T_null		= 2656;
	   dabModeParameters. T_s		= 2552;
	   dabModeParameters. T_u		= 2048;
	   dabModeParameters. guardLength	= 504;
	   dabModeParameters. carrierDiff	= 1000;
	}
}


static 
const char *table12 [] = {
"none",
"news",
"current affairs",
"information",
"sport",
"education",
"drama",
"arts",
"science",
"talk",
"pop music",
"rock music",
"easy listening",
"light classical",
"classical music",
"other music",
"wheather",
"finance",
"children\'s",
"factual",
"religion",
"phone in",
"travel",
"leisure",
"jazz and blues",
"country music",
"national music",
"oldies music",
"folk music",
"entry 29 not used",
"entry 30 not used",
"entry 31 not used"
};

const char *RadioInterface::get_programm_type_string (uint8_t type) {
	if (type > 0x40) {
	   fprintf (stderr, "GUI: programmtype wrong (%d)\n", type);
	   return (table12 [0]);
	}

	return table12 [type];
}

static
const char *table9 [] = {
"unknown",
"Albanian",
"Breton",
"Catalan",
"Croatian",
"Welsh",
"Czech",
"Danish",
"German",
"English",
"Spanish",
"Esperanto",
"Estonian",
"Basque",
"Faroese",
"French",
"Frisian",
"Irish",
"Gaelic",
"Galician",
"Icelandic",
"Italian",
"Lappish",
"Latin",
"Latvian",
"Luxembourgian",
"Lithuanian",
"Hungarian",
"Maltese",
"Dutch",
"Norwegian",
"Occitan",
"Polish",
"Postuguese",
"Romanian",
"Romansh",
"Serbian",
"Slovak",
"Slovene",
"Finnish",
"Swedish",
"Tuskish",
"Flemish",
"Walloon"
};

const char *RadioInterface::get_programm_language_string (uint8_t language) {
	if (language > 43) {
	   fprintf (stderr, "GUI: wrong language (%d)\n", language);
	   return table9 [0];
	}
	return table9 [language];
}

//
//
//	Most GUI specific things for the initialization are here
void	RadioInterface::init_your_gui (void) {
	ficBlocks               = 0;
	ficSuccess              = 0;

/**
  *	Devices can be included or excluded, setting is in the configuration
  *	files. Inclusion is reflected in the selector on the GUI.
  *	Note that HAVE_EXTIO is only useful for Windows
  */
#ifdef	HAVE_SDRPLAY
	deviceSelector	-> addItem ("sdrplay");
#endif
#ifdef	HAVE_DABSTICK
	deviceSelector	-> addItem ("dabstick");
#endif
#ifdef	HAVE_AIRSPY
	deviceSelector	-> addItem ("airspy");
#endif
#ifdef HAVE_UHD
	deviceSelector	-> addItem("UHD");
#endif
#ifdef HAVE_EXTIO
	deviceSelector	-> addItem("extio");
#endif
#ifdef	HAVE_RTL_TCP
	deviceSelector	-> addItem ("rtl_tcp");
#endif
	
	connect (ensembleDisplay, SIGNAL (clicked (QModelIndex)),
	              this, SLOT (selectService (QModelIndex)));
	connect (startButton, SIGNAL (clicked (void)),
	              this, SLOT (setStart (void)));
	connect (quitButton, SIGNAL (clicked ()),
	              this, SLOT (TerminateProcess (void)));
	connect (audioDumpButton, SIGNAL (clicked (void)),
	              this, SLOT (set_audioDump (void)));
/**	
  *	Happily, Qt is very capable of handling the representation
  *	of the ensemble and selecting an item
  */
	pictureLabel	= NULL;
	saveSlide	= dabSettings -> value ("saveSlides", 1). toInt ();
	ensemble.setStringList (Services);
	ensembleDisplay	-> setModel (&ensemble);
	Services << " ";
	ensemble. setStringList (Services);
	ensembleDisplay	-> setModel (&ensemble);

/**
  *	The only timer we use is for displaying the running time.
  *	The number of seconds passed is kept in numberofSeconds
  */	
	numberofSeconds		= 0;
	displayTimer		= new QTimer ();
	displayTimer		-> setInterval (1000);
	connect (displayTimer,
	         SIGNAL (timeout (void)),
	         this,
	         SLOT (updateTimeDisplay (void)));
//
	audioDumping		= false;
	audiofilePointer	= NULL;
//
//	display the version
	QString v = "dab backend  " ;
	v. append (CURRENT_VERSION);
	versionName	-> setText (v);
//	and start the timer
	displayTimer		-> start (1000);
}
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//	
//	The public slots are called from other places within the dab software
//	so please provide some implementation, perhaps an empty one
//
/**
  *	clearEnsemble
  *	on changing settings, we clear all things in the gui
  *	related to the ensemble.
  *	The function is called from "deep" within the handling code
  *	Potentially a dangerous approach, since the fic handler
  *	might run in a separate thread and generate data to be displayed
  */
void	RadioInterface::clearEnsemble	(void) {
//
//	it obviously means: stop processing
	my_fibHandler		-> clearEnsemble ();
	clear_showElements	();
}

//
//	a slot, called by the fic/fib handlers
void	RadioInterface::addtoEnsemble (const QString &s) {
	Services << s;
	Services. removeDuplicates ();
	ensemble. setStringList (Services);
	ensembleDisplay	-> setModel (&ensemble);
}

//
///	a slot, called by the fib processor
void	RadioInterface::nameofEnsemble (int id, const QString &v) {
QString s;
	(void)v;
	ensembleId		-> display (id);
	ensembleLabel		= v;
	ensembleName		-> setText (v);
}

void	RadioInterface::show_successRate (int s) {
	errorDisplay	-> display (s);
}

void    RadioInterface::show_ficCRC (bool b) {
        if (b)
           ficSuccess ++;
        if (++ficBlocks >= 100) {
           ficRatioDisplay      -> display (ficSuccess);
           ficSuccess   = 0;
           ficBlocks    = 0;
        }
}

//	showLabel is triggered by the message handler
//	the GUI may decide to ignore this
void	RadioInterface::showLabel	(QString s) {
	if (running)
	   dynamicLabel	-> setText (s);
}
//
//	showMOT is triggered by the MOT handler,
//	the GUI may decide to ignore the data sent
//	since data is only sent whenever a data channel is selected
void	RadioInterface::showMOT		(QByteArray data,
	                                 int subtype, QString pictureName) {
	if (!running)
	   return;
	if (pictureLabel != NULL)
	   delete pictureLabel;
	pictureLabel	= new QLabel (NULL);

	const char *type =
	           subtype == 0 ? "GIF" :
	           subtype == 1 ? "JPG" :
	           subtype == 2 ? "BMP" : "PNG";
	QPixmap p;
	p. loadFromData (data, type);
	
	if (saveSlide) {
	   FILE *x = fopen ((pictureName. toLatin1 (). data ()), "w+b");
	   if (x == NULL)
	      fprintf (stderr, "cannot write file %s\n",
	                            pictureName. toLatin1 (). data ());
	   else {
	      (void)fwrite (data. data (), 1, data.length (), x);
	      fclose (x);
	   }
	}
	
	pictureLabel ->  setPixmap (p);
	pictureLabel ->  show ();
}

//
/**
  *	\brief changeinConfiguration
  *	No idea yet what to do, so just give up
  *	with what we were doing. The user will -eventually -
  *	see the new configuration from which he can select
  */
void	RadioInterface::changeinConfiguration	(void) {
}

void	RadioInterface::newAudio	(int rate) {
	if (running) {
	   soundOut	-> audioOut (rate);
	}
}

void	RadioInterface::setStereo	(bool s) {
	(void)s;
}

void	RadioInterface::show_mscErrors	(int er) {
	(void)er;
}
//
void	RadioInterface::show_ipErrors	(int er) {
	(void)er;
}
//
//	This function is only used in the Gui to clear
//	the details of a selection
void	RadioInterface::clear_showElements (void) {
	Services = QStringList ();
	ensemble. setStringList (Services);
	ensembleDisplay		-> setModel (&ensemble);
	my_fibHandler		-> clearEnsemble ();

	ensembleLabel		= QString ();
	ensembleName		-> setText (ensembleLabel);
	dynamicLabel		-> setText ("");
	
//	Then the various displayed items
	ensembleName		-> setText ("   ");
	if (pictureLabel != NULL)
	   delete pictureLabel;
	pictureLabel = NULL;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//	
//	Private slots relate to the modeling of the GUI
//
/**
  *	\brief setStart is a function that is called after pushing
  *	the start button.
  */
void	RadioInterface::setStart	(void) {
	if (running)		// only listen when not running yet
	   return;
//
	clearEnsemble ();		// the display
	my_etiController	-> start ();
	soundOut	-> restart ();
	running = true;
}

/**
  *	\brief TerminateProcess
  *	Pretty critical, since there are many threads involved
  *	A clean termination is what is needed, regardless of the GUI
  */
void	RadioInterface::TerminateProcess (void) {
	running		= false;
	displayTimer	-> stop ();

	if (audioDumping) {
	   soundOut	-> stopDumping ();
	   sf_close (audiofilePointer);
	}
	soundOut		-> stop ();
//
//	everything should be halted by now
	my_etiController	-> stop ();
	delete		soundOut;
	soundOut	= NULL;		// signals may be pending, so careful
	delete		displayTimer;
	if (pictureLabel != NULL)
	   delete pictureLabel;
	pictureLabel = NULL;		// signals may be pending, so careful
	fprintf (stderr, "Termination started\n");
	close ();
	fprintf (stderr, "closed\n");
}
//
//	Whenever the eti controller has read a frame, it will 
//	put a structure with  <fibnumber, fibdata> in a buffer
//	and will send a signal
void	RadioInterface::handle_fibData (void) {
fibElement f;

	while (fibBuffer -> GetRingBufferReadAvailable () > 0) {
	   fibBuffer		-> getDataFromBuffer (&f, 1);
	   my_fibHandler	-> process_FIB (f. fibdata, f. fibno);
	}
}
//	Selecting a service. The interface is GUI dependent,
//	most of the actions are not
//
//	Note that the audiodata or the packetdata contains quite some
//	info on the service (i.e. rate, address, etc)
void	RadioInterface::selectService (QModelIndex s) {
QString a = ensemble. data (s, Qt::DisplayRole). toString ();

	switch (my_fibHandler -> kindofService (a)) {
	   case AUDIO_SERVICE:
	      { audiodata d;
	        my_fibHandler		-> dataforAudioService (a, &d);
	        my_etiController	-> set_audioChannel (d);
	        dabModus -> setText (d. ASCTy == 077 ? "DAB+" : "DAB");
	        showLabel (QString (" "));
	        rateDisplay -> display (d. bitRate);
	        fprintf (stderr, "subchannel %d\n", d. subchId);
	        break;
	      }
	   case PACKET_SERVICE:
	      {  packetdata d;
	         my_fibHandler ->  dataforDataService (a, &d);
	         if ((d.  DSCTy == 0) || (d. bitRate == 0))
	            return;
	         my_etiController	-> set_dataChannel (d);
	         switch (d. DSCTy) {
	            default:
	               showLabel (QString ("unimplemented Data"));
	               break;
	            case 5:
	               showLabel (QString ("Transp. Channel not implemented"));
	               break;
	            case 60:
	               showLabel (QString ("MOT partially implemented"));
	               break;
	            case 59: {
	               }
	               break;
	            case 44:
	               showLabel (QString ("Journaline"));
	               break;
	         }
	        break;
	      }
	   default:
	      return;
	}

	if (pictureLabel != NULL)
	   delete pictureLabel;
	pictureLabel = NULL;
}
//

void	RadioInterface::set_audioDump (void) {
SF_INFO	*sf_info	= (SF_INFO *)alloca (sizeof (SF_INFO));

	if (audioDumping) {
	   soundOut	-> stopDumping ();
	   sf_close (audiofilePointer);
	   audioDumping = false;
	   audioDumpButton	-> setText ("audioDump");
	   return;
	}

	QString file = QFileDialog::getSaveFileName (this,
	                                        tr ("open file .."),
	                                        QDir::homePath (),
	                                        tr ("Sound (*.wav)"));
	file		= QDir::toNativeSeparators (file);
	if (!file. endsWith (".wav", Qt::CaseInsensitive))
	   file. append (".wav");
	sf_info		-> samplerate	= 48000;
	sf_info		-> channels	= 2;
	sf_info		-> format	= SF_FORMAT_WAV | SF_FORMAT_PCM_16;

	audiofilePointer	= sf_open (file. toLatin1 (). data (),
	                                   SFM_WRITE, sf_info);
	if (audiofilePointer == NULL) {
	   qDebug () << "Cannot open " << file. toLatin1 (). data ();
	   return;
	}

	audioDumpButton		-> setText ("WRITING");
	audioDumping		= true;
	soundOut		-> startDumping (audiofilePointer);
}

//
void	RadioInterface::updateTimeDisplay (void) {
//QDateTime	currentTime = QDateTime::currentDateTime ();
//	timeDisplay	-> setText (currentTime.
//	                            toString (QString ("dd.MM.yy:hh:mm:ss")));
	numberofSeconds ++;
	int16_t	numberHours	= numberofSeconds / 3600;
	int16_t	numberMinutes	= (numberofSeconds / 60) % 60;
	QString text = QString ("runtime ");
	text. append (QString::number (numberHours));
	text. append (" hr, ");
	text. append (QString::number (numberMinutes));
	text. append (" min");
	timeDisplay	-> setText (text);
}


#include "QuizEntry.hpp"

#include <stdexcept>
#include <functional>

#include <QMouseEvent>
#include <QSizePolicy>

#include "common/Log.hpp"


MusicQuiz::QuizEntry::QuizEntry(const QString& audioFile, const QString& answer, const size_t points, const size_t startTime, const size_t answerStartTime, const audio::AudioPlayer::Ptr& audioPlayer, QWidget* parent) :
	QPushButton(parent), _audioFile(audioFile), _answer(answer), _points(points), _startTime(startTime), _answerStartTime(answerStartTime), _audioPlayer(audioPlayer)
{
	/** Sanity Check */
	if ( _audioPlayer == nullptr ) {
		throw std::runtime_error("Failed to create quiz entry. Invalid audio player.");
	}

	/** Set Button Text */
	setText("$" + QString::fromLocal8Bit(std::to_string(_points).c_str()));

	/** Set Start State */
	_state = EntryState::IDLE;

	/** Set Object Name */
	setObjectName("QuizEntry");

	/** Set Size Policy */
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	/** Set Entry Type */
	_type = EntryType::Song;
}

MusicQuiz::QuizEntry::QuizEntry(const QString& audioFile, const QString& videoFile, const QString& answer, size_t points, size_t startTime, size_t videoStartTime, size_t answerStartTime,
	const audio::AudioPlayer::Ptr& audioPlayer, const media::VideoPlayer::Ptr& videoPlayer, QWidget* parent) :
	QPushButton(parent), _audioFile(audioFile), _videoFile(videoFile), _answer(answer), _points(points), _startTime(startTime), _videoStartTime(videoStartTime),
	_answerStartTime(answerStartTime), _audioPlayer(audioPlayer), _videoPlayer(videoPlayer)
{
	/** Sanity Check */
	if ( _audioPlayer == nullptr ) {
		throw std::runtime_error("Failed to create quiz entry. Invalid audio player.");
	}

	if ( _videoPlayer == nullptr ) {
		throw std::runtime_error("Failed to create quiz entry. Invalid video player.");
	}

	/** Set Button Text */
	setText("$" + QString::fromLocal8Bit(std::to_string(_points).c_str()));

	/** Set Start State */
	_state = EntryState::IDLE;

	/** Set Object Name */
	setObjectName("QuizEntry");

	/** Set Size Policy */
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	/** Set Entry Type */
	_type = EntryType::Video;

	/** Create Callback Function */
	_mouseEventCallback = std::bind(&MusicQuiz::QuizEntry::handleMouseEvent, this, std::placeholders::_1);
}

void MusicQuiz::QuizEntry::mouseReleaseEvent(QMouseEvent* event)
{
	/** Install Event Filter in Video Player Widget */
	if ( _videoPlayer != nullptr ) {
		_videoPlayer->setMouseEventCallbackFunction(_mouseEventCallback);
	}

	/** Handle Event */
	handleMouseEvent(event);
}

void MusicQuiz::QuizEntry::handleMouseEvent(QMouseEvent* event)
{
	if ( event->button() == Qt::LeftButton ) {
		leftClickEvent();
	} else if ( event->button() == Qt::RightButton ) {
		rightClickEvent();
	}

	/** Set Object Name (this changes the color) */
	switch ( _state )
	{
	case MusicQuiz::QuizEntry::EntryState::IDLE:
		applyColor(QColor(0, 0, 255));
		break;
	case MusicQuiz::QuizEntry::EntryState::PLAYING:
		applyColor(QColor(0, 0, 139));
		break;
	case MusicQuiz::QuizEntry::EntryState::PAUSED:
		applyColor(QColor(255, 215, 0));
		break;
	case MusicQuiz::QuizEntry::EntryState::PLAYING_ANSWER:
		applyColor(QColor(0, 128, 0));
		if ( !_entryAnswered ) {
			_entryAnswered = true;
			emit answered(_points);
		}
		break;
	case MusicQuiz::QuizEntry::EntryState::PLAYED:
		emit played();
		applyColor(_answeredColor);
		break;
	default:
		break;
	}
}

void MusicQuiz::QuizEntry::leftClickEvent()
{
	switch ( _state )
	{
	case EntryState::IDLE: // Start Media
		_state = EntryState::PLAYING;
		if ( _type == EntryType::Song ) {
			_audioPlayer->play(_audioFile.toStdString(), _startTime);
		} else if ( _type == EntryType::Video ) {
			_audioPlayer->stop();
			_videoPlayer->play(_videoFile, _videoStartTime, true);
			_videoPlayer->show();
			_audioPlayer->play(_audioFile.toStdString(), _startTime);
		}
		break;
	case EntryState::PLAYING: // Pause Media
		_state = EntryState::PAUSED;
		_audioPlayer->pause();
		if ( _videoPlayer != nullptr ) {
			_videoPlayer->pause();
		}
		break;
	case EntryState::PAUSED: // Play Answer
		_state = EntryState::PLAYING_ANSWER;
		if ( _type == EntryType::Song ) {
			_audioPlayer->play(_audioFile.toStdString(), _answerStartTime);
		} else if ( _type == EntryType::Video ) {
			_videoPlayer->play(_videoFile, _answerStartTime);
			_videoPlayer->show();
		}

		if ( !_hiddenAnswer ) {
			setText(QString::fromLocal8Bit(_answer.toStdString().c_str()));
		}
		break;
	case EntryState::PLAYING_ANSWER: // Entry Answered
		_state = EntryState::PLAYED;
		_audioPlayer->stop();
		if ( _videoPlayer != nullptr ) {
			_videoPlayer->stop();
			_videoPlayer->hide();
		}
		break;
	case QuizEntry::EntryState::PLAYED: // Play Answer Again
		if ( _type == EntryType::Song ) {
			_audioPlayer->play(_audioFile.toStdString(), _answerStartTime);
		} else if ( _type == EntryType::Video ) {
			_videoPlayer->play(_videoFile, _answerStartTime);
			_videoPlayer->show();
		}
		_state = EntryState::PLAYING_ANSWER;
		break;
	default:
		throw std::runtime_error("Unknown Quiz Entry State Encountered.");
		break;
	}
}

void MusicQuiz::QuizEntry::rightClickEvent()
{
	switch ( _state )
	{
	case EntryState::IDLE:
		break;
	case EntryState::PLAYING: // Back to initial state
		_state = EntryState::IDLE;
		_audioPlayer->pause();
		if ( _videoPlayer != nullptr ) {
			_videoPlayer->pause();
			_videoPlayer->show();
		}
		break;
	case EntryState::PAUSED: // Continue playing
		_state = EntryState::PLAYING;
		_audioPlayer->resume();
		if ( _videoPlayer != nullptr ) {
			_videoPlayer->resume();
			_videoPlayer->show();
		}
		break;
	case EntryState::PLAYING_ANSWER: // Pause Media
		_state = EntryState::PAUSED;
		_audioPlayer->pause();
		if ( _videoPlayer != nullptr ) {
			_videoPlayer->pause();
			_videoPlayer->show();
		}

		if ( !_hiddenAnswer ) {
			setText(QString::fromLocal8Bit(_answer.toStdString().c_str()));
		}
		break;
	case QuizEntry::EntryState::PLAYED: // Back to idle
		_entryAnswered = false;
		_state = EntryState::IDLE;
		setText("$" + QString::fromLocal8Bit(std::to_string(_points).c_str()));
		break;
	default:
		throw std::runtime_error("Unknown Quiz Entry State Encountered.");
		break;
	}
}

void MusicQuiz::QuizEntry::setColor(const QColor& color)
{
	/** Set Color */
	_answeredColor = color;
}

void MusicQuiz::QuizEntry::applyColor(const QColor& color)
{
	std::stringstream ss;
	ss << "background-color	: rgb(" << color.red() << ", " << color.green() << ", " << color.blue() << ");"
		<< "border-color : rgb(" << color.red() << ", " << color.green() << ", " << color.blue() << ");";
	if ( _state == QuizEntry::EntryState::PLAYED ) {
		ss << "color : rgb(" << 255 - color.red() << ", " << 255 - color.green() << ", " << 255 - color.blue() << ");";
	} else {
		ss << "color : Yellow;";
	}
	setStyleSheet(QString::fromStdString(ss.str()));
}

MusicQuiz::QuizEntry::EntryState MusicQuiz::QuizEntry::getEntryState()
{
	return _state;
}

void MusicQuiz::QuizEntry::setHiddenAnswer(bool hidden)
{
	_hiddenAnswer = hidden;
}
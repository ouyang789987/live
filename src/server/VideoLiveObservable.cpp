/*
 * VideoLiveObservable.cpp
 *
 *  Created on: Jan 24, 2015
 *      Author: ouyangfeng
 */
#include <iostream>

#include "VideoLiveObservable.h"

using namespace std;

static VideoLiveObservable* instance = NULL;

void cdear(int type, void* cookie, void* data)
{
	VideoLiveObservable* observer = (VideoLiveObservable*) cookie;
	if (type)
	{
		VencOutputBuffer* out = (VencOutputBuffer*) data;
		observer->notification(out);
	}
	else
	{
		VencSeqHeader* header = (VencSeqHeader*) data;
		observer->cpyVencSeqHeader(header);
	}
}

VideoLiveObservable::VideoLiveObservable()
{
	pthread_mutex_init(&mt, NULL);
	recorder = create_video_recorder(640, 480);
	recorder->setDataCallBack(recorder, (void*) this, (void*) cdear);
	start_video_recorder_newthread(recorder);
}

VideoLiveObservable::~VideoLiveObservable()
{
	stop_video_recorder(recorder);
	free(recorder);
	recorder = NULL;
	pthread_mutex_destroy(&mt);
}

VideoLiveObservable* VideoLiveObservable::createNew()
{
	if (instance == NULL)
	{
		instance = new VideoLiveObservable();
	}
	return instance;
}

void VideoLiveObservable::destory(VideoLiveObservable* instance)
{
	if (instance)
	{
		delete instance;
		instance = NULL;
	}
}

VencSeqHeader* VideoLiveObservable::registerVideoLiveObserver(VideoLiveObserver* observer)
{
	cout << "registerVideoLiveObserver" << endl;
	if (seqhead.length > 0)
	{
		if (observer)
		{
			pthread_mutex_lock(&mt);
			obables.push_back(observer);
			pthread_mutex_unlock(&mt);
		}
		cout << "registerVideoLiveObserver end" << endl;
		return &seqhead;
	}
	return NULL;
}

void VideoLiveObservable::unRegisterVideoLiveObserver(VideoLiveObserver* observer)
{
	cout << "unRegisterVideoLiveObserver" << endl;
	pthread_mutex_lock(&mt);
	if (observer)
	{
		obables.remove(observer);
	}
	pthread_mutex_unlock(&mt);
}

void VideoLiveObservable::cpyVencSeqHeader(VencSeqHeader* head)
{
	this->seqhead.bufptr = (unsigned char*) malloc(head->length);
	this->seqhead.bufsize = head->bufsize;
	this->seqhead.length = head->length;
}

void VideoLiveObservable::notification(VencOutputBuffer* output)
{
	pthread_mutex_lock(&mt);
	if (obables.size() > 0)
	{
		time_t t = time(NULL);
		cout << t << "found " << obables.size() << " VideoLiveObserver" << endl;
		list<VideoLiveObserver*>::iterator it;
		for (it = obables.begin(); it != obables.end(); ++it)
		{
			((VideoLiveObserver*) *it)->update(output);
		}
	}
	pthread_mutex_unlock(&mt);
}

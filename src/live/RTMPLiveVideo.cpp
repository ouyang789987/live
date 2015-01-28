/*
 * live.cpp
 *
 *  Created on: 2015-1-8
 *      Author: maygolf
 */

#include "RTMPLiveVideo.h"

#include "../base/utils.h"


using namespace std;

bool RTMPVideoLive::init()
{
	recorder->setDataCallBack(recorder, (void*) this, (void*) recevie_data);
	output->open();
	return true;
}

RTMPVideoLive::RTMPVideoLive(char* server, int width, int height)
{
	output = new RTMPOutPutStream(server, true, 30);
	flvEncoder = new FLVEncoder(output);
	recorder = create_video_recorder(width, height);
}

RTMPVideoLive::~RTMPVideoLive()
{
	if (output)
		delete output;
	output = NULL;

	if (flvEncoder)
		delete flvEncoder;
	flvEncoder = NULL;

}

int recevie_data(int type, void* cookie, void* data)
{
	RTMPVideoLive* live = (RTMPVideoLive*) cookie;

	if (type)
	{
		/**/
		H264MemReader reader;
		reader.setVencOutputBuffer((VencOutputBuffer*) data);
		/**/
		H264NALU* bytes = (H264NALU*) reader.reader();
		if (bytes)
		{
			live->flvEncoder->encoder(bytes);
			bytes->setData(NULL);
			delete bytes;
			bytes = NULL;
		}
	}
	else
	{
		//解析sps pps
		H264MemReader reader;
		reader.setVencSeqHeader((VencSeqHeader*) data);
		int i = 0;
		while (true)
		{ //读取sps pss
			H264NALU* bytes = (H264NALU*) reader.reader();
			if (bytes)
			{
				live->flvEncoder->encoder(bytes);
				delete bytes;
				bytes = NULL;
				i++;
			}
			else
			{
				//读取失败
				break;
			}

		}
	}
	return 1;
}

bool RTMPVideoLive::start()
{
	start_video_recorder(recorder);
	return true;
}

bool RTMPVideoLive::stop()
{
	output->close();
	stop_video_recorder(recorder);
	free(recorder);
	return true;
}

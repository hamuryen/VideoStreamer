#include <iostream>

#include "liveMedia.hh"
#include "GroupsockHelper.hh"
#include "BasicUsageEnvironment.hh"

//#define VIDEO
#define RTSP

static void announceStream(RTSPServer* rtspServer, ServerMediaSession* sms, char const* streamName, char const* inputFileName)
{
	char* url = rtspServer->rtspURL(sms);
	UsageEnvironment& env = rtspServer->envir();
	env << "\n\"" << streamName << "\" stream, from the source \"" << inputFileName << "\"\n";
	env << "Play this stream using the URL \"" << url << "\"\n\n";
	delete[] url;
}

static char newDemuxWatchVariable;
static MatroskaFileServerDemux* matroskaDemux;
static void onMatroskaDemuxCreation(MatroskaFileServerDemux* newDemux, void*)
{
	matroskaDemux = newDemux;
	newDemuxWatchVariable = 1;
}

int main(int argc, char** argv)
{
	std::string username = "admin";
	std::string password = "admin";
	std::string description = "Session streamed by \"Burak\"";
	std::string streamName = "video";
	std::string source;

	TaskScheduler* scheduler = BasicTaskScheduler::createNew();
	UsageEnvironment* env = BasicUsageEnvironment::createNew(*scheduler);

	UserAuthenticationDatabase* authDB = new UserAuthenticationDatabase;
	authDB->addUserRecord(username.c_str(), password.c_str());

	RTSPServer* rtspServer = RTSPServer::createNew(*env, 8554, authDB);
	if (rtspServer == NULL)
	{
		*env << "Failed to create RTSP server: " << env->getResultMsg() << "\n";
		exit(1);
	}

#ifdef VIDEO
	source = "C:\\Users\\Burak Hamuryen\\Desktop\\video_sample\\test5.mkv";
	newDemuxWatchVariable = 0;

	ServerMediaSession* sms = ServerMediaSession::createNew(*env, streamName.c_str(), description.c_str(), description.c_str());
	MatroskaFileServerDemux::createNew(*env, source.c_str(), onMatroskaDemuxCreation, NULL);
	env->taskScheduler().doEventLoop(&newDemuxWatchVariable);

	bool sessionHasTracks = false;
	ServerMediaSubsession* smss;
	while ((smss = matroskaDemux->newServerMediaSubsession()) != NULL)
	{
		sms->addSubsession(smss);
		sessionHasTracks = true;
	}
	if (sessionHasTracks)
	{
		rtspServer->addServerMediaSession(sms);
	}
#endif // VIDEO

#ifdef RTSP
	source = argv[1];
	std::string sourceUsername = argv[2];
	std::string sourcePassword = argv[3];

	ServerMediaSession* sms = ProxyServerMediaSession::createNew(*env, rtspServer, source.c_str(), streamName.c_str(), sourceUsername.c_str(), sourcePassword.c_str());
	rtspServer->addServerMediaSession(sms);
#endif // RTSP

	announceStream(rtspServer, sms, streamName.c_str(), source.c_str());
	env->taskScheduler().doEventLoop();

	std::cin.get();
	return 0;
}
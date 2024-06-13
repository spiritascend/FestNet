#pragma once


#define CURL_STATICLIB


#include "..\Dependencies\json.hpp"
#pragma comment (lib,"Dependencies/curl/libcurl_a.lib")
#pragma comment (lib,"Normaliz.lib")
#pragma comment (lib,"Ws2_32.lib")
#pragma comment (lib,"Wldap32.lib")
#pragma comment (lib,"Crypt32.lib")
#pragma comment (lib,"advapi32.lib")


#include <filesystem>
#include <fstream>

#include "../Dependencies/globals.h"
#include "..\Dependencies/httplib.h"
#include "..\Dependencies/curl/curl.h"




using json = nlohmann::json;
namespace fs = std::filesystem;



struct SongEntry {
	std::string Name;
	std::string MidiPath;
	std::string SegmentDir;
	std::string id;
};


class api
{
public:
	static void Init();


	static json GetCachedSongs() {
		return cachedsongs;
	}

	static std::string GetActiveSongID() {
		return activesong.id;
	}

	static SongEntry GetActiveSong() {
		return activesong;
	}

	static bool HasActiveSong() {
		return hasactivesong;
	}

private:
	static inline std::string baseconfigdirectory;
	static inline std::string songsconfigpath;
	inline static json cachedsongs;
	static inline SongEntry activesong;
	static inline bool hasactivesong = false;


	static size_t WriteCallback(char* contents, size_t size, size_t nmemb, void* userp);
	static std::string get_query_param(const httplib::Request& req, const std::string& name);

	static void Handle_MainPage(const httplib::Request& req, httplib::Response& res);
	static void Handle_AddSong(const httplib::Request& req, httplib::Response& res);
	static void Handle_RemoveSong(const httplib::Request& req, httplib::Response& res);
	static void Handle_SetActiveSong(const httplib::Request& req, httplib::Response& res);
	static void Handle_RemoveActive(const httplib::Request& req, httplib::Response& res);


	static void Handle_GetSongs(const httplib::Request& req, httplib::Response& res);
	static void Handle_GetActiveSong(const httplib::Request& req, httplib::Response& res);
	static void Handle_GetPlaylist(const httplib::Request& req, httplib::Response& res);



};


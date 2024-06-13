#include "api.h"



std::string api::get_query_param(const httplib::Request& req, const std::string& name) {
    auto it = req.params.find(name);
    if (it != req.params.end()) {
        return it->second;
    }
    return "";
}


size_t api::WriteCallback(char* contents, size_t size, size_t nmemb, void* userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb); return size * nmemb;
}

void api::Init()
{
    auto createserver = []() {
        httplib::Server server;

        baseconfigdirectory = "festnet";
        songsconfigpath = baseconfigdirectory + "/songs.json";


        auto createsongsconfig = [](std::string songsconfigpath) {
            std::ofstream outFile(songsconfigpath);
            if (outFile) {
                outFile << "[]";
            }
            else {
                std::cerr << "Failed to create song config." << std::endl;
            }
            };

        if (!fs::exists(baseconfigdirectory)) {
            if (fs::create_directory(baseconfigdirectory)) {
                createsongsconfig(songsconfigpath);
                cachedsongs = json::array();
            }
            else {
                std::cerr << "Failed to create festnet folder." << std::endl;
            }
        }
        else {
            if (!fs::exists(songsconfigpath)) {
                createsongsconfig(songsconfigpath);
            }
            else {
                std::ifstream songconfigstream(songsconfigpath);
                if (!songconfigstream) {
                    std::cerr << "Failed to get song config: " << songsconfigpath << std::endl;
                    return;
                }

                songconfigstream >> cachedsongs;
            }
        }

        server.Get("/", Handle_MainPage);
        server.Post("/addsong", Handle_AddSong);
        server.Post("/removesong", Handle_RemoveSong);
        server.Get("/songs", Handle_GetSongs);
        server.Get("/getactive", Handle_GetActiveSong);
        server.Post("/setactive", Handle_SetActiveSong);
        server.Get("/getplaylist", Handle_GetPlaylist);
        server.Post("/removeactive", Handle_RemoveActive);


        std::string host = "localhost";

        std::cout << "Starting server on " << host << ":" << HTTP_SERVER_PORT << "..." << std::endl;

        if (!server.listen(host.c_str(), HTTP_SERVER_PORT)) {
            std::cerr << "Error: Failed to start server on " << host << ":" << HTTP_SERVER_PORT << std::endl;
            return;
        }

        std::cout << "Server started successfully." << std::endl;
        };

    std::thread cs(createserver);
    cs.join();
}

void api::Handle_AddSong(const httplib::Request& req, httplib::Response& res) {

    json payload = nlohmann::json::parse(req.body);

    std::string id = payload["id"].get<std::string>();
    std::string name = payload["name"].get<std::string>();
    std::string midipath = payload["midipath"].get<std::string>();
    std::string imageurl = payload["imageurl"].get<std::string>();
    std::string songSegmentDir = payload["songSegmentDir"].get<std::string>();

    if (name.length() == 0 || id.length() == 0 || imageurl.length() == 0 || midipath.length() == 0 || songSegmentDir.length() == 0) {
        res.set_content("invalid request", "text/html");
        res.status = 400;
        return;
    }

    json obj = {
     {"id", id},
     {"name", name},
     {"midipath", midipath},
     {"imageurl", imageurl},
     {"songSegmentDir", songSegmentDir}
    };

    cachedsongs.push_back(obj);


    std::ofstream songconfigstream(songsconfigpath);
    if (!songconfigstream) {
        res.set_content("Failed to get song config when adding song", "text/html");
        res.status = 500;
        return;
    }

   
    songconfigstream << cachedsongs.dump(4);
    songconfigstream.close();
}

void api::Handle_RemoveSong(const httplib::Request& req, httplib::Response& res) {
    std::string id = get_query_param(req, "id");

    if (id.length() == 0) {
        res.set_content("Invalid request: id is required", "text/html");
        res.status = 400;
        return;
    }

    bool found = false;

    for (json::iterator it = cachedsongs.begin(); it != cachedsongs.end(); ++it) {
        std::string objid = (*it)["id"].get<std::string>();

        if (objid == id) {
            cachedsongs.erase(it);
            found = true;
            break;
        }
    }

    if (!found) {
        res.set_content("Failed to find song with the specified id", "text/html");
        res.status = 404;
        return;
    }

    std::ofstream songconfigstream(songsconfigpath);
    if (!songconfigstream) {
        res.set_content("Failed to open song config file for writing", "text/html");
        res.status = 500;
        return;
    }

    songconfigstream << cachedsongs.dump(4);
    songconfigstream.close();

    res.status = 200;
}


void api::Handle_RemoveActive(const httplib::Request& req, httplib::Response& res) {
    if (hasactivesong) {
        hasactivesong = false;
        res.status = 200;
    }
    else {
        res.status = 404;
        res.set_content("no active song found", "text/html");
    }
}

void api::Handle_GetSongs(const httplib::Request& req, httplib::Response& res) {
    res.status = 200;
    res.set_content(cachedsongs.dump(), "text/html");
}

void api::Handle_GetActiveSong(const httplib::Request& req, httplib::Response& res) {
    res.status = 200;
    res.set_content(GetActiveSongID(), "text/html");
}

void api::Handle_GetPlaylist(const httplib::Request& req, httplib::Response& res)
{
    SongEntry Song = activesong;

    std::string manifest_path = activesong.SegmentDir + "\\Manifest.mpd";
    res.set_content("tbd", "");
}

void api::Handle_SetActiveSong(const httplib::Request& req, httplib::Response& res) {
    std::string id = get_query_param(req, "id");
    if (id.size() > 0) {
        bool found = false;
        SongEntry entry;
        for (json::iterator it = cachedsongs.begin(); it != cachedsongs.end(); ++it) {
            std::string objid = (*it)["id"].get<std::string>();

            if (objid == id) {
                std::string midipath = (*it)["midipath"].get<std::string>();
                std::string songSegmentDir = (*it)["songSegmentDir"].get<std::string>();
                std::string name = (*it)["name"].get<std::string>();

                if (midipath.length() > 0 || songSegmentDir.length() > 0 || name.length() > 0) {
                    entry.id = objid;
                    entry.Name = name;
                    entry.SegmentDir = songSegmentDir;
                    entry.MidiPath = midipath;
                    activesong = entry;
                    found = true;
                    hasactivesong = true;
                }
                else {
                    res.status = 400;
                    res.set_content("failed to set active song (corrupted object)", "text/html");
                }
                break;
            }
        }
        return;
    }
    else {
        res.status = 404;
        res.set_content("failed to find id in active song request", "text/html");
    }
}



void api::Handle_MainPage(const httplib::Request& req, httplib::Response& res)
{
    CURL* curl;
    CURLcode response;
    std::string readBuffer;

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "https://gist.githubusercontent.com/spiritascend/9e361bf72dc1cda35811e1b19dd5c4b4/raw");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        response = curl_easy_perform(curl);
        if (response != CURLE_OK) {
            fprintf(stderr, "Failed to send handle_mainpage request: %s\n", curl_easy_strerror(response));
            res.status = 500;
            res.set_content("Failed to fetch static HTML content", "text/html");
        }
        else if (readBuffer.empty()) {
            fprintf(stderr, "Empty response body received from handle_mainpage request\n");
            res.status = 404;
            res.set_content("Static HTML content not found", "text/html");
        }
        else {
            res.set_content(readBuffer, "text/html");
        }

        curl_easy_cleanup(curl);
    }
    else {
        fprintf(stderr, "Failed to initialize CURL\n");
        res.status = 500;
        res.set_content("Failed to initialize CURL", "text/html");
    }
}

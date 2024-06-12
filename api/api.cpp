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
        if (fs::create_directory(baseconfigdirectory)){
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


    std::string host = "localhost";

    std::cout << "Starting server on " << host << ":" << HTTP_SERVER_PORT << "..." << std::endl;

    if (!server.listen(host.c_str(), HTTP_SERVER_PORT)) {
        std::cerr << "Error: Failed to start server on " << host << ":" << HTTP_SERVER_PORT << std::endl;
        return;
    }

    std::cout << "Server started successfully." << std::endl;
}

void api::Handle_AddSong(const httplib::Request& req, httplib::Response& res) {
    std::string name = get_query_param(req, "name");
    std::string id = get_query_param(req, "id");
    std::string url = get_query_param(req, "imageurl");


    if (name.length() == 0 || id.length() == 0 || url.length() == 0) {
        res.set_content("invalid request", "text/html");
        res.status = 400;
        return;
    }

    cachedsongs.push_back(json{ {"name", name},{"id", id},{"imageurl", url} });


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

void api::Handle_GetSongs(const httplib::Request& req, httplib::Response& res) {
    res.status = 200;
    res.set_content(cachedsongs.dump(4), "text/html");
}


void api::Handle_MainPage(const httplib::Request& req, httplib::Response& res)
{
    CURL* curl;
    CURLcode response;
    std::string readBuffer;

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "https://gist.githubusercontent.com/spiritascend/a07ce83e0537f93e51b9e8f5f8cf665d/raw");
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

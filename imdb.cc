#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include "imdb.h"
#include <stdlib.h>
#include <iostream>
#include <iterator>
#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
using namespace std;

const char *const imdb::kActorFileName = "actordata";
const char *const imdb::kMovieFileName = "moviedata";
imdb::imdb(const string& directory) {
  const string actorFileName = directory + "/" + kActorFileName;
  const string movieFileName = directory + "/" + kMovieFileName;
  actorFile = acquireFileMap(actorFileName, actorInfo);
  movieFile = acquireFileMap(movieFileName, movieInfo);
}

bool imdb::good() const {
  return !( (actorInfo.fd == -1) ||
	    (movieInfo.fd == -1) );
}

imdb::~imdb() {
  releaseFileMap(actorInfo);
  releaseFileMap(movieInfo);
}

bool imdb::getCast(const film& movie, vector<string>& players) const {
  int num_movies = *(int*)(movieFile);

  int* begin =(int*)movieFile + 1;
  int* end = (int*)movieFile + num_movies;

  auto low = std::lower_bound(begin, end, movie, [&](int offset,const film& movie) {
      char* ch = (char*) movieFile + offset;
      char other_year_short = *(ch + strlen(ch) + 1);
      int other_year = other_year_short + 1900;
      film other_film = {ch, other_year};
      return (other_film < movie);
    });

  char* ch = (char*) movieFile + *low;
  char other_year_short = *(ch + strlen(ch) + 1);
  int other_year = other_year_short + 1900;
  film other_film = {ch, other_year};
  if (!(other_film == movie)) {
    low = end;
  }

  if (low != end) {
    char* p = (char*) movieFile + *low;
    int title_length = strlen(p);
    p += title_length + 2 + title_length % 2;
    short num_players = *(short*)p;
    p += 2 + (title_length + title_length % 2)%4;
    for (int i = 0; i < num_players; i++) {
      int offset = *((int*)p + i);
      char* ch = (char*)actorFile + offset;
      players.push_back(ch);
    }
    return true;
  }
  return false;
}

bool imdb::getCredits(const string& player, vector<film>& films) const {
  int num_actors = *(int*)(actorFile);
  string name = player;

  int* begin =(int*)actorFile + 1;
  int* end = (int*)actorFile + num_actors;

  auto first = std::lower_bound(begin, end, player, [&](int offset,const string& player) {
      char* ch = (char*) actorFile + offset;
      return (ch < player);
    });

  auto low = (first != end && (*first + (char*)actorFile == player)) ? first : end;

  if (low != end) {

    char* p = (char*)actorFile + *low;
    int name_length = strlen(p);
    p += name_length + 2 - name_length % 2;
    short num_movies =  *(short*)p;
    p += 2 + (name_length - name_length % 2)%4;

    for (int i = 0; i < num_movies; i++) {
      int offset = *((int*)p + i);
      char* ch = (char*)movieFile + offset;
      int year = 1900 + *(ch + 1 +  strlen(ch));
      struct film film = {ch, year};
      films.push_back(film);
    }
    return true;
  }
  return false;
}

const void *imdb::acquireFileMap(const string& fileName, struct fileInfo& info) {
  struct stat stats;
  stat(fileName.c_str(), &stats);
  info.fileSize = stats.st_size;
  info.fd = open(fileName.c_str(), O_RDONLY);
  return info.fileMap = mmap(0, info.fileSize, PROT_READ, MAP_SHARED, info.fd, 0);
}

void imdb::releaseFileMap(struct fileInfo& info) {
  if (info.fileMap != NULL) munmap((char *) info.fileMap, info.fileSize);
  if (info.fd != -1) close(info.fd);
}

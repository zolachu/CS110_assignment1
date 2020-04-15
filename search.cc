#include <vector>
#include <algorithm>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include "imdb.h"
#include <stdlib.h>
#include <fstream>
#include "path.h"
#include <iomanip> // for setw formatter
#include <map>
#include <set>
#include <list>
#include <string>
#include "imdb-utils.h"
using namespace std;

static const int kWrongArgumentCount = 1;
static const int kDatabaseNotFound = 2;



bool bfs(path& p, const string& actor,const string& another, const imdb& db){
  set<string> visitedActor;
  set<film> visitedMovies;

  visitedActor.insert(actor);
  list<path>queue;
  queue.push_back(p);

  while (!queue.empty()) {
    path last_path  =  queue.front();
    queue.pop_front();
    string last_actor = last_path.getLastPlayer();

    vector<film> credits;
    if(db.getCredits(last_actor, credits)) {
      for (const film movie : credits) {
	if (visitedMovies.count(movie)) {
	  continue;
	} else {
	  visitedMovies.insert(movie);
	  vector<string> cast;
	  if (db.getCast(movie, cast)) {
	    for (string coactor : cast) {
	      if (visitedActor.count(coactor)) {
		continue;
	      } else {
		visitedActor.insert(coactor);
		path lastCopy = last_path;
		lastCopy.addConnection(movie, coactor);
		queue.push_back(lastCopy);
		if(coactor == another) {
		  p = lastCopy;
		  return true;
		}
	      }
	    }
	  }
	}
      }
    }
  }
  return false;
}



int main(int argc, char *argv[]) {
  if (argc != 3) {
    fprintf(stderr, "Usage: %s <source-actor> <target-actor>\n", argv[0]);
    return kWrongArgumentCount;
  }

  imdb db(kIMDBDataDirectory);

  if (!db.good()) {
    cerr << "Data directory not found!  Aborting..." << endl;
    return kDatabaseNotFound;
  }

  const string startPlayer = argv[1];
  const string endPlayer = argv[2];

  if ( startPlayer.compare(endPlayer) == 0) {
    cerr << "Ensure that source and target actors are different!" << endl;
    //    fprintf(stderr,"",argv[0]);
    return kWrongArgumentCount;
  }

  path p = path(startPlayer);
  vector<film> credits;
  db.getCredits(startPlayer,credits);       // check bool!!!
  list<string> myPath ;
  if(bfs(p, startPlayer, endPlayer , db)) {
    cout << p << endl;
  } else {
    cout <<"No path between those two people could be found."<< endl;
  }

  return 0;
}

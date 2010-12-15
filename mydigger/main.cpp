/* main.cpp
 */  
#include "ip_filter.h"
#include "header_often.h"
#include "dbg.h"
#include <ctime>

#define MASK 0xFF000000
#define POSTFIX2 ".type2.result"
#define POSTFIX0_1_5 ".type0_1_5.result"
#define POSTFIX3_4 ".type3_4.result"
#define OUTPUT_LOG "output.log"
#define DBG_LOG "dbg.log"
#define CONFIG ".conf"

typedef struct {
  long long lower;
  long long upper;
}means;

int digger (char fname[], unsigned juid_per_ip, float factor1, long long lifetime);

means fetchmeans(vector<long long> & vec);

means fetchmeans(vector<long long> & vec){
  means result;
  size_t size;

  size = vec.size();
  result.lower = 0;
  result.upper = 0;
  
  if(1 == size){
    result.lower = result.upper = vec[0];
  }
  else{
    bool changed = true;
	size_t pos;

    srand(time(NULL));
	pos = rand()%size;
    result.lower = vec[pos];
    pos = (pos+1)%size;
    if(vec[pos] >= result.lower)
      result.upper = vec[pos];
    else{
      result.upper = result.lower;
      result.lower = vec[pos];
    }

    while(changed){
      long long d1, d2, lowcnt=0, upcnt=0, lowergroup=0, uppergroup=0;

      changed = false;
      for(unsigned i=0; i<size; i++){
	d1 = vec[i] - result.lower;
	if(d1 < 0)
	  d1 = -d1;
	d2 = vec[i] - result.upper;
	if(d2 < 0)
	  d2 = -d2;
	if(d1 > d2){
	  upcnt++;
	  uppergroup += vec[i];
	}
	else if(d1 == d2){
	  if(!upcnt){
	    upcnt++;
	    uppergroup += vec[i];
	  }
	  else{
	    lowcnt++;
	    lowergroup += vec[i];
	  }
	}
	else{
	  lowcnt++;
	  lowergroup += vec[i];
	}
      }
      lowergroup /= lowcnt;
      uppergroup /= upcnt;
      if(result.lower != lowergroup){
	result.lower = lowergroup;
	changed = true;
      }
      if(result.upper != uppergroup){
	result.upper = uppergroup;
	changed = true;
      }
    }
  }

  return result;
}

/*
int main(int, char **)
{
  vector <long long> vec;
  means result;
  vec.push_back(16016132422);
  vec.push_back(20924);
  vec.push_back(131721);
  vec.push_back(131721);

  result = fetchmeans(vec);
  cout << result.lower << '\t' << result.upper << endl;

  return 0;
}
*/

int main(int argc, char *argv[])
{
  unsigned juid_per_ip;
  float factor1;
  long long lifetime;
  ofstream flogout(OUTPUT_LOG, ios_base::out | ios_base::app);
  string fname_program, fname_conf;
  fname_program = string(argv[0]);
  size_t pos1, pos2;
  pos1 = fname_program.find_last_of("/\\");
  if(pos1 == string::npos)
    pos1 = 0;
  else{
    if(pos1<fname_program.size())
      pos1 += 1;
    else{
      cerr << "Failed to guess conf file name" << endl;
      exit (-1);
    }
  }
  pos2 = fname_program.find_last_of(".");
  if(pos2 != string::npos){
    pos2 -= pos1;
    if(pos2 <= 0)
      pos2 = string::npos;
  }

  fname_conf = fname_program.substr(pos1, pos2) + CONFIG;
  FILE * fp_conf;
  fp_conf = fopen (fname_conf.c_str(), "r");
  if(NULL == fp_conf){
    cerr << argv[0] << " need a configure file named " 
	 << fname_conf << " like " << endl
	 << "juid_per_ip: unsigned\nFactor1: float\nLIFETIME: unsigned"
	 << endl;
    exit (-1);
  }
  else{
    fscanf(fp_conf, "%*[^:]:%u", &juid_per_ip);
    cout << "JUID_PER_IP: " << juid_per_ip << endl;
    fscanf(fp_conf, "%*[^:]:%f", &factor1);
    cout << "FACTOR1: " << factor1 << endl;
    fscanf(fp_conf, "%*[^:]:%lld", &lifetime);
    cout << "LIFETIME: " << lifetime << endl;
  }

  if(argc < 2){
    cout << "Err: Need at least one file\n";
    flogout << "Err: Need at least one file\n";
    exit(-1);
  }
  else{
    for(int i=1; i<argc; i++)
      digger(argv[i], juid_per_ip, factor1, lifetime);
  }

  return 0;
}

int digger (char fname[], unsigned juid_per_ip, float factor1, long long lifetime)
{
  time_t start, end;
  int ip_cur=0;
  long long diff, diff_total;
  vector <long long> vec;
  means result;
  ifstream fin(fname);
  string ofname(fname), type0_1_5, type3_4, line;
  ip_filter ip_f (3, 0, 256);
  /* When `all' was used, make sure that we have one IP or more. */
  unsigned all = 0, old = 0;
  ofstream ferr(DBG_LOG);
  size_t pos = ofname.find_first_of(".");
  ofname = ofname.substr(0, pos);
  type0_1_5 = ofname + POSTFIX0_1_5;
  type3_4 = ofname + POSTFIX3_4;
  ofname += POSTFIX2;
  ofstream fout(ofname.c_str()), fout0_1_5(type0_1_5.c_str()), fout3_4(type3_4.c_str());

  if (!fin.is_open()){
    cerr << "Can't open file " << fname << end;
    ferr << "Can't open file " << fname << end;
    exit(-1);
  }

  time(&start);
  while(getline(fin, line).good()){ // Get the first IP data that we expect.
    long long time_juid, vtime;
    int ip;
    if(sscanf(line.c_str(), "%d\t%13lld%*[^\t]\t%lld", &ip, &time_juid, &vtime)){ //Three values are scanned.
      if(!ip_f.ok (ip)){
	ip_cur = ip;
	diff = vtime - time_juid;
	all += 1;
	diff_total = diff;
	vec.push_back(diff);
	if(diff >= lifetime)
	  old += 1;
	break;//The first IP was got.
      }
      else{//skip a line
	//cerr << "The line will be skipped as for the IP of this line is not `ok' \n" 
	//     << line << endl;
	continue;
      }
    }
    else{
      cerr << "Failed to parse the line as follow" << endl
	   << line << endl;
      continue;
    }
  }
  
  if(!all){
    cerr << "Can't get a line as we expect from the input file `" << fname << endl
	 << "Do NOTHING and exit" << endl;
    exit(-1);
  }
  
  while(getline(fin, line).good()){ //Do for all.
    long long time_juid, vtime;
    int ip;
    if(sscanf(line.c_str(), "%d\t%13lld%*[^\t]\t%lld", &ip, &time_juid, &vtime) == 3){//skip the last 4 random digits (dec)
      if(!ip_f.ok (ip)){
	diff = vtime - time_juid;
	if(ip == ip_cur){
	  all += 1;
	  diff_total += diff;
	  vec.push_back(diff);
	  if(diff >= lifetime)
	    old += 1;
	}
	else{
	  result = fetchmeans(vec);
	  if(all > juid_per_ip){
	    if((float)old/all < factor1)
	      fout << ip_cur << '\t' << all << '\t' << old << '\t' << diff_total/all << '\t' << result.lower << '\t' << result.upper << '\n';
	    else
	      fout3_4 << ip_cur << '\t' << all << '\t' << old << '\t' << diff_total/all << '\t' << result.lower << '\t' << result.upper << '\n';
	  }
	  else{
	    fout0_1_5 << ip_cur << '\t' << all << '\t' << old << '\t' << diff_total/all << '\t' << result.lower << '\t' << result.upper << '\n';
	  }
	  all = 1;
	  diff_total = diff;
	  vec.clear();
	  vec.push_back(diff);
	  if(diff >= lifetime)
	    old = 1;
	  else 
	    old = 0;
	  ip_cur = ip;
	}
      }
    }
  }

  result = fetchmeans(vec);

  if(all > juid_per_ip){
    if((float)old/all < factor1)
      fout << ip_cur << '\t' << all << '\t' << old << '\t' << diff_total/all << '\t' << result.lower << '\t' << result.upper << '\n';
    else
      fout3_4 << ip_cur << '\t' << all << '\t' << old << '\t' << diff_total/all << '\t' << result.lower << '\t' << result.upper << '\n';
  }
  else{
    fout0_1_5 << ip_cur << '\t' << all << '\t' << old << '\t' << diff_total/all << '\t' << result.lower << '\t' << result.upper << '\n';
  }

  time (&end);

  cout << "Cost " << difftime (end, start) << " seconds totally." << endl;

  return 0;
}



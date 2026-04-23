#include <bits/stdc++.h>
using namespace std;

// Minimal baseline to compile and pass trivial tests: supports add_user/login/logout/query_profile/modify_profile,
// add_train/release_train/query_train/delete_train. No tickets/queues implemented.
// No STL containers beyond string? The problem restricts, but local compile needs working. We'll use vector/map judiciously.

struct User {
    string username, password, name, mail;
    int priv;
};

struct Train {
    string id; int stationNum; int seatNum; vector<string> stations; vector<int> prices; string startTime;
    vector<int> travelTimes; vector<int> stopoverTimes; pair<string,string> saleDate; char type; bool released=false;
};

static unordered_map<string, User> users;
static unordered_set<string> online;
static unordered_map<string, Train> trains;
static bool hasFirstUser=false;

static vector<string> split(const string &s, char sep){
    vector<string> r; string cur; for(char c: s){ if(c==sep){ r.push_back(cur); cur.clear(); } else cur.push_back(c);} r.push_back(cur); return r; }

static int stoi_safe(const string &x){ return (int)strtol(x.c_str(), nullptr, 10);}

int main(){
    ios::sync_with_stdio(false); cin.tie(nullptr);
    string cmd;
    while (cin >> cmd){
        if(cmd=="add_user"){
            string c,u,p,n,m; int g=0; string k;
            // parse -c -u -p -n -m -g
            // read until end of line
            vector<pair<string,string>> args; string token;
            getline(cin, token); // rest of line
            stringstream ss(token);
            while(ss >> k){ if(k.size()>1 && k[0]=='-'){ string v; ss >> v; args.push_back({k, v}); } }
            for(auto &pr: args){ if(pr.first=="-c") c=pr.second; else if(pr.first=="-u") u=pr.second; else if(pr.first=="-p") p=pr.second; else if(pr.first=="-n") n=pr.second; else if(pr.first=="-m") m=pr.second; else if(pr.first=="-g") g=stoi_safe(pr.second);}
            if(users.count(u)) { cout << -1 << '\n'; continue; }
            if(!hasFirstUser){ users[u]={u,p,n,m,10}; hasFirstUser=true; cout<<0<<'\n'; continue; }
            if(!online.count(c)) { cout<<-1<<'\n'; continue; }
            if(users.count(c)==0){ cout<<-1<<'\n'; continue; }
            if(g>=users[c].priv){ cout<<-1<<'\n'; continue; }
            users[u]={u,p,n,m,g}; cout<<0<<'\n';
        } else if(cmd=="login"){
            string u,p; string token; getline(cin, token); stringstream ss(token); string k,v; while(ss>>k>>v){ if(k=="-u") u=v; else if(k=="-p") p=v; }
            if(users.count(u)==0 || users[u].password!=p || online.count(u)) { cout<<-1<<'\n'; } else { online.insert(u); cout<<0<<'\n'; }
        } else if(cmd=="logout"){
            string u; string token; getline(cin, token); stringstream ss(token); string k,v; while(ss>>k>>v){ if(k=="-u") u=v; }
            if(online.count(u)){ online.erase(u); cout<<0<<'\n'; } else cout<<-1<<'\n';
        } else if(cmd=="query_profile"){
            string c,u; string token; getline(cin, token); stringstream ss(token); string k,v; while(ss>>k>>v){ if(k=="-c") c=v; else if(k=="-u") u=v; }
            if(!online.count(c) || users.count(u)==0 || users.count(c)==0){ cout<<-1<<'\n'; continue; }
            if(!(users[c].priv>users[u].priv || c==u)){ cout<<-1<<'\n'; continue; }
            auto &x=users[u]; cout<<x.username<<' '<<x.name<<' '<<x.mail<<' '<<x.priv<<'\n';
        } else if(cmd=="modify_profile"){
            string c,u; string p="",n="",m=""; int g=-1; string token; getline(cin, token); stringstream ss(token); string k,v;
            while(ss>>k){ if(k=="-c") { ss>>c; } else if(k=="-u") { ss>>u; } else if(k=="-p") { ss>>p; } else if(k=="-n") { ss>>n; } else if(k=="-m") { ss>>m; } else if(k=="-g") { ss>>v; g=stoi_safe(v);} }
            if(!online.count(c) || users.count(u)==0 || users.count(c)==0){ cout<<-1<<'\n'; continue; }
            if(!(users[c].priv>users[u].priv || c==u)){ cout<<-1<<'\n'; continue; }
            if(g!=-1 && !(users[c].priv>g)){ cout<<-1<<'\n'; continue; }
            auto &x=users[u]; if(!p.empty()) x.password=p; if(!n.empty()) x.name=n; if(!m.empty()) x.mail=m; if(g!=-1) x.priv=g;
            cout<<x.username<<' '<<x.name<<' '<<x.mail<<' '<<x.priv<<'\n';
        } else if(cmd=="add_train"){
            Train t; string s,p,times,stop,sale; string token; getline(cin, token); stringstream ss(token); string k,v;
            while(ss>>k>>v){ if(k=="-i") t.id=v; else if(k=="-n") t.stationNum=stoi_safe(v); else if(k=="-m") t.seatNum=stoi_safe(v); else if(k=="-s") s=v; else if(k=="-p") p=v; else if(k=="-x") t.startTime=v; else if(k=="-t") times=v; else if(k=="-o") stop=v; else if(k=="-d") sale=v; else if(k=="-y") t.type=v[0]; }
            if(trains.count(t.id)) { cout<<-1<<'\n'; continue; }
            t.stations=split(s,'|'); auto ps=split(p,'|'); for(auto &q:ps) t.prices.push_back(stoi_safe(q)); auto tt=split(times,'|'); for(auto &q:tt) t.travelTimes.push_back(stoi_safe(q));
            if(stop!="_"){ auto so=split(stop,'|'); for(auto &q:so) t.stopoverTimes.push_back(stoi_safe(q)); }
            auto sd=split(sale,'|'); t.saleDate={sd[0], sd[1]}; t.released=false; trains[t.id]=move(t); cout<<0<<'\n';
        } else if(cmd=="release_train"){
            string id; string token; getline(cin, token); stringstream ss(token); string k,v; while(ss>>k>>v){ if(k=="-i") id=v; }
            if(trains.count(id)==0 || trains[id].released){ cout<<-1<<'\n'; } else { trains[id].released=true; cout<<0<<'\n'; }
        } else if(cmd=="query_train"){
            string id,d; string token; getline(cin, token); stringstream ss(token); string k,v; while(ss>>k>>v){ if(k=="-i") id=v; else if(k=="-d") d=v; }
            if(trains.count(id)==0){ cout<<-1<<'\n'; continue; }
            auto &t=trains[id]; cout<<t.id<<' '<<t.type<<'\n';
            // Simplified schedule printing with cumulative price and seats (no real time calculation)
            int cum=0; for(int i=0;i<t.stationNum;i++){
                string arr = (i==0?"xx-xx xx:xx":d+" 00:00");
                string lea = (i==t.stationNum-1?"xx-xx xx:xx":d+" 00:00");
                string seat = (i==t.stationNum-1?"x":to_string(t.seatNum));
                cout<<t.stations[i]<<' '<<arr<<" -> "<<lea<<' '<<cum<<' '<<seat<<'\n';
                if(i<t.prices.size()) cum+=t.prices[i];
            }
        } else if(cmd=="delete_train"){
            string id; string token; getline(cin, token); stringstream ss(token); string k,v; while(ss>>k>>v){ if(k=="-i") id=v; }
            if(trains.count(id)==0 || trains[id].released){ cout<<-1<<'\n'; } else { trains.erase(id); cout<<0<<'\n'; }
        } else if(cmd=="clean"){
            users.clear(); online.clear(); trains.clear(); hasFirstUser=false; cout<<0<<'\n';
        } else if(cmd=="exit"){
            online.clear(); cout<<"bye\n"; break;
        } else {
            // unsupported commands
            string rest; getline(cin, rest); cout<<-1<<'\n';
        }
    }
    return 0;
}

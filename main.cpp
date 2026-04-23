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
    vector<int> arriveOffset; // minutes from start to arrival at i (i>=1), i==0 unused
    vector<int> departOffset; // minutes from start to departure at i
    vector<int> priceCum;     // cum price to station i
};

static unordered_map<string, User> users;
static unordered_set<string> online;
static unordered_map<string, Train> trains;
struct Order { string trainID; string from; string to; string leaveS; string arriveS; int price; int num; string status; int timeIdx; string dateKey; int si; int ti; };
static unordered_map<string, vector<Order>> ordersByUser; // username -> orders (oldest to newest)
static int globalTimeIdx=0;
static unordered_map<string, vector<int>> seatByTrainDate; // key: trainID#MM-DD -> remaining seats per segment
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
            auto sd=split(sale,'|'); t.saleDate={sd[0], sd[1]}; t.released=false;
            // precompute offsets
            t.arriveOffset.assign(t.stationNum, 0);
            t.departOffset.assign(t.stationNum, 0);
            t.priceCum.assign(t.stationNum, 0);
            int cum=0; int cur=0; // start at station 0: departOffset[0]=0
            t.departOffset[0]=0; t.priceCum[0]=0;
            for(int i=1;i<t.stationNum;i++){
                cur += t.travelTimes[i-1];
                t.arriveOffset[i]=cur;
                cum += (i-1<(int)t.prices.size()? t.prices[i-1]:0);
                t.priceCum[i]=cum;
                if(i<t.stationNum-1){
                    cur += t.stopoverTimes[i-1];
                    t.departOffset[i]=cur;
                }
            }
            trains[t.id]=move(t); cout<<0<<'\n';
        } else if(cmd=="release_train"){
            string id; string token; getline(cin, token); stringstream ss(token); string k,v; while(ss>>k>>v){ if(k=="-i") id=v; }
            if(trains.count(id)==0 || trains[id].released){ cout<<-1<<'\n'; } else { trains[id].released=true; cout<<0<<'\n'; }
        } else if(cmd=="query_train"){
            string id,d; string token; getline(cin, token); stringstream ss(token); string k,v; while(ss>>k>>v){ if(k=="-i") id=v; else if(k=="-d") d=v; }
            if(trains.count(id)==0){ cout<<-1<<'\n'; continue; }
            auto &t=trains[id];
            // check date in sale range
            auto parse_md=[&](const string &md){ int mm=(md[0]-'0')*10+(md[1]-'0'); int dd=(md[3]-'0')*10+(md[4]-'0'); return pair<int,int>(mm,dd); };
            auto [mm_d, dd_d]=parse_md(d);
            auto [mm_s, dd_s]=parse_md(t.saleDate.first);
            auto [mm_e, dd_e]=parse_md(t.saleDate.second);
            auto cmp_md=[&](int a_m,int a_d,int b_m,int b_d){ if(a_m!=b_m) return a_m<b_m?-1:1; if(a_d!=b_d) return a_d<b_d?-1: (a_d>b_d?1:0); return 0; };
            if(cmp_md(mm_d,dd_d,mm_s,dd_s)<0 || cmp_md(mm_d,dd_d,mm_e,dd_e)>0){ cout<<-1<<'\n'; continue; }
            // time helpers
            auto parse_hm=[&](const string &hm){ int hr=(hm[0]-'0')*10+(hm[1]-'0'); int mi=(hm[3]-'0')*10+(hm[4]-'0'); return hr*60+mi; };
            auto month_len=[&](int m){ if(m==6) return 30; if(m==7) return 31; return 31; };
            auto add_minutes=[&](int m,int d,int minutes){ int hr = minutes/60; int mi = minutes%60; return tuple<int,int,int>(m,d,hr*60+mi); };
            auto to_str=[&](int m,int d,int minutes){ int hr=minutes/60; int mi=minutes%60; char buf[16]; char bufmd[6]; snprintf(bufmd, sizeof(bufmd), "%02d-%02d", m, d); snprintf(buf, sizeof(buf), "%02d:%02d", hr, mi); string res=string(bufmd)+" "+string(buf); return res; };
            // advance date by delta minutes from start
            auto advance=[&](int m,int d,int base_minutes,int delta){ long long tot=base_minutes+delta; int day_add = (int)(tot/1440); int mins = (int)(tot%1440); if(mins<0){ mins+=1440; day_add--; }
                int nm=m, nd=d+day_add; while(nd>month_len(nm)){ nd-=month_len(nm); nm++; } while(nd<=0){ nm--; nd+=month_len(nm); }
                return pair<int,int>(nm*100+nd, mins); };
            int base = parse_hm(t.startTime);
            cout<<t.id<<' '<<t.type<<'\n';
            // compute departure date at station 0 such that station s leaves on d; here we assume s=0 (query_train)
            int m=mm_d, day=dd_d; // leaving station 0 on date d at startTime
            for(int i=0;i<t.stationNum;i++){
                string arr, lea;
                if(i==0){ arr="xx-xx xx:xx"; auto md=advance(m,day,base, t.departOffset[0]); int cm=(md.first/100), cd=(md.first%100); lea=to_str(cm,cd,md.second); }
                else{
                    auto mdA=advance(m,day,base, t.arriveOffset[i]); int cm=(mdA.first/100), cd=(mdA.first%100); arr=to_str(cm,cd,mdA.second);
                    if(i==t.stationNum-1){ lea="xx-xx xx:xx"; }
                    else{
                        auto mdL=advance(m,day,base, t.departOffset[i]); int cm2=(mdL.first/100), cd2=(mdL.first%100); lea=to_str(cm2,cd2,mdL.second);
                    }
                }
                string seat;
                if(i==t.stationNum-1){ seat="x"; }
                else{
                    string key = t.id + string("#") + d;
                    auto it = seatByTrainDate.find(key);
                    int rem = t.seatNum;
                    if(it!=seatByTrainDate.end() && (int)it->second.size()==t.stationNum-1) rem = it->second[i];
                    seat = to_string(rem);
                }
                cout<<t.stations[i]<<' '<<arr<<" -> "<<lea<<' '<<t.priceCum[i]<<' '<<seat<<'\n';
            }
        } else if(cmd=="delete_train"){
            string id; string token; getline(cin, token); stringstream ss(token); string k,v; while(ss>>k>>v){ if(k=="-i") id=v; }
            if(trains.count(id)==0 || trains[id].released){ cout<<-1<<'\n'; } else { trains.erase(id); cout<<0<<'\n'; }
        } else if(cmd=="query_ticket"){
            string S,T,D,sortKey="time"; string token; getline(cin, token); stringstream ss(token); string k,v;
            while(ss>>k>>v){ if(k=="-s") S=v; else if(k=="-t") T=v; else if(k=="-d") D=v; else if(k=="-p") sortKey=v; }
            auto parse_md=[&](const string &md){ int mm=(md[0]-'0')*10+(md[1]-'0'); int dd=(md[3]-'0')*10+(md[4]-'0'); return pair<int,int>(mm,dd); };
            auto [mm_d, dd_d]=parse_md(D);
            auto month_len=[&](int m){ if(m==6) return 30; if(m==7) return 31; return 31; };
            auto to_str=[&](int m,int d,int minutes){ int hr=minutes/60; int mi=minutes%60; char buf[16]; char bufmd[6]; snprintf(bufmd, sizeof(bufmd), "%02d-%02d", m, d); snprintf(buf, sizeof(buf), "%02d:%02d", hr, mi); string res=string(bufmd)+" "+string(buf); return res; };
            auto advance_abs=[&](int m,int d,int base_minutes,int delta){ long long tot=base_minutes+delta; int day_add = (int)(tot/1440); int mins = (int)(tot%1440); if(mins<0){ mins+=1440; day_add--; }
                int nm=m, nd=d+day_add; while(nd>month_len(nm)){ nd-=month_len(nm); nm++; } while(nd<=0){ nm--; nd+=month_len(nm); }
                return tuple<int,int,int>(nm, nd, mins); };
            struct Item{ string id; string from,to; string leaveS, arriveS; int price; int seat; int timeNeeded; int si; int ti; string dateKey; };
            vector<Item> items;
            for(auto &pr: trains){ auto &t=pr.second; if(!t.released) continue; // only released trains
                int si=-1, ti=-1; for(int i=0;i<t.stationNum;i++){ if(t.stations[i]==S) si=i; if(t.stations[i]==T) ti=i; }
                if(si==-1 || ti==-1 || si>=ti) continue;
                // Find starting station departure date d0 so that station si departs on D
                int departOff = (si==0? 0 : t.departOffset[si]);
                int base = (t.startTime[0]-'0')*600 + (t.startTime[1]-'0')*60 + (t.startTime[3]-'0')*10 + (t.startTime[4]-'0');
                // compute start date nm,nd so that leave at si on D: subtract floor((base+departOff)/1440) days
                int shiftDays = (base + departOff)/1440; int nm=mm_d, nd=dd_d - shiftDays; while(nd<=0){ nm--; nd+=month_len(nm); }
                // check sale range
                auto [mm_s, dd_s]=parse_md(t.saleDate.first); auto [mm_e, dd_e]=parse_md(t.saleDate.second);
                auto cmp_md=[&](int a_m,int a_d,int b_m,int b_d){ if(a_m!=b_m) return a_m<b_m?-1:1; if(a_d!=b_d) return a_d<b_d?-1: (a_d>b_d?1:0); return 0; };
                if(cmp_md(nm,nd,mm_s,dd_s)<0 || cmp_md(nm,nd,mm_e,dd_e)>0) continue;
                auto [lm,ld,lmin]=advance_abs(nm,nd,base, t.departOffset[si]);
                auto [am,ad,amin]=advance_abs(nm,nd,base, t.arriveOffset[ti]);
                int price = t.priceCum[ti] - t.priceCum[si];
                // seat availability per segment for this train/date (start date nm,nd but we key by leave date at start)
                char bufmd[6]; snprintf(bufmd, sizeof(bufmd), "%02d-%02d", nm, nd);
                string key = t.id + string("#") + string(bufmd);
                auto &vec = seatByTrainDate[key]; if(vec.empty()) vec.assign(t.stationNum-1, t.seatNum);
                int seat = INT_MAX; for(int seg=si; seg<ti; ++seg) seat = min(seat, vec[seg]);
                int timeNeeded = (ti==si?0: (t.arriveOffset[ti]-t.departOffset[si]));
                items.push_back({t.id, S, T, to_str(lm,ld,lmin), to_str(am,ad,amin), price, seat, timeNeeded, si, ti, key});
            }
            // sort
            if(sortKey=="cost") sort(items.begin(), items.end(), [](const Item&a, const Item&b){ if(a.price!=b.price) return a.price<b.price; return a.id<b.id; });
            else sort(items.begin(), items.end(), [](const Item&a, const Item&b){ if(a.timeNeeded!=b.timeNeeded) return a.timeNeeded<b.timeNeeded; return a.id<b.id; });
            cout<<items.size()<<'\n';
            for(auto &it: items){ cout<<it.id<<' '<<it.from<<' '<<it.leaveS<<" -> "<<it.to<<' '<<it.arriveS<<' '<<it.price<<' '<<it.seat<<'\n'; }
        } else if(cmd=="buy_ticket"){
            string u,id,d,f,t; int n=0; string q="false"; string token; getline(cin, token); stringstream ss(token); string k,v;
            while(ss>>k>>v){ if(k=="-u") u=v; else if(k=="-i") id=v; else if(k=="-d") d=v; else if(k=="-f") f=v; else if(k=="-t") t=v; else if(k=="-n") n=stoi_safe(v); else if(k=="-q") q=v; }
            if(!online.count(u) || n<=0){ cout<<-1<<'\n'; continue; }
            if(trains.count(id)==0 || !trains[id].released){ cout<<-1<<'\n'; continue; }
            auto &tr=trains[id]; int si=-1, ti=-1; for(int i=0;i<tr.stationNum;i++){ if(tr.stations[i]==f) si=i; if(tr.stations[i]==t) ti=i; }
            if(si==-1 || ti==-1 || si>=ti) { cout<<-1<<'\n'; continue; }
            // compute start date key same as query_ticket
            auto parse_md=[&](const string &md){ int mm=(md[0]-'0')*10+(md[1]-'0'); int dd=(md[3]-'0')*10+(md[4]-'0'); return pair<int,int>(mm,dd); };
            auto [mm_d, dd_d]=parse_md(d);
            auto month_len=[&](int m){ if(m==6) return 30; if(m==7) return 31; return 31; };
            int base = (tr.startTime[0]-'0')*600 + (tr.startTime[1]-'0')*60 + (tr.startTime[3]-'0')*10 + (tr.startTime[4]-'0');
            int departOff = (si==0?0:tr.departOffset[si]);
            int shiftDays = (base + departOff)/1440; int nm=mm_d, nd=dd_d - shiftDays; while(nd<=0){ nm--; nd+=month_len(nm); }
            char bufmd[6]; snprintf(bufmd, sizeof(bufmd), "%02d-%02d", nm, nd);
            string key = id + string("#") + string(bufmd);
            auto &vec = seatByTrainDate[key]; if(vec.empty()) vec.assign(tr.stationNum-1, tr.seatNum);
            int seat = INT_MAX; for(int seg=si; seg<ti; ++seg) seat = min(seat, vec[seg]);
            if(seat>=n){ for(int seg=si; seg<ti; ++seg) vec[seg]-=n; // deduct
                // compute strings
                auto to_str=[&](int m,int d,int minutes){ int hr=minutes/60; int mi=minutes%60; char bf[16]; char bmd[6]; snprintf(bmd, sizeof(bmd), "%02d-%02d", m, d); snprintf(bf, sizeof(bf), "%02d:%02d", hr, mi); return string(bmd)+" "+string(bf); };
                auto advance_abs=[&](int m,int d,int base_minutes,int delta){ long long tot=base_minutes+delta; int day_add = (int)(tot/1440); int mins = (int)(tot%1440); if(mins<0){ mins+=1440; day_add--; }
                    int nm2=m, nd2=d+day_add; while(nd2>month_len(nm2)){ nd2-=month_len(nm2); nm2++; } while(nd2<=0){ nm2--; nd2+=month_len(nm2); }
                    return tuple<int,int,int>(nm2, nd2, mins); };
                auto [lm,ld,lmin]=advance_abs(nm,nd,base, tr.departOffset[si]);
                auto [am,ad,amin]=advance_abs(nm,nd,base, tr.arriveOffset[ti]);
                int price = tr.priceCum[ti]-tr.priceCum[si];
                ordersByUser[u].push_back({id,f,t,to_str(lm,ld,lmin),to_str(am,ad,amin),price,n,"success",globalTimeIdx++,string(bufmd),si,ti});
                cout<<price*n<<'\n';
            } else {
                if(q=="true" && n<=tr.seatNum){ // enter queue (not implemented fully)
                    cout<<"queue\n";
                } else cout<<-1<<'\n';
            }
        } else if(cmd=="query_order"){
            string u; string token; getline(cin, token); stringstream ss(token); string k,v; while(ss>>k>>v){ if(k=="-u") u=v; }
            if(!online.count(u)){ cout<<-1<<'\n'; continue; }
            auto &vec = ordersByUser[u]; cout<<vec.size()<<'\n';
            for(int i=(int)vec.size()-1;i>=0;--i){ auto &o=vec[i]; cout<<'['<<o.status<<"] "<<o.trainID<<' '<<o.from<<' '<<o.leaveS<<" -> "<<o.to<<' '<<o.arriveS<<' '<<o.price<<' '<<o.num<<'\n'; }
        } else if(cmd=="refund_ticket"){
            string u; int nth=1; string token; getline(cin, token); stringstream ss(token); string k,v; while(ss>>k){ if(k=="-u") { ss>>u; } else if(k=="-n") { ss>>v; nth=stoi_safe(v);} }
            if(!online.count(u)){ cout<<-1<<'\n'; continue; }
            auto &vec=ordersByUser[u]; if(nth<=0 || nth>(int)vec.size()){ cout<<-1<<'\n'; continue; }
            int idx = (int)vec.size()-nth; auto &o=vec[idx]; if(o.status=="refunded"){ cout<<-1<<'\n'; continue; }
            // restore seats
            auto &tr=trains[o.trainID]; auto &seatVec = seatByTrainDate[o.trainID+string("#")+o.dateKey];
            for(int seg=o.si; seg<o.ti; ++seg) seatVec[seg]+=o.num;
            o.status="refunded"; cout<<0<<'\n';
        } else if(cmd=="clean"){
            users.clear(); online.clear(); trains.clear(); hasFirstUser=false; cout<<0<<'\n';
        } else if(cmd=="exit"){
            online.clear(); cout<<"bye\n"; break;
        } else {
            string rest; getline(cin, rest); cout<<-1<<'\n';
        }
    }
    return 0;
}

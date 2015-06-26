#include<bits/stdc++.h>
#include"Jtol.h"
using namespace Jtol;
using namespace std;

mutex mtx;
mutex mtx_net;
mutex mtx_card;
int puts(string s){return puts(s.c_str());}
string Content(string s){
    return "Content-Length: "+IntToStr(s.length())+"\r\n\r\n"+s;
    }
string JsonGetObj(string &s,string fnd){
    size_t x=s.find(fnd),y;
    if(x==string::npos)return "";
    while(s[x+fnd.length()]!='\"'){
        y=s.substr(x+1).find(fnd);
        if(y==string::npos)return "";
        x+=y+1;
        }
    int a=0,b=0,len=s.length();
    for(int i=x+fnd.size()-1;i<len;i++)
        if(s[i]==':')a=i;
        else if(s[i]==','||s[i]=='}'){b=i;break;}
    string ret;
    if(s[a+1]=='"')
        ret=s.substr(a+2,b-a-1-2);
    else
        ret=s.substr(a+1,b-a-1);
    return ret;
    }
string JsonGetCardID(string &s,string fnd){
    size_t x=s.find(fnd);
    if(x==string::npos){
        return "";
        }
    string ss=s.substr(x-30,70);
    string ret=JsonGetObj(ss,"uniqid");
    s=s.substr(x+1);
    return ret;
    }
string key_main;
string UserID;
string uuid;
void relogin();
pair<string,string> kalisin(string path,string snd="",int dcnt=0){
    Net net=NetCreat("app.kairisei-ma.jp",80);
    while(!net){
        net=NetCreat("app.kairisei-ma.jp",80);
        }
    string str;
    str+="POST "+path+" HTTP/1.1\r\n";
    str+="User-Agent: Jtol/1.0 (Linux; U; Android 4.2.2; Droid4X-WIN Build/JDQ39E)\r\n";
    str+="Host: app.kairisei-ma.jp\r\n";
    str+="Connection: Keep-Alive\r\n";
    str+="Content-Type: application/x-www-form-urlencoded\r\n";
    str+=Content(key_main+snd);
    NetSend(net,str.c_str());
    Time tim=GetTime();
    string get;
    while(GetTime()-tim<300||get==""){
        const char *buf=NetGet(net);
        if(!buf)break;
        if(GetTime()-tim>2000)break;
        if(strlen(buf)){
            get+=buf;
            tim=GetTime();
            }
        }
    Node http=HttpDecode(get);
    if(http.Type["Transfer-Encoding"]=="chunked"){
        crope cont=http.Type["HTML"].c_str();
        http.Type["HTML"]="";
        while(cont.size()){
            string code;
            int i=0;
            for(;cont[i];i++){
                if(cont[i]=='\n'){
                    i++;
                    break;
                    }
                }
            int num;
            sscanf(cont.substr(0,i).c_str(),"%x",&num);
            http.Type["HTML"]+=cont.substr(i,num).c_str();
            cont.erase(0,i+num+2);
            }
        }
    //printf("Key: %s\n",key_main.c_str());
    if(http.Type["HTML"].length()<=1){
        cout<<"Key Error!"<<endl;
        if(dcnt>=5)
            relogin();
        return kalisin(path,snd,dcnt+1);
        }
    string nk=http.Type["HTML"].substr(0,30);
    if(nk!=key_main&&nk[0]!='{'){
        key_main=nk;
        printf("Key: %s\n",key_main.c_str());
        }
    int len=http.Type["HTML"].length(),a=-1,b=len;
    int cnt=0;
    for(int i=30;i<len;i++){
        if(http.Type["HTML"][i]=='}'){
            cnt--;
            if(!cnt)a=i;
            }
        else if(http.Type["HTML"][i]=='{'){
            if(!cnt&&~a){
                b=i;
                break;
                }
            cnt++;
            }
        }
    if(b!=len)
        return make_pair(http.Type["HTML"].substr(30,a-30+1),http.Type["HTML"].substr(b,len-b));
    else
        return make_pair(http.Type["HTML"].substr(30,a-30+1),"");
    }
string hash_token;
string device_os;
string clver;
string token;
string market;
string HomeShow(){
    string home=kalisin("/Game/HomeShow").s;
    string item=kalisin("/Game/ItemShow","{\"item_type\":0}").s;
    mtx.lock();
    puts("=====================Home Status=====================");
    puts("Name: "+JsonGetObj(home,"name"));
    puts("Level: "+JsonGetObj(home,"level")+"(NextExp:"+JsonGetObj(home,"next_lv_exp")+")");
    puts("AP: "+JsonGetObj(home,"ap")+"/"+JsonGetObj(home,"ap_max")+"("+JsonGetObj(home,"ap_next_sec")+".."+JsonGetObj(home,"ap_heal_sec")+")");
    puts("BP: "+JsonGetObj(home,"bp")+"/"+JsonGetObj(home,"bp_max")+"("+JsonGetObj(home,"bp_next_sec")+".."+JsonGetObj(home,"bp_heal_sec")+")");
    puts("Card: "+JsonGetObj(home,"card_num")+"/"+JsonGetObj(home,"card_max"));
    puts("Gold: "+JsonGetObj(home,"gold"));
    puts("Rainbow Coin:"+JsonGetObj(item,"itemid\":4000,\"num"));
    puts("Frog Coin:"+JsonGetObj(item,"itemid\":4013,\"num"));
    puts("======================================================");
    mtx.unlock();
    return home;
    }
void login(){
    printf("uuid:");
    getline(cin,uuid);
    if(uuid!=""){
        printf("hash_token:");
        getline(cin,hash_token);
        printf("token[\"\"]:");
        getline(cin,token);
        printf("os[1]:");
        getline(cin,device_os);
        if(device_os=="")device_os="1";
        printf("client version[8]:");
        getline(cin,clver);
        if(clver=="")clver="8";
        printf("market[2]:");
        getline(cin,market);
        if(market=="")market="2";
        //puts(("{\"uuid\":\""+uuid+"\",\"hash_token\":\""+hash_token+"\",\"clver\":\"3\",\"os\":"+device_os+",\"carrier\":1,\"market\":2,\"lang\":0,\"device\":\"MIT Droid4X-WIN\",\"token\":\""+token+"\"}").c_str());
        system(("curl -s -d \"{\\\"uuid\\\":\\\""+uuid+"\\\",\\\"hash_token\\\":\\\""+hash_token+"\\\",\\\"clver\\\":\\\""+clver+"\\\",\\\"os\\\":"+device_os+",\\\"carrier\\\":1,\\\"market\\\":"+market+",\\\"lang\\\":0,\\\"device\\\":\\\"MIT Droid4X-WIN\\\",\\\"token\\\":\\\""+token+"\\\"}\" -A \"Dalvik/1.6.0 (Linux; U; Android 4.2.2; Droid4X-WIN\" https://app.login.kairisei-ma.jp/Auth/login.php > Auth").c_str());
        string auth=FileToStr("Auth");
        key_main=JsonGetObj(auth,"sess_key");
        puts(("Key: "+key_main).c_str());
        }
    else{
        printf("Key:");
        cin>>key_main;
        }
    if(key_main.length()==29)key_main+='=';
    kalisin("/Game/Connect");
    string home=HomeShow();
    UserID=JsonGetObj(home,"userid");
    puts(("UserID: "+UserID).c_str());
    }
void relogin(){
    if(uuid!=""){
        system(("curl -s -d \"{\\\"uuid\\\":\\\""+uuid+"\\\",\\\"hash_token\\\":\\\""+hash_token+"\\\",\\\"clver\\\":\\\""+clver+"\\\",\\\"os\\\":"+device_os+",\\\"carrier\\\":1,\\\"market\\\":"+market+",\\\"lang\\\":0,\\\"device\\\":\\\"MIT Droid4X-WIN\\\",\\\"token\\\":\\\""+token+"\\\"}\" -A \"Dalvik/1.6.0 (Linux; U; Android 4.2.2; Droid4X-WIN\" https://app.login.kairisei-ma.jp/Auth/login.php > Auth").c_str());
        string auth=FileToStr("Auth");
        key_main=JsonGetObj(auth,"sess_key");
        puts(("Key: "+key_main).c_str());
        if(key_main.length()==29)key_main+='=';
        kalisin("/Game/Connect");
        //string home=HomeShow();
        //UserID=JsonGetObj(home,"userid");
        //puts(("UserID: "+UserID).c_str());
        }
    else{
        puts("Connect User Error!");
        exit(0);
        }
    }
pair<string,string> NetObjGet(Net net,queue<char> &Obj_Queue,int &now,string &head,string &body){
    const char *buf=NetGet(net);
    for(;buf&&*buf;buf++)
        Obj_Queue.push(*buf);
    while(!Obj_Queue.empty()){
        char x=Obj_Queue.front();
        Obj_Queue.pop();
        if(x=='}'){
            string h=head,b=body;
            head="";body="";now=0;
            return make_pair(h,b);
            }
        else if(x=='{')now=1;
        else if(x=='\r'||x==' ')continue;
        else if(now&&(x!='\n'||body!=""))body+=x;
        else if(!now&&x!='\n')head+=x;
        }
    return make_pair("","");
    }
vector<string> StringCut(string s){
    vector<string> ve;
    ve.push_back("");
    int len=s.length();
    for(int i=0;i<len;i++){
        if(s[i]==','||s[i]=='\n')ve.push_back("");
        else ve.back()+=s[i];
        }
    return ve;
    }
void SellCard(string crd_nam="20000001",int num=-1){
    string crd=kalisin("/Game/CardShow").s;
    Sleep(2000);
    string ss=JsonGetCardID(crd,"\"cardid\":"+crd_nam);
    vector<string>chacha;
    int ccc=0;
    while(ss!=""){
        chacha.push_back(ss);
        ccc++;
        if(ccc==num)break;
        ss=JsonGetCardID(crd,"\"cardid\":"+crd_nam);
        }
    int chnt=0;
    string chs;
    for(auto z:chacha){
        if(chnt)chs+=',';
        chs+=z;
        chnt++;
        if(chnt==10){
            chnt=0;
            puts(("  {\"uniqids\":["+chs+"]}").c_str());
            kalisin("/Game/CardSell","{\"uniqids\":["+chs+"]}");
            Sleep(3000);
            chs="";
            }
        }
    if(chnt){
        chnt=0;
        puts(("  {\"uniqids\":["+chs+"]}").c_str());
        kalisin("/Game/CardSell","{\"uniqids\":["+chs+"]}");
        Sleep(3000);
        chs="";
        }
    }
void SellCardLess(string crd_nam="20000001",int num=0,string crd=""){
    if(crd=="")crd=kalisin("/Game/CardShow").s;
    Sleep(500);
    string ss=JsonGetCardID(crd,"\"cardid\":"+crd_nam);
    vector<string>chacha;
    int ccc=0;
    while(ss!=""){
        chacha.push_back(ss);
        ccc++;
        ss=JsonGetCardID(crd,"\"cardid\":"+crd_nam);
        }
    int chnt=0;
    string chs;
    int sz=chacha.size();
    for(int i=max(num,0);i<sz;i++){
        string z=chacha[i];
        if(chnt)chs+=',';
        chs+=z;
        chnt++;
        if(chnt==10){
            chnt=0;
            puts(("  {\"uniqids\":["+chs+"]}").c_str());
            kalisin("/Game/CardSell","{\"uniqids\":["+chs+"]}");
            Sleep(500);
            chs="";
            }
        }
    if(chnt){
        chnt=0;
        puts(("  {\"uniqids\":["+chs+"]}").c_str());
        kalisin("/Game/CardSell","{\"uniqids\":["+chs+"]}");
        Sleep(500);
        chs="";
        }
    }
void AutoSellCard(){
    mtx_card.lock();
    puts("Selling something~");
    string crd=kalisin("/Game/CardShow").s;

    SellCardLess("10000153",2,crd);
    SellCardLess("10118019",7,crd);

    SellCardLess("10000061",7,crd);
    SellCardLess("10000185",2,crd);
    SellCardLess("10118017",7,crd);

    SellCardLess("10001023",7,crd);
    SellCardLess("10000125",7,crd);
    SellCardLess("10118015",7,crd);

    SellCardLess("20000001",0,crd);
    SellCardLess("20000002",5,crd);
    SellCardLess("20000026",0,crd);
    mtx_card.unlock();
    }
void ChangeName(string s);   
string deck_arthur_type,deck_arthur_type_idx,bossid,pass;
int AutoChaCha;
string ACC;
string gocts;
bool FinishAllThread=0;
void OnlineBattleOther(int thread_id=0){
    queue<char> Obj_Queue;
    int now;
    string head;
    string body;
    int goct=-1;
    if(gocts!="")goct=StrToInt(gocts);
    if(AutoChaCha)AutoSellCard();
    for(int gocti=0;gocti!=goct;gocti++){
        mtx_net.lock();
        string teamlist=kalisin("/Game/TeamBattleMultiRoomSearch","{\"deck_arthur_type\":"+deck_arthur_type+",\"deck_arthur_type_idx\":"+deck_arthur_type_idx+",\"bossid\":"+(pass==""?bossid:(string)"0")+",\"pass\":\""+pass+"\"}").s;
        string RoomID=JsonGetObj(teamlist,"roomid");
        if(RoomID==""){
            /*if(notfnd>=10&&(notfnd>=30||pass=="")){
                //relogin();
                notfnd=0;
                }
            notfnd++;*/
            mtx_net.unlock();
            puts("Not Found");
            continue;
            }
        printf("RoomID:%s\n",RoomID.c_str());
        string team=kalisin("/Game/TeamBattleMultiRoomEnter","{\"roomid\":"+RoomID+",\"deck_arthur_type\":"+deck_arthur_type+",\"deck_arthur_type_idx\":"+deck_arthur_type_idx+"}").s;
        if(team=="{}"||team==""){
            mtx_net.unlock();
            puts("Too Slow(Or Bag Is Full)");
            continue;
            }
        else{
            string host=JsonGetObj(team,"host"),port_s=JsonGetObj(team,"port"),auth_token=JsonGetObj(team,"auth_token"),signature=JsonGetObj(team,"signature");
            printf("host:%s port:%s\n%s\n",host.c_str(),port_s.c_str(),auth_token.c_str());
            int port=StrToInt(port_s);
            Net net=NetCreat(host.c_str(),port);
            if(!net){
                mtx_net.unlock();
                puts("Net Error!");
                continue;
                }
            while(!Obj_Queue.empty())Obj_Queue.pop();
            NetObjGet(net,Obj_Queue,now,head,body);
            NetSend(net,("RoomEnterRequest{\n"+UserID+","+RoomID+","+deck_arthur_type+","+pass+","+auth_token+","+signature+"\n}\n").c_str());
            mtx_net.unlock();
            Time ti=GetTime(),gist=ti,lt=ti,sever_gt=ti,ltc=0;
            int Win=0;
            string target="5";
            int okgo=0;
            int rnd=1,tur=0,auto_go=1;
            int member=0;
            Time RERRT=0x7fffffffffffffffLL;
            Time TCT=0x7fffffffffffffffLL;
            string TCS;
            int mecnt=0;
            Time mvt=0x7fffffffffffffffLL,pongt=0x7fffffffffffffffLL;
            bool wtpong=0,brek=0;
            pair<int,int> crid[5][11];
            while(1){
                Time gt=GetTime();
                if(gt-ti>5000){
                    ti=GetTime();
                    NetSend(net,"Ping{\n}\n");
                    if(!wtpong)
                        pongt=gt+30000,wtpong=1;
                    }
                if(gt>=pongt)break;
                if(gt-gist>60000&&!okgo){
                    puts("Wait Too Long!");
                    break;
                    }
                if(gt-sever_gt>60000&&!okgo){
                    puts("Wait Too Long!");
                    break;
                    }
                pair<string,string> Obj=NetObjGet(net,Obj_Queue,now,head,body);;
                if(Obj.f!=""&&Obj.f.find("Pong")==string::npos){
                    mtx.lock();
                    puts((Obj.f+" "+IntToStr(thread_id)).c_str());
                    lt=gt,sever_gt=gt;
                    fprintf(stderr,"%s %d\n%s\n",Obj.f.c_str(),thread_id,Obj.s.c_str());
                    mtx.unlock();
                    }
                if(Obj.f.find("Pong")==string::npos)
                    lt=gt;
                if(GetTime()-lt>30000){
                    puts("No Respond!");
                    break;
                    }
                if(gt>=RERRT){
                    puts(("RoomLoadingFinish "+IntToStr(thread_id)).c_str());
                    NetSend(net,"RoomLoadingFinish{\n}\n");
                    NetSend(net,"RoomLoadingFinish{\n}\n");
                    RERRT=0x7fffffffffffffffLL;
                    }
                if(gt>=TCT){
                    puts(("CardPlay "+IntToStr(thread_id)+" {"+TCS+"}").c_str());
                    NetSend(net,"CardPlay{\n"+TCS+"\n}\n");
                    TCT=0x7fffffffffffffffLL;
                    }
                bool nomsg=0;
                if(Obj.f.find("RoomEnterRequestResult")!=string::npos){
                    printf("  %s",Obj.s.c_str());
                    if(Obj.s.find("OK")==string::npos){
                        puts("  No OK");
                        brek=1;
                        break;
                        }
                    RERRT=GetTime()+4000;
                    }
                else if(Obj.f.find("RoomDelete")!=string::npos){
                    puts("Break");
                    break;
                    }
                else if(Obj.f.find("RoomMember")!=string::npos){
                    gist=gt;
                    printf("  %s",Obj.s.c_str());
                    vector<string> ve=StringCut(Obj.s);
                    if(StrToInt(ve[1])<0)
                        member&=(15^(1<<(StrToInt(ve[0])-1)));
                    else{
                        member|=(1<<(StrToInt(ve[0])-1));
                        }
                    mecnt++;
                    if(mecnt>4){
                        puts(("RoomLoadingFinish "+IntToStr(thread_id)).c_str());
                        NetSend(net,"RoomLoadingFinish{\n}\n");
                        }
                    }
                else if(Obj.f.find("RoomCountdownFinish")!=string::npos){
                    okgo=1;
                    Sleep(2000);
                    puts(("LoadingFinish "+IntToStr(thread_id)).c_str());
                    NetSend(net,"LoadingFinish{\n}\n");
                    Sleep(1000);
                    RERRT=0x7fffffffffffffffLL;
                    }
                else if(Obj.f.find("ApiGameStart")!=string::npos){
                    vector<string> ve=StringCut(Obj.s);
                    int sz=ve.size();
                    for(int i=0;i<sz;i++){
                        if(ve[i]=="999")i+=2;
                        else if(ve[i]=="23"){
                            crid[StrToInt(ve[i+1])][StrToInt(ve[i+2])]=make_pair(StrToInt(ve[i+3]),StrToInt(ve[i+4]));
                            i+=4;
                            }
                        }
                    puts(("GameStartFinish "+IntToStr(thread_id)).c_str());
                    Sleep(500);
                    NetSend(net,"GameStartFinish{\n}\n");
                    }
                else if(Obj.f.find("ApiTurnPhase")!=string::npos){
                    tur++;
                    printf("  R:%d T:%d C:%d\n",rnd,tur,min(tur+2,10));
                    puts(("TurnPhaseFinish "+IntToStr(thread_id)).c_str());
                    Sleep(500);
                    NetSend(net,"TurnPhaseFinish{\n}\n");
                    }
                else if(Obj.f.find("GameNextStart")!=string::npos){
                    puts(("GameNextFinish "+IntToStr(thread_id)).c_str());
                    target="5";
                    Sleep(500);
                    rnd++;
                    tur=0;
                    NetSend(net,"GameNextFinish{\n}\n");
                    }
                else if(Obj.f.find("MemberChat")!=string::npos){
                    gist=GetTime();
                    if(rand()&1&&ti-ltc>2000){
                        vector<string> ve=StringCut(Obj.s);
                        Sleep(1000);
                        puts(("Say "+ve[0]+"!").c_str());
                        NetSend(net,"Chat{\n"+ve[0]+"\n}\n");
                        ltc=ti;
                        }
                    }
                else if(Obj.f.find("ApiUserAttack")!=string::npos){
                    //int addtime=0;
                    /*vector<string> ve=StringCut(Obj.s);
                    int sz=ve.size();
                    for(int i=0;i<sz;i++){
                        if(ve[i]=="50"){
                            printf("  Boss%d Do Something(%d)\n",StrToInt(ve[i+1]),StrToInt(ve[i+2]));
                            i+=6;
                            }
                        else if(ve[i]=="6"){
                            //printf("  %d Status C\n",StrToInt(ve[i+1]),StrToInt(ve[i+2]));
                            i+=8;
                            }
                        else if(ve[i]=="62"){
                            printf("  %d Get Some Buff\n",StrToInt(ve[i+1]),StrToInt(ve[i+2]));
                            i+=10;
                            }
                        }*/
                    //BossT  50 BossID ......
                    //Throw  51 PlayerType CardPlace CardID CardTo CardLv
                    //DisHP  60 PlayerID ? DisHP LessHP ? ? ?
                    //HealHP 61 PlayerID ? HealHP LessHP
                    //Status 6 PlayerID HP HPMax Atk Int Mnd
                    //Buff 62
                    //uat=GetTime();
                    Sleep(5000);
                    puts(("UserAttackFinish "+IntToStr(thread_id)).c_str());
                    NetSend(net,"UserAttackFinish{\n}\n");
                    }
                else if(Obj.f.find("ApiEnemyPhase")!=string::npos){
                    //BossT  50 BossID ......
                    //Throw  51 PlayerType CardPlace CardID CardTo CardLv
                    //DisHP  60 PlayerID ? DisHP LessHP ? ? ?
                    //HealHP 61 PlayerID ? HealHP LessHP
                    //Status 6 PlayerID HP HPMax Atk Int Mnd
                    //Buff 62
                    //ept=GetTime();
                    vector<string> ve=StringCut(Obj.s);
                    int sz=ve.size(),ok70=0,oao=0;
                    for(int i=0;i<sz;i++)
                        if(ve[i]=="60")i+=8;
                        else if(ve[i]=="50")i+=6;
                        else if(ve[i]=="61")i+=4;
                        else if(ve[i]=="3")i+=4;
                        else if(ve[i]=="62")i+=10;
                        else if(ve[i]=="70")ok70=1;
                        else if(ve[i]=="71")i+=3;
                        else if(ve[i]=="72")i+=4;
                        else if(ve[i]=="6"){
                            if(StrToInt(ve[i+1])>8||StrToInt(ve[i+1])<=0)
                                oao=1;
                            if(ok70)
                                printf("  %s HP:%8d/%-8d Atk:%-7d Int:%-7d Mnd:%-7d\n",ve[i+1].c_str(),StrToInt(ve[i+2]),StrToInt(ve[i+3]),StrToInt(ve[i+4]),StrToInt(ve[i+5]),StrToInt(ve[i+6]));
                            i+=8;
                            }
                    if(oao)puts(Obj.s.c_str());
                    Sleep(3000);
                    puts(("EnemyPhaseFinish "+IntToStr(thread_id)).c_str());
                    NetSend(net,"EnemyPhaseFinish{\n}\n");
                    }
                else if(Obj.f.find("ApiUserPhase")!=string::npos){
                    //printf(Obj.s.c_str());
                    vector<string> ve=StringCut(Obj.s);
                    int sz=ve.size();
                    string ar[5]={"0","0","0","0","0"},arr[5]={"0","0","0","0","0"};
                    int cnt=0;
                    //string gd="0";
                    target="5";
                    pair<int,int>cdc[15];
                    bool loc[15]={0};
                    int cnt29=0,cnt25=0;
                    for(int i=0;i<sz;i++){
                        if(ve[i]=="20")i+=3;
                        else if(ve[i]=="21")i+=3;
                        else if(ve[i]=="24")i+=6;
                        else if(ve[i]=="29"){
                            if(ve[i+1]==deck_arthur_type){
                                if(StrToInt(ve[i+3])){
                                    //cdc[cnt29].s=99;
                                    loc[cnt29]=1;
                                    printf("  Card%02d Locked! Less %d Round.\n",cdc[cnt29].f,StrToInt(ve[i+4]));
                                    }
                                cnt29++;
                                i+=4;
                                }
                            }
                        else if(ve[i]=="25"){
                            if(ve[i+1]==deck_arthur_type){
                                printf("  Card%02d Value:%-6d Cost:%-2d\n",StrToInt(ve[i+2]),StrToInt(ve[i+8]),cdc[cnt25].s);
                                cnt25++;
                                }
                            i+=8;
                            }
                        else if(ve[i]=="27"){
                            if(ve[i+1]==deck_arthur_type){
                                cdc[cnt]=make_pair(StrToInt(ve[i+2]),StrToInt(ve[i+3]));
                                cnt++;
                                }
                            i+=3;
                            continue;
                            }
                        else if(ve[i]=="13"){
                            if(StrToInt(ve[i+2]))
                                target=max(target,IntToStr(StrToInt(ve[i+1])));
                            i+=2;
                            }
                        }
                    //write AI here start
                    int nowcos=min(tur+2,10);
                    for(int i=0;i<min(cnt,5);i++){
                        if(cdc[i].s<=nowcos&&!loc[i]){
                            ar[i]=IntToStr(cdc[i].f);
                            arr[i]=target;
                            nowcos-=cdc[i].s;
                            }
                        }
                    //write AI here end
                    if(auto_go){
                        TCS="";
                        for(int i=0;i<5;i++){
                            if(i)TCS+=",";
                            TCS+=ar[i]+","+arr[i];
                            }
                        puts(("  Target:"+target).c_str());
                        target="5";
                        TCT=GetTime()+rand()%6000+4000;
                        }
                    else{
                        //Not Finish
                        while(1){
                            break;
                            }
                        string ss;
                        for(int i=0;i<5;i++){
                            if(i)ss+=",";
                            ss+=ar[i]+","+arr[i];
                            }
                        puts(("CardPlay "+IntToStr(thread_id)+" {"+ss+"}").c_str());
                        NetSend(net,"CardPlay{\n"+ss+"\n}\n");
                        }
                    }
                else if(Obj.f.find("MemberOver")!=string::npos);
                else if(Obj.f.find("GameEnd")!=string::npos){
                    if(Obj.s.find("1")!=string::npos)
                        Win=1;
                    break;
                    }
                else if(Obj.f.find("ApiCardPlayR")!=string::npos){
                    vector<string> ve=StringCut(Obj.s);
                    int sz=ve.size();
                    for(int i=0;i<sz;i++){
                        if(ve[i]=="22"){
                            printf("  %d Throw %02d(%d Lv.%d) To %d\n",StrToInt(ve[i+1]),StrToInt(ve[i+2]),crid[StrToInt(ve[i+1])][StrToInt(ve[i+2])].f,crid[StrToInt(ve[i+1])][StrToInt(ve[i+2])].s,StrToInt(ve[i+3]));
                            i+=3;
                            }
                        else if(ve[i]=="25")i+=8;
                        }
                    }
                else if(Obj.f.find("RoomCountdownStart")!=string::npos)
                    okgo=1;
                else if(Obj.f.find("Pong")!=string::npos){
                    wtpong=0;
                    pongt=0x7fffffffffffffffLL;
                    }
                else if(Obj.f.find("ApiContinuePhaseStart")!=string::npos){
                    break;
                    Sleep(1000);
                    NetSend(net,"ContinuePhaseFinish{\n}\n");
                    }
                else if(Obj.f.find("ApiContinue")!=string::npos){
                    break;
                    Sleep(1000);
                    NetSend(net,"ContinueFinish{\n}\n");
                    }
                else if(Obj.f.find("ApiGameOver")!=string::npos){
                    Sleep(1000);
                    NetSend(net,"GameOverPhaseFinish{\n}\n");
                    }
                else if(Obj.f!=""){
                    puts(Obj.s.c_str());
                    }
                else{
                    nomsg=1;
                    }
                if(!nomsg){
                    mvt=GetTime()+300000;
                    }
                if(GetTime()>=mvt)break;
                Sleep(1);
                }
            NetClose(net);
            if(brek)continue;
            if(Win){
                puts(("Win "+IntToStr(thread_id)).c_str());
                kalisin("/Game/TeamBattleResult","{\"roomid\":"+RoomID+"}");
                Sleep(2000);
                if(AutoChaCha)
                    AutoSellCard();
                }
            string home=HomeShow();
            if(StrToInt(JsonGetObj(home,"card_num"))>=StrToInt(JsonGetObj(home,"card_max"))){
                puts("pack is full~");
                break;
                }
            int nmcl[6];
            for(int i=0;i<6;i++) 
                nmcl[i]=(rand()/100)%16;
            string new_name="[";
            for(int i=0;i<6;i++){
                if(nmcl[i]>=0&&nmcl[i]<=9)new_name+=nmcl[i]+'0';
                else new_name+=nmcl[i]-10+'A';    
                }
            new_name+="]SQUARE ENIX";
            //new_name+="]SE";
            ChangeName(new_name);
            //kalisin("/Game/HomeShow");
            //Sleep(1000);
            }
        puts("Press E to exit battle");
        if(ChkKey('E'))break;
        if(FinishAllThread)break;
        }
    FinishAllThread=1;
    }
void OnlineBattleSelf(int thread_id=0){//Maybe Full Of Bugs

    queue<char> Obj_Queue;
    int now;
    string head;
    string body;

    string deck_arthur_type,deck_arthur_type_idx,bossid,pass,room_type,is_need_deck_rank,deck_rank;
    printf("deck_arthur_type:");
    cin>>deck_arthur_type;
    printf("deck_arthur_type_idx:");
    cin>>deck_arthur_type_idx;
    printf("bossid:");
    cin>>bossid;
    getline(cin,pass);
    printf("pass[\"\"]:");
    getline(cin,pass);
    printf("room_type(only friend?)[0]:");
    getline(cin,room_type);if(room_type=="")room_type="0";
    printf("is_need_deck_rank[0]:");
    getline(cin,is_need_deck_rank);
    if(is_need_deck_rank=="")is_need_deck_rank="0",deck_rank="0";
    if(is_need_deck_rank=="1"){
        printf("deck_rank(1=F+~13=SS)[12]:");
        getline(cin,deck_rank);
        if(deck_rank=="")deck_rank="12";
        }
    else
        deck_rank=="0";
        
    string mf;
    //int min_fame=0;
    printf("min fame[0]:");
    getline(cin,mf);if(mf!="")mf="0";//min_fame=StrToInt(mf);
    
    string mh;
    //int min_hp=0;
    printf("min hp[0]:");
    getline(cin,mh);if(mh!="")mh="0";//min_hp=StrToInt(mh);
    
    string comment;
    printf("comment[""]:");
    getline(cin,comment);
    
    printf("AutoSellBlueQieQie[1]:");
    int AutoChaCha=1;
    string ACC;
    getline(cin,ACC);
    if(ACC!="")AutoChaCha=StrToInt(ACC);
    printf("repeat times[-1]:");
    string gocts;
    int goct=-1;
    getline(cin,gocts);if(gocts!="")goct=StrToInt(gocts);
    if(AutoChaCha)AutoSellCard();
    for(int gocti=0;gocti!=goct;gocti++){
        mtx_net.lock();
        string RoomID;
        string team=kalisin("/Game/TeamBattleMultiRoomCreate","{\"bossid\":"+bossid+",\"deck_arthur_type\":"+deck_arthur_type+",\"deck_arthur_type_idx\":"+deck_arthur_type_idx+",\"room_type\":"+room_type+",\"pass\":\""+pass+"\",\"is_need_deck_rank\":"+is_need_deck_rank+",\"deck_rank\":"+deck_rank+",\"fame\":"+mf+",\"hp\":"+mh+",\"comment\":\""+comment+"\"}").s;
        //puts(("{\"bossid\":"+bossid+",\"deck_arthur_type\":"+deck_arthur_type+",\"deck_arthur_type_idx\":"+deck_arthur_type_idx+",\"room_type\":"+room_type+",\"pass\":\""+pass+"\",\"is_need_deck_rank\":"+is_need_deck_rank+",\"deck_rank\":"+deck_rank+"}").c_str());
        //puts(team.c_str());
        if(team=="{}"||team==""){
            puts("Error(Maybe no pt)!");
            mtx_net.unlock();
            continue;
            }
        else{
            string host=JsonGetObj(team,"host"),port_s=JsonGetObj(team,"port"),auth_token=JsonGetObj(team,"auth_token"),signature=JsonGetObj(team,"signature");
            printf("host:%s port:%s\n%s\n",host.c_str(),port_s.c_str(),auth_token.c_str());
            int port=StrToInt(port_s);
            Net net=NetCreat(host.c_str(),port);
            if(!net){
                mtx_net.unlock();
                puts("Net Error!");
                continue;
                }
            while(!Obj_Queue.empty())Obj_Queue.pop();
            NetObjGet(net,Obj_Queue,now,head,body);
            NetSend(net,("RoomCreateRequest{\n"+UserID+","+bossid+","+room_type+","+pass+","+is_need_deck_rank+","+deck_rank+","+auth_token+","+signature+"\n}\n").c_str());
            mtx_net.unlock();
            Time ti=GetTime(),gist=ti,lt=ti,sever_gt=ti,ltc=0,ready=-1;
            int Win=0;
            string target="5";
            int okgo=0;
            int rnd=1,tur=0,auto_go=1,gone=0,went=0;
            int member=0;
            Time RERRT=0x7fffffffffffffffLL;
            Time TCT=0x7fffffffffffffffLL;
            string TCS;
            int mecnt=0;
            Time mvt=0x7fffffffffffffffLL,pongt=0x7fffffffffffffffLL;
            bool wtpong=0,brek=0;
            pair<int,int> crid[5][11];
            while(1){
                Time gt=GetTime();
                if(gt-ti>5000){
                    ti=GetTime();
                    NetSend(net,"Ping{\n}\n");
                    if(!wtpong)
                        pongt=gt+30000,wtpong=1;
                    }
                if(gt>=pongt)break;
                if(gt-gist>60000&&!okgo){
                    puts("Wait Too Long!");
                    break;
                    }
                if(gt-sever_gt>60000&&!okgo){
                    puts("Wait Too Long!");
                    break;
                    }
                pair<string,string> Obj=NetObjGet(net,Obj_Queue,now,head,body);;
                if(Obj.f!=""&&Obj.f.find("Pong")==string::npos){
                    mtx.lock();
                    puts((Obj.f+" "+IntToStr(thread_id)).c_str());
                    lt=gt,sever_gt=gt;
                    fprintf(stderr,"%s %d\n%s\n",Obj.f.c_str(),thread_id,Obj.s.c_str());
                    mtx.unlock();
                    }
                if(Obj.f.find("Pong")==string::npos)
                    lt=gt;
                if(GetTime()-lt>30000){
                    puts("No Respond!");
                    break;
                    }
                if(gt>=RERRT){
                    puts(("RoomLoadingFinish "+IntToStr(thread_id)).c_str());
                    NetSend(net,"RoomLoadingFinish{\n}\n");
                    NetSend(net,"RoomLoadingFinish{\n}\n");
                    RERRT=0x7fffffffffffffffLL;
                    }
                if(gt>=TCT){
                    puts(("CardPlay "+IntToStr(thread_id)+" {"+TCS+"}").c_str());
                    NetSend(net,"CardPlay{\n"+TCS+"\n}\n");
                    TCT=0x7fffffffffffffffLL;
                    }
                if(member==15&&~ready&&gt-ready>12000&&!gone){
                    puts("Go!");
                    NetSend(net,"RoomCountdownStartRequest{\n}\n");
                    gone=1;
                    }
                else if(member==15&&~ready&&gt-ready>10000&&!gone&&!went){
                    went=1;
                    }
                else if(member==15&&!~ready){
                    ready=gt;
                    puts("Ready...");
                    }
                else if(member!=15)
                    ready=-1;
                    
                bool nomsg=0;
                if(Obj.f.find("RoomCreateRequestResult")!=string::npos){
                    printf("  %s",Obj.s.c_str());
                    if(Obj.s.find("OK")==string::npos){
                        puts("No OK");
                        break;
                        }
                    vector<string> ve=StringCut(Obj.s);
                    RoomID=ve[2];
                    puts(("RoomID:"+RoomID).c_str());
                    Sleep(1000);
                    puts("RoomLoadingFinish");
                    NetSend(net,"RoomLoadingFinish{\n}\n");
                    }
                else if(Obj.f.find("RoomDelete")!=string::npos){
                    puts("Break");
                    break;
                    }
                else if(Obj.f.find("RoomMember")!=string::npos){
                    gist=gt;
                    printf("  %s",Obj.s.c_str());
                    vector<string> ve=StringCut(Obj.s);
                    if(StrToInt(ve[1])<0)
                        member&=(15^(1<<(StrToInt(ve[0])-1)));
                    else{
                        member|=(1<<(StrToInt(ve[0])-1));
                        }
                    mecnt++;
                    if(mecnt>4){
                        puts(("RoomLoadingFinish "+IntToStr(thread_id)).c_str());
                        NetSend(net,"RoomLoadingFinish{\n}\n");
                        }
                    }
                else if(Obj.f.find("RoomCountdownFinish")!=string::npos){
                    okgo=1;
                    Sleep(2000);
                    puts(("LoadingFinish "+IntToStr(thread_id)).c_str());
                    NetSend(net,"LoadingFinish{\n}\n");
                    Sleep(1000);
                    RERRT=0x7fffffffffffffffLL;
                    }
                else if(Obj.f.find("ApiGameStart")!=string::npos){
                    vector<string> ve=StringCut(Obj.s);
                    int sz=ve.size();
                    for(int i=0;i<sz;i++){
                        if(ve[i]=="999")i+=2;
                        else if(ve[i]=="23"){
                            crid[StrToInt(ve[i+1])][StrToInt(ve[i+2])]=make_pair(StrToInt(ve[i+3]),StrToInt(ve[i+4]));
                            i+=4;
                            }
                        }
                    puts(("GameStartFinish "+IntToStr(thread_id)).c_str());
                    Sleep(500);
                    NetSend(net,"GameStartFinish{\n}\n");
                    }
                else if(Obj.f.find("ApiTurnPhase")!=string::npos){
                    tur++;
                    printf("  R:%d T:%d C:%d\n",rnd,tur,min(tur+2,10));
                    puts(("TurnPhaseFinish "+IntToStr(thread_id)).c_str());
                    Sleep(500);
                    NetSend(net,"TurnPhaseFinish{\n}\n");
                    }
                else if(Obj.f.find("GameNextStart")!=string::npos){
                    puts(("GameNextFinish "+IntToStr(thread_id)).c_str());
                    target="5";
                    Sleep(500);
                    rnd++;
                    tur=0;
                    NetSend(net,"GameNextFinish{\n}\n");
                    }
                else if(Obj.f.find("MemberChat")!=string::npos){
                    gist=GetTime();
                    if(rand()&1&&ti-ltc>2000){
                        vector<string> ve=StringCut(Obj.s);
                        Sleep(1000);
                        puts(("Say "+ve[0]+"!").c_str());
                        NetSend(net,"Chat{\n"+ve[0]+"\n}\n");
                        ltc=ti;
                        }
                    }
                else if(Obj.f.find("ApiUserAttack")!=string::npos){
                    //int addtime=0;
                    /*vector<string> ve=StringCut(Obj.s);
                    int sz=ve.size();
                    for(int i=0;i<sz;i++){
                        if(ve[i]=="50"){
                            printf("  Boss%d Do Something(%d)\n",StrToInt(ve[i+1]),StrToInt(ve[i+2]));
                            i+=6;
                            }
                        else if(ve[i]=="6"){
                            //printf("  %d Status C\n",StrToInt(ve[i+1]),StrToInt(ve[i+2]));
                            i+=8;
                            }
                        else if(ve[i]=="62"){
                            printf("  %d Get Some Buff\n",StrToInt(ve[i+1]),StrToInt(ve[i+2]));
                            i+=10;
                            }
                        }*/
                    //BossT  50 BossID ......
                    //Throw  51 PlayerType CardPlace CardID CardTo CardLv
                    //DisHP  60 PlayerID ? DisHP LessHP ? ? ?
                    //HealHP 61 PlayerID ? HealHP LessHP
                    //Status 6 PlayerID HP HPMax Atk Int Mnd
                    //Buff 62
                    //uat=GetTime();
                    Sleep(5000);
                    puts(("UserAttackFinish "+IntToStr(thread_id)).c_str());
                    NetSend(net,"UserAttackFinish{\n}\n");
                    }
                else if(Obj.f.find("ApiEnemyPhase")!=string::npos){
                    //BossT  50 BossID ......
                    //Throw  51 PlayerType CardPlace CardID CardTo CardLv
                    //DisHP  60 PlayerID ? DisHP LessHP ? ? ?
                    //HealHP 61 PlayerID ? HealHP LessHP
                    //Status 6 PlayerID HP HPMax Atk Int Mnd
                    //Buff 62
                    //ept=GetTime();
                    vector<string> ve=StringCut(Obj.s);
                    int sz=ve.size(),ok70=0,oao=0;
                    for(int i=0;i<sz;i++)
                        if(ve[i]=="60")i+=8;
                        else if(ve[i]=="50")i+=6;
                        else if(ve[i]=="61")i+=4;
                        else if(ve[i]=="3")i+=4;
                        else if(ve[i]=="62")i+=10;
                        else if(ve[i]=="70")ok70=1;
                        else if(ve[i]=="71")i+=3;
                        else if(ve[i]=="72")i+=4;
                        else if(ve[i]=="6"){
                            if(StrToInt(ve[i+1])>8||StrToInt(ve[i+1])<=0)
                                oao=1;
                            if(ok70)
                                printf("  %s HP:%8d/%-8d Atk:%-7d Int:%-7d Mnd:%-7d\n",ve[i+1].c_str(),StrToInt(ve[i+2]),StrToInt(ve[i+3]),StrToInt(ve[i+4]),StrToInt(ve[i+5]),StrToInt(ve[i+6]));
                            i+=8;
                            }
                    if(oao)puts(Obj.s.c_str());
                    Sleep(3000);
                    puts(("EnemyPhaseFinish "+IntToStr(thread_id)).c_str());
                    NetSend(net,"EnemyPhaseFinish{\n}\n");
                    }
                else if(Obj.f.find("ApiUserPhase")!=string::npos){
                    //printf(Obj.s.c_str());
                    vector<string> ve=StringCut(Obj.s);
                    int sz=ve.size();
                    string ar[5]={"0","0","0","0","0"},arr[5]={"0","0","0","0","0"};
                    int cnt=0;
                    //string gd="0";
                    target="5";
                    pair<int,int>cdc[15];
                    bool loc[15]={0};
                    int cnt29=0,cnt25=0;
                    for(int i=0;i<sz;i++){
                        if(ve[i]=="20")i+=3;
                        else if(ve[i]=="21")i+=3;
                        else if(ve[i]=="24")i+=6;
                        else if(ve[i]=="29"){
                            if(ve[i+1]==deck_arthur_type){
                                if(StrToInt(ve[i+3])){
                                    //cdc[cnt29].s=99;
                                    loc[cnt29]=1;
                                    printf("  Card%02d Locked! Less %d Round.\n",cdc[cnt29].f,StrToInt(ve[i+4]));
                                    }
                                cnt29++;
                                i+=4;
                                }
                            }
                        else if(ve[i]=="25"){
                            if(ve[i+1]==deck_arthur_type){
                                printf("  Card%02d Value:%-6d Cost:%-2d\n",StrToInt(ve[i+2]),StrToInt(ve[i+8]),cdc[cnt25].s);
                                cnt25++;
                                }
                            i+=8;
                            }
                        else if(ve[i]=="27"){
                            if(ve[i+1]==deck_arthur_type){
                                cdc[cnt]=make_pair(StrToInt(ve[i+2]),StrToInt(ve[i+3]));
                                cnt++;
                                }
                            i+=3;
                            continue;
                            }
                        else if(ve[i]=="13"){
                            if(StrToInt(ve[i+2]))
                                target=max(target,IntToStr(StrToInt(ve[i+1])));
                            i+=2;
                            }
                        }
                    //write AI here start
                    int nowcos=min(tur+2,10);
                    for(int i=0;i<min(cnt,5);i++){
                        if(cdc[i].s<=nowcos&&!loc[i]){
                            ar[i]=IntToStr(cdc[i].f);
                            arr[i]=target;
                            nowcos-=cdc[i].s;
                            }
                        }
                    //write AI here end
                    if(auto_go){
                        TCS="";
                        for(int i=0;i<5;i++){
                            if(i)TCS+=",";
                            TCS+=ar[i]+","+arr[i];
                            }
                        puts(("  Target:"+target).c_str());
                        target="5";
                        TCT=GetTime()+rand()%6000+4000;
                        }
                    else{
                        //Not Finish
                        while(1){
                            break;
                            }
                        string ss;
                        for(int i=0;i<5;i++){
                            if(i)ss+=",";
                            ss+=ar[i]+","+arr[i];
                            }
                        puts(("CardPlay "+IntToStr(thread_id)+" {"+ss+"}").c_str());
                        NetSend(net,"CardPlay{\n"+ss+"\n}\n");
                        }
                    }
                else if(Obj.f.find("MemberOver")!=string::npos);
                else if(Obj.f.find("GameEnd")!=string::npos){
                    if(Obj.s.find("1")!=string::npos)
                        Win=1;
                    break;
                    }
                else if(Obj.f.find("ApiCardPlayR")!=string::npos){
                    vector<string> ve=StringCut(Obj.s);
                    int sz=ve.size();
                    for(int i=0;i<sz;i++){
                        if(ve[i]=="22"){
                            printf("  %d Throw %02d(%d Lv.%d) To %d\n",StrToInt(ve[i+1]),StrToInt(ve[i+2]),crid[StrToInt(ve[i+1])][StrToInt(ve[i+2])].f,crid[StrToInt(ve[i+1])][StrToInt(ve[i+2])].s,StrToInt(ve[i+3]));
                            i+=3;
                            }
                        else if(ve[i]=="25")i+=8;
                        }
                    }
                else if(Obj.f.find("RoomCountdownStart")!=string::npos)
                    okgo=1;
                else if(Obj.f.find("Pong")!=string::npos){
                    wtpong=0;
                    pongt=0x7fffffffffffffffLL;
                    }
                else if(Obj.f.find("ApiContinuePhaseStart")!=string::npos){
                    break;
                    Sleep(1000);
                    NetSend(net,"ContinuePhaseFinish{\n}\n");
                    }
                else if(Obj.f.find("ApiContinue")!=string::npos){
                    break;
                    Sleep(1000);
                    NetSend(net,"ContinueFinish{\n}\n");
                    }
                else if(Obj.f.find("ApiGameOver")!=string::npos){
                    Sleep(1000);
                    NetSend(net,"GameOverPhaseFinish{\n}\n");
                    }
                else if(Obj.f!=""){
                    puts(Obj.s.c_str());
                    }
                else{
                    nomsg=1;
                    }
                if(!nomsg){
                    mvt=GetTime()+300000;
                    }
                if(GetTime()>=mvt)break;
                Sleep(1);
                }
            NetClose(net);
            if(brek)continue;
            if(Win){
                puts(("Win "+IntToStr(thread_id)).c_str());
                kalisin("/Game/TeamBattleResult","{\"roomid\":"+RoomID+"}");
                Sleep(2000);
                if(AutoChaCha)
                    AutoSellCard();
                }
            string home=HomeShow();
            if(StrToInt(JsonGetObj(home,"card_num"))>=StrToInt(JsonGetObj(home,"card_max"))){
                puts("pack is full~");
                break;
                }
            //kalisin("/Game/HomeShow");
            //Sleep(1000);
            }
        puts("Press E to exit battle");
        if(ChkKey('E'))break;
        if(FinishAllThread)break;
        }
    FinishAllThread=1;
    }
    
void Walk(){
    kalisin("/Game/ExploreStart","{\"arthur_type\":1,\"deck_idx\":0}");
    printf("Start Walking");
    for(int i=0;i<5;i++){
        putchar('.');
        Sleep(500);
        }
    puts("\nFinish Walking!");
    kalisin("/Game/ExploreEnd");
    }
void UseItem(string obj="1000"){
    kalisin("/Game/ItemUse","{\"itemid\":"+obj+"}");
    }
void PvP(){
    //string pvps=kalisin("/Game/PvpShow");
    }
void SoloPlay(){
    //kalisin("/Game/TeamBattleSoloStart");
    }
void ChangeName(string s){
    kalisin("/Game/UserSetName","{\"name\":\""+s+"\"}");
    }
int main(/*int argc, char** argv*/){
    freopen("stderr","w",stderr);
    login();
    Sleep(2000);
    while(1){
        puts("Chose Service:");
        puts("1 Online Battle Other PT");
        puts("2 Online Battle Self PT   //Full of bugs");
        puts("3 Walk");
        puts("4 Sell Card");
        puts("5 Use Item //testing");
        puts("6 Fight Single            //Not Finish");
        puts("7 PvP                     //Not Finish");
        puts("8 Change Name");
        puts("0 Kalisin Cmd");
        int inp;
        scanf("%d",&inp);
        if(inp==1){
            printf("deck_arthur_type:");
            cin>>deck_arthur_type;
            printf("deck_arthur_type_idx:");
            cin>>deck_arthur_type_idx;
            printf("bossid:");
            cin>>bossid;
            getline(cin,pass);
            printf("pass[\"\"]:");
            getline(cin,pass);
            printf("AutoSellCard[0]:");
            string ACC;
            getline(cin,ACC);
            if(ACC!="")AutoChaCha=StrToInt(ACC);
            else AutoChaCha=0;
            printf("repeat times[-1]:");
            string gocts;
            getline(cin,gocts);
            FinishAllThread=0;
            int thread_max=1;
            printf("Threads[1]:");
            string tms;
            getline(cin,tms);
            if(tms!="")thread_max=StrToInt(tms);
            Thread thr[thread_max];
            for(int i=0;i<thread_max;i++)
                thr[i]=ThreadCreat(OnlineBattleOther,i),Sleep(1000);
            for(int i=0;i<thread_max;i++)
                Wait(thr[i]);
            //OnlineBattleOther();
            }
        else if(inp==2){
            //puts("Error!");
            OnlineBattleSelf();
            }
        else if(inp==3)
            Walk();
        else if(inp==4){
            puts("Sell Card ID[20000001]:");
            string cid;
            getline(cin,cid);
            getline(cin,cid);
            if(cid=="")cid="20000001";
            puts("Sell Card Num[-1]:");
            int cnum=-1;
            string scnum;
            getline(cin,scnum);
            if(scnum!="")cnum=StrToInt(scnum);
            SellCard(cid,cnum);
            }
        else if(inp==5){
            puts("Item ID(1000=half water)[NULL]:");
            string cid;
            cin>>cid;
            if(cid=="")continue;
            UseItem(cid);
            }
        else if(inp==6){
            SoloPlay();
            }
        else if(inp==7){
            PvP();
            }
        else if(inp==8){
            string nwm;
            printf("New Name:");
            getline(cin,nwm);
            ChangeName(nwm);
            }
        else if(inp==0){
            string a,b;
            getline(cin,a);
            getline(cin,a);
            getline(cin,b);
            printf(kalisin(a,b).s.c_str());
            }
        //kalisin("/Game/HomeShow");
        HomeShow();
        Sleep(2000);
        }
    return 0;
    }
